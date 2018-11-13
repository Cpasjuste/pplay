//
// Created by cpasjuste on 13/11/18.
//

#include <libavformat/avformat.h>
#include "kitchensink/kitchensink.h"
#include "media_thread.h"

static std::string srcName;
static std::string srcPath;
static std::string dstPath;

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
        // init Kit library
        int err = Kit_Init(KIT_INIT_NETWORK /*| KIT_INIT_ASS*/);
        if (err != 0) {
            printf("media_info_thread: unable to initialize Kitchensink: %s\n", Kit_GetError());
            return false;
        }

        // open source file
        Kit_Source *source = Kit_CreateSourceFromUrl(srcPath.c_str());
        if (!source) {
            printf("media_info_thread: unable to load '%s': %s\n", srcPath.c_str(), Kit_GetError());
            Kit_Quit();
            return false;
        }

        MediaInfo mediaInfo;
        mediaInfo.name = srcName;
        mediaInfo.path = srcPath;

        auto *ctx = (AVFormatContext *) source->format_ctx;
        int stream_count = Kit_GetSourceStreamCount(source);
        for (int i = 0; i < stream_count; i++) {
            const AVCodecParameters *codec = ctx->streams[i]->codecpar;
            const AVCodecDescriptor *codec_desc = avcodec_descriptor_get(codec->codec_id);
            int type = codec->codec_type;
            if (type == AVMEDIA_TYPE_VIDEO) {
                MediaInfo::StreamInfo stream;
                // TODO: name
                stream.name = av_dict_get_string(ctx->streams[i]->metadata, ) //ctx->streams[i]->metadata
                stream.codec = codec_desc ? codec_desc->name : "unknown";
                stream.rate = (int) codec->bit_rate;
                stream.width = codec->width;
                stream.height = codec->height;
                mediaInfo.videos.push_back(stream);
            } else if (type == AVMEDIA_TYPE_AUDIO) {

            } else if (type == AVMEDIA_TYPE_SUBTITLE) {

            }
        }

        mediaInfo.serialize(dstPath);

        SDL_UnlockMutex(mediaThread->getMutex());
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
