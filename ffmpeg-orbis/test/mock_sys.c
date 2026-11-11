/* Host mock of libSceVideodec2 + the Orbis platform layer.
 * It behaves like the console decoder as far as the contract matters:
 *  - synchronous Decode, must not run on the main thread (needs a big stack)
 *  - AU and frame buffers must come from ovd_sys_dmem_alloc
 *  - output is NV12 with pitch > width, padded height, garbage in the padding
 *  - pictures come out in display order, with decoder delay, no timestamps
 *  - Flush returns buffered pictures one per call
 * The "hardware" is FFmpeg's own software h264 decoder (uncropped output). */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavcodec/orbisvdec.h"

pid_t mock_main_tid;
int   mock_extra_pad;          /* extra rows between Y and UV */
int   mock_fail_at = -1;       /* Decode returns an error for this AU index */
int   mock_err_frame_at = -1;  /* picture flagged isErrorFrame */
int   mock_no_flush;           /* Flush not exported */
int   mock_no_reset;           /* Reset not exported */
int   mock_stats_decode, mock_stats_reset, mock_stats_flush, mock_stats_create;
int   mock_wrong_thread;
int   mock_order_violation, mock_dec_alive, mock_queue_alive, mock_queue_when_alloc_decoder;

#define MAXA 64
static void *allocs[MAXA]; static size_t asz[MAXA]; static int na;

static int in_alloc(const void *p, size_t n) {
    for (int i = 0; i < na; i++)
        if ((const char *)p >= (char *)allocs[i] && (const char *)p + n <= (char *)allocs[i] + asz[i]) return 1;
    return 0;
}

int ovd_sys_dmem_alloc(OVDMem *m, size_t size, int type) {
    size = (size + 0x3fff) & ~(size_t)0x3fff;
    void *p = aligned_alloc(0x4000, size);
    if (!p || na >= MAXA) return -12;
    memset(p, 0xA5, size);
    allocs[na] = p; asz[na++] = size;
    m->va = p; m->size = size; m->off = 0;
    return 0;
}
void ovd_sys_priv_leave(void) {}
static int cfg_w, cfg_h;
void ovd_sys_dmem_free(OVDMem *m) {
    /* the real console faults if memory goes away while the decoder or the compute queue still use it */
    if (mock_dec_alive || mock_queue_alive) mock_order_violation++;
    for (int i = 0; i < na; i++) if (allocs[i] == m->va) { free(allocs[i]); allocs[i] = allocs[--na]; asz[i] = asz[na]; m->va = NULL; return; }
    mock_order_violation++;                       /* double free / foreign pointer */
}
int ovd_mock_live_allocs(void) { return na; }
void ovd_sys_usleep(unsigned u) { usleep(u / 20); }   /* the mock does not need the real delays */

typedef struct Dec { AVCodecContext *c; AVPacket *pkt; AVFrame *f; int n; int draining; int sent_null; } Dec;
static Dec D;

static int q_cmem(OVDComputeMemInfo *i) { i->cpuGpuMemorySize = 0x10000; return 0; }
static int q_alloc(const OVDComputeConfig *c, const OVDComputeMemInfo *m, void **q) {
    if (!in_alloc(m->cpuGpuMemory, 16)) return -1; *q = (void *)0x1234; mock_queue_alive = 1; return 0; }
static int q_rel(void *q) { if (mock_dec_alive) mock_order_violation++; mock_queue_alive = 0; return 0; }
static int q_dmem(const OVDDecoderConfig *c, OVDDecoderMemInfo *m) {
    if (c->thisSize != sizeof(*c) || c->maxFrameWidth <= 0 || c->maxFrameWidth > 1920 || c->maxFrameHeight <= 0 || c->maxFrameHeight > 1088 || (c->maxFrameWidth & 15) || (c->maxFrameHeight & 15) || c->maxDpbFrameCount < 1 || c->maxDpbFrameCount > 8 || !c->computeQueue) return -1;
    cfg_w = c->maxFrameWidth; cfg_h = c->maxFrameHeight;
    m->cpuMemorySize = 0x100000; m->gpuMemorySize = 0x100000; m->cpuGpuMemorySize = 0x100000;
    m->maxFrameBufferSize = ((size_t)((cfg_w + 255) & ~255) + 256) * (cfg_h + 32) * 3 / 2; m->frameBufferAlignment = 0x100; return 0; }
static int d_create(const OVDDecoderConfig *c, const OVDDecoderMemInfo *m, void **d) {
    if (!in_alloc(m->cpuMemory, 16) || !in_alloc(m->gpuMemory, 16) || !in_alloc(m->cpuGpuMemory, 16)) return -2;
    mock_stats_create++;
    const AVCodec *codec = avcodec_find_decoder_by_name("h264");
    D.c = avcodec_alloc_context3(codec);
    D.c->apply_cropping = 0; D.c->thread_count = 1;
    if (avcodec_open2(D.c, codec, NULL) < 0) return -3;
    D.pkt = av_packet_alloc(); D.f = av_frame_alloc(); D.n = 0; D.draining = D.sent_null = 0;
    mock_dec_alive = 1; *d = &D; return 0; }
static int d_delete(void *d) { mock_dec_alive = 0; avcodec_free_context(&D.c); av_packet_free(&D.pkt); av_frame_free(&D.f); return 0; }

static int emit(OVDFrameBuffer *fb, OVDOutput *o, AVFrame *f, int idx) {
    if (f->width > cfg_w || f->height > cfg_h) return -7;   /* the hardware was sized for a smaller picture */
    int pitch = (f->width + 255) & ~255; if (pitch == f->width) pitch += 256;
    int ph = f->height + mock_extra_pad;
    if ((size_t)pitch * ph * 3 / 2 > fb->frameBufferSize) return -5;
    if (!in_alloc(fb->frameBuffer, fb->frameBufferSize)) return -6;
    uint8_t *b = fb->frameBuffer;
    memset(b, 0x55, (size_t)pitch * ph * 3 / 2);               /* garbage padding */
    for (int y = 0; y < f->height; y++) memcpy(b + (size_t)y * pitch, f->data[0] + (size_t)y * f->linesize[0], f->width);
    uint8_t *uv = b + (size_t)pitch * ph;
    for (int y = 0; y < f->height / 2; y++)
        for (int x = 0; x < f->width / 2; x++) {
            uv[(size_t)y * pitch + 2 * x]     = f->data[1][(size_t)y * f->linesize[1] + x];
            uv[(size_t)y * pitch + 2 * x + 1] = f->data[2][(size_t)y * f->linesize[2] + x];
        }
    o->isValid = 1; o->isErrorFrame = (idx == mock_err_frame_at); o->pictureCount = 1; o->codecType = 1;
    o->frameWidth = f->width; o->framePitch = pitch; o->frameHeight = ph; o->frameBuffer = b;
    o->frameBufferSize = (size_t)pitch * ph * 3 / 2; o->frameFormat = 0; o->framePitchInBytes = pitch;
    return 0; }

static int d_decode(void *d, const OVDInput *in, OVDFrameBuffer *fb, OVDOutput *o) {
    if (syscall(SYS_gettid) == mock_main_tid) mock_wrong_thread++;
    if (in->thisSize != sizeof(*in) || fb->thisSize != sizeof(*fb) || o->thisSize != sizeof(*o)) return -10;
    if (!in_alloc(in->auData, in->auSize)) return -11;
    int idx = mock_stats_decode++;
    if (idx == mock_fail_at) return 0x80020001;
    D.pkt->data = (uint8_t *)in->auData; D.pkt->size = in->auSize;
    int r = avcodec_send_packet(D.c, D.pkt); if (r < 0) return -12;
    r = avcodec_receive_frame(D.c, D.f);
    if (r < 0) return 0;                       /* buffered */
    int e = emit(fb, o, D.f, idx); av_frame_unref(D.f); return e; }

static int d_flush(void *d, OVDFrameBuffer *fb, OVDOutput *o) {
    mock_stats_flush++;
    if (!D.sent_null) { avcodec_send_packet(D.c, NULL); D.sent_null = 1; }
    int r = avcodec_receive_frame(D.c, D.f); if (r < 0) return 0;
    int e = emit(fb, o, D.f, -2); av_frame_unref(D.f); return e; }
static int d_reset(void *d) { mock_stats_reset++; avcodec_flush_buffers(D.c); D.sent_null = 0; return 0; }

int ovd_sys_bind(OVDApi *a) {
    memset(a, 0, sizeof(*a));
    a->QueryComputeMemoryInfo = q_cmem; a->AllocateComputeQueue = q_alloc; a->ReleaseComputeQueue = q_rel;
    a->QueryDecoderMemoryInfo = q_dmem; a->CreateDecoder = d_create; a->DeleteDecoder = d_delete;
    a->Decode = d_decode; a->Flush = mock_no_flush ? NULL : d_flush; a->Reset = mock_no_reset ? NULL : d_reset;
    return 0; }
