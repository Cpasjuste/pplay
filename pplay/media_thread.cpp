//
// Created by cpasjuste on 13/11/18.
//

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}

#include "media_thread.h"

static std::string srcName;
static std::string srcPath;
static std::string dstPath;

static void dump_metadata(const std::string &desc, AVDictionary *dic) {
    AVDictionaryEntry *t = nullptr;
    printf("- %s\n", desc.c_str());
    while ((t = av_dict_get(dic, "", t, AV_DICT_IGNORE_SUFFIX))) {
        printf("\t%s -> %s\n", t->key, t->value);
    }
}

static int media_info_thread(void *ptr) {

    auto *mediaThread = (MediaThread *) ptr;

    printf("media_info_thread: start\n");

    while (mediaThread->isRunning()) {

        SDL_LockMutex(mediaThread->getMutex());
        SDL_CondWait(mediaThread->getCond(), mediaThread->getMutex());
        // check if we want to quit...
        if (!mediaThread->isRunning()) {
            break;
        }

        // TODO: extract media info
        printf("media_info_thread: process: %s => %s\n", srcPath.c_str(), dstPath.c_str());

        // open
        avformat_network_init();
        AVFormatContext *ctx = nullptr;
        if (avformat_open_input(&ctx, srcPath.c_str(), nullptr, nullptr) != 0) {
            printf("media_info_thread: unable to open '%s'\n", srcPath.c_str());
            avformat_network_deinit();
            continue;
        }

        av_opt_set_int(ctx, "probesize", INT_MAX, 0);
        av_opt_set_int(ctx, "analyzeduration", INT_MAX, 0);
        if (avformat_find_stream_info(ctx, nullptr) < 0) {
            printf("media_info_thread: unable to open '%s'\n", srcPath.c_str());
            avformat_close_input(&ctx);
            avformat_network_deinit();
            continue;
        }

#if 1
        dump_metadata("media", ctx->metadata);
#endif

        MediaInfo mediaInfo;
        AVDictionaryEntry *tag = av_dict_get(ctx->metadata, "title", nullptr, 0);
        mediaInfo.name = tag ? tag->value : srcName;
        mediaInfo.path = srcPath;

        for (int i = 0; i < (int) ctx->nb_streams; i++) {
            const AVCodecParameters *codec = ctx->streams[i]->codecpar;
            int type = codec->codec_type;
            if (type == AVMEDIA_TYPE_VIDEO) {
                MediaInfo::StreamInfo stream;
                tag = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
                stream.name = tag ? tag->value : "unknown";
                stream.codec = avcodec_get_name(codec->codec_id);
                stream.rate = (int) codec->bit_rate;
                stream.width = codec->width;
                stream.height = codec->height;
                mediaInfo.videos.push_back(stream);
#if 1
                dump_metadata("video stream", ctx->streams[i]->metadata);
#endif
            } else if (type == AVMEDIA_TYPE_AUDIO) {
                MediaInfo::StreamInfo stream;
                tag = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
                stream.name = tag ? tag->value : "unknown";
                stream.codec = avcodec_get_name(codec->codec_id);
                stream.rate = (int) codec->bit_rate;
                mediaInfo.audios.push_back(stream);
#if 1
                dump_metadata("audio stream", ctx->streams[i]->metadata);
#endif
            } else if (type == AVMEDIA_TYPE_SUBTITLE) {
                MediaInfo::StreamInfo stream;
                tag = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
                stream.name = tag ? tag->value : "unknown";
                stream.codec = avcodec_get_name(codec->codec_id);
                mediaInfo.subtitles.push_back(stream);
#if 1
                dump_metadata("subtitle stream", ctx->streams[i]->metadata);
#endif
            }
        }

        mediaInfo.serialize(dstPath);

        // close
        avformat_close_input(&ctx);
        avformat_network_deinit();

        SDL_UnlockMutex(mediaThread->getMutex());

        printf("media_info_thread: process: OK\n");
    }

    printf("media_info_thread: end\n");

    return 0;
}

MediaThread::MediaThread() {

    mutex = SDL_CreateMutex();
    cond = SDL_CreateCond();
    thread = SDL_CreateThread(media_info_thread, "pplay_info", (void *) this);
}

MediaInfo *MediaThread::getInfo(c2d::Io *io, const std::string &n, const std::string &src, const std::string &dst) {

    if (io->exist(dstPath)) {
        auto *mi = new MediaInfo();
        mi->deserialize(dstPath);
        return mi;
    }

    // media info not yet available, cache for later use
    srcName = n;
    srcPath = src;
    dstPath = dst;
    SDL_LockMutex(mutex);
    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);

    return nullptr;
}

bool MediaThread::isRunning() {
    return running;
}

SDL_mutex *MediaThread::getMutex() {
    return mutex;
}

SDL_cond *MediaThread::getCond() {
    return cond;
}

MediaThread::~MediaThread() {

    int ret;

    running = false;

    printf("~MediaInfo: signal...\n");
    SDL_LockMutex(mutex);
    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);

    printf("~MediaInfo: wait thread...\n");
    SDL_WaitThread(thread, &ret);
    //SDL_DetachThread(thread);

    SDL_DestroyCond(cond);
    SDL_DestroyMutex(mutex);
    printf("~MediaInfo: done...\n");
}
