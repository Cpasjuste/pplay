#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"

extern pid_t mock_main_tid; extern int mock_extra_pad, mock_fail_at, mock_err_frame_at, mock_no_flush, mock_no_reset;
extern int mock_order_violation, mock_dec_alive, mock_queue_alive; int ovd_mock_live_allocs(void);
extern int mock_stats_decode, mock_stats_reset, mock_stats_flush, mock_stats_create, mock_wrong_thread;

typedef struct { int64_t pts; uint64_t h; } Rec;
static uint64_t hash_frame(const AVFrame *f) {
    uint64_t h = 1469598103934665603ull;
    for (int p = 0; p < 3; p++) { int w = p ? f->width / 2 : f->width, hh = p ? f->height / 2 : f->height;
        for (int y = 0; y < hh; y++) { const uint8_t *s = f->data[p] + (size_t)y * f->linesize[p];
            for (int x = 0; x < w; x++) { h ^= s[x]; h *= 1099511628211ull; } } }
    return h; }

static Rec *ref; static int nref;
static int build_ref(const char *file) {
    AVFormatContext *fc = NULL; if (avformat_open_input(&fc, file, NULL, NULL) < 0) return -1;
    avformat_find_stream_info(fc, NULL);
    int vs = av_find_best_stream(fc, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    const AVCodec *c = avcodec_find_decoder_by_name("h264");
    AVCodecContext *cc = avcodec_alloc_context3(c); avcodec_parameters_to_context(cc, fc->streams[vs]->codecpar);
    cc->thread_count = 1; if (avcodec_open2(cc, c, NULL) < 0) return -2;
    AVPacket *p = av_packet_alloc(); AVFrame *f = av_frame_alloc(); ref = calloc(4096, sizeof(Rec));
    int draining = 0;
    for (;;) {
        if (!draining) { int r = av_read_frame(fc, p); if (r < 0) { avcodec_send_packet(cc, NULL); draining = 1; }
            else { if (p->stream_index == vs) avcodec_send_packet(cc, p); av_packet_unref(p); } }
        while (avcodec_receive_frame(cc, f) >= 0) { ref[nref].pts = f->pts; ref[nref++].h = hash_frame(f); av_frame_unref(f); }
        if (draining) break; }
    return 0; }

static uint64_t find_ref(int64_t pts, int *found) { for (int i = 0; i < nref; i++) if (ref[i].pts == pts) { *found = 1; return ref[i].h; } *found = 0; return 0; }

#define FAIL(...) do { printf("FAIL: " __VA_ARGS__); printf("\n"); return 1; } while (0)

int main(int argc, char **argv) {
    const char *file = argv[1], *mode = argc > 2 ? argv[2] : "play";
    mock_main_tid = syscall(SYS_gettid);
    av_log_set_level(getenv("V") ? AV_LOG_VERBOSE : AV_LOG_ERROR);
    if (!strcmp(mode, "pad")) mock_extra_pad = 16;
    if (!strcmp(mode, "noreset")) mock_no_reset = 1;
    if (!strcmp(mode, "noflush")) mock_no_flush = 1;
    if (!strcmp(mode, "failau")) mock_fail_at = 10;
    if (!strcmp(mode, "errframe")) mock_err_frame_at = 10;
    if (build_ref(file) < 0) FAIL("reference decode");
    int expect_reject = !strcmp(mode, "reject");

    AVFormatContext *fc = NULL; avformat_open_input(&fc, file, NULL, NULL); avformat_find_stream_info(fc, NULL);
    int vs = av_find_best_stream(fc, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    const AVCodec *c = avcodec_find_decoder_by_name("h264_orbis"); if (!c) FAIL("codec not registered");
    int nsess = !strcmp(mode, "reopen") ? 3 : 1, total_ok = 0, total_frames = 0;
    for (int s = 0; s < nsess; s++) {
        av_seek_frame(fc, vs, 0, AVSEEK_FLAG_BACKWARD);
        AVCodecContext *cc = avcodec_alloc_context3(c); avcodec_parameters_to_context(cc, fc->streams[vs]->codecpar);
        AVDictionary *opts = NULL; if (strcmp(mode, "noflush")) av_dict_set(&opts, "drain", "1", 0);
        int r = avcodec_open2(cc, c, &opts); av_dict_free(&opts);
        if (expect_reject) { if (r >= 0) FAIL("expected the stream to be rejected"); printf("OK rejected (%s)\n", av_err2str(r)); return 0; }
        if (r < 0) FAIL("open: %s", av_err2str(r));
        AVPacket *p = av_packet_alloc(); AVFrame *f = av_frame_alloc();
        int64_t last = INT64_MIN; int npk = 0, seeked = 0, draining = 0, got = 0, bad = 0, out_of_order = 0, nomatch = 0;
        for (;;) {
            if (!draining) {
                int rr = av_read_frame(fc, p);
                if (rr < 0) { avcodec_send_packet(cc, NULL); draining = 1; }
                else if (p->stream_index == vs) {
                    if (!strcmp(mode, "seek") && npk == 40 && !seeked) {          /* seek into the middle */
                        av_packet_unref(p); avcodec_flush_buffers(cc);
                        av_seek_frame(fc, vs, fc->streams[vs]->duration * 2 / 3, AVSEEK_FLAG_BACKWARD);
                        seeked = 1; last = INT64_MIN; continue; }
                    npk++; int sr = avcodec_send_packet(cc, p); if (sr < 0) { if (!strcmp(mode, "midchange") && got > 0) { printf("OK: send_packet refused after %d frames (%s)\n", got, av_err2str(sr)); return 0; } FAIL("send_packet"); }
                    av_packet_unref(p);
                } else av_packet_unref(p);
            }
            for (;;) { int g = avcodec_receive_frame(cc, f); if (g == AVERROR(EAGAIN) || g == AVERROR_EOF) break; if (g < 0) { if (!strcmp(mode, "midchange") && got > 0) { printf("OK: left hardware decoding after %d frames (%s)\n", got, av_err2str(g)); return 0; } FAIL("receive %s", av_err2str(g)); }
                int found; uint64_t h = find_ref(f->pts, &found);
                if (!found) nomatch++; else if (h != hash_frame(f)) bad++;
                if (f->pts <= last) out_of_order++; last = f->pts; got++; av_frame_unref(f); }
            if (draining) { int g = avcodec_receive_frame(cc, f); if (g == AVERROR_EOF || g == AVERROR(EAGAIN)) break; if (g >= 0) { av_frame_unref(f); got++; } }
        }
        total_frames += got;
        printf("[%s] session %d: %dx%d pts-frames=%d ref=%d bad=%d nomatch=%d out_of_order=%d decode_calls=%d resets=%d flushes=%d creates=%d\n",
               mode, s, cc->width, cc->height, got, nref, bad, nomatch, out_of_order, mock_stats_decode, mock_stats_reset, mock_stats_flush, mock_stats_create);
        int exp_min = !strcmp(mode, "seek") ? 1 : (!strcmp(mode, "failau") || !strcmp(mode, "errframe") ? nref - 3 : (!strcmp(mode, "noflush") ? nref - 4 : nref));
        int lossy = !strcmp(mode, "failau");
        if ((bad && !lossy) || nomatch || out_of_order) FAIL("content/pts mismatch");
        if (got < exp_min) FAIL("missing frames: got %d expected >= %d", got, exp_min);
        if (mock_wrong_thread) FAIL("Decode ran on the main thread");
        avcodec_free_context(&cc);
        if (mock_dec_alive || mock_queue_alive || ovd_mock_live_allocs()) FAIL("resources still allocated after close (dec=%d queue=%d mem=%d)", mock_dec_alive, mock_queue_alive, ovd_mock_live_allocs());
        if (mock_order_violation) FAIL("teardown order violation");
        total_ok++; av_packet_free(&p); av_frame_free(&f);
    }
    printf("OK\n"); return 0; }
