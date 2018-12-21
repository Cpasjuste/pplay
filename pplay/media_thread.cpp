//
// Created by cpasjuste on 13/11/18.
//

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
int ffmpeg_main(int argc, const char **argv);
}

#include <algorithm>
#include "main.h"
#include "media_thread.h"
#include "utility.h"

#if 0
static void dump_metadata(const std::string &desc, AVDictionary *dic) {
    AVDictionaryEntry *t = nullptr;
    printf("- %s\n", desc.c_str());
    while ((t = av_dict_get(dic, "", t, AV_DICT_IGNORE_SUFFIX))) {
        printf("\t%s -> %s\n", t->key, t->value);
    }
}
#endif

static const MediaInfo get_media_info(MediaThread *mediaThread, const c2d::Io::File &file) {

    MediaInfo media;
    std::string cachePath = mediaThread->getMediaCachePath(file);
    printf("get_media_info: %s => %s\n", file.name.c_str(), cachePath.c_str());
    mediaThread->getMain()->getStatus()->show("Scanning...", file.name, true, true);

    // open
    AVFormatContext *ctx = nullptr;
    printf("get_media_info: avformat_open_input\n");
    int res = avformat_open_input(&ctx, file.path.c_str(), nullptr, nullptr);
    if (res != 0) {
        char err_str[256];
        av_strerror(res, err_str, 255);
        printf("get_media_info: unable to open '%s': %s\n", file.path.c_str(), err_str);
        mediaThread->getMain()->getStatus()->show("Error...", err_str, false, false);
        // cache an "unknow" media file so we don't try that file again
        media.serialize(cachePath);
        return media;
    }

    printf("get_media_info: avformat_find_stream_info\n");
    res = avformat_find_stream_info(ctx, nullptr);
    if (res < 0) {
        char err_str[256];
        av_strerror(res, err_str, 255);
        printf("get_media_info: unable to parse '%s': %s\n", file.path.c_str(), err_str);
        mediaThread->getMain()->getStatus()->show("Error...", err_str, false, false);
        avformat_close_input(&ctx);
        // cache an "unknow" media file so we don't try that file again
        media.serialize(cachePath);
        return media;
    }

    AVDictionaryEntry *language, *title = av_dict_get(ctx->metadata, "title", nullptr, 0);
    media.title = title ? title->value : "N/A";
    media.path = file.path;
    media.bit_rate = (int) ctx->bit_rate;
    media.duration = ctx->duration / AV_TIME_BASE;
    printf("get_media_info: stream count: %i\n", ctx->nb_streams);
    //dump_metadata("media", ctx->metadata);

    for (int i = 0; i < (int) ctx->nb_streams; i++) {
        const AVCodecParameters *codec = ctx->streams[i]->codecpar;
        int type = codec->codec_type;
        if (type == AVMEDIA_TYPE_VIDEO) {
            MediaInfo::Stream stream{};
            title = av_dict_get(ctx->streams[i]->metadata, "title", nullptr, 0);
            language = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
            stream.id = i;
            stream.title = title ? title->value : "N/A";
            stream.language = language ? language->value : "N/A";
            stream.codec = avcodec_get_name(codec->codec_id);
            stream.bit_rate = (int) codec->bit_rate;
            stream.width = codec->width;
            stream.height = codec->height;
            media.videos.push_back(stream);
            printf("get_media_info: found video stream: %s\n", stream.title.c_str());
            //dump_metadata("video stream", ctx->streams[i]->metadata);
        } else if (type == AVMEDIA_TYPE_AUDIO) {
            MediaInfo::Stream stream{};
            title = av_dict_get(ctx->streams[i]->metadata, "title", nullptr, 0);
            language = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
            stream.id = i;
            stream.title = title ? title->value : "N/A";
            stream.language = language ? language->value : "N/A";
            stream.codec = avcodec_get_name(codec->codec_id);
            stream.bit_rate = (int) codec->bit_rate;
            stream.sample_rate = codec->sample_rate;
            media.audios.push_back(stream);
            printf("get_media_info: found audio stream: %s\n", stream.title.c_str());
            //dump_metadata("audio stream", ctx->streams[i]->metadata);
        } else if (type == AVMEDIA_TYPE_SUBTITLE) {
            MediaInfo::Stream stream{};
            title = av_dict_get(ctx->streams[i]->metadata, "title", nullptr, 0);
            language = av_dict_get(ctx->streams[i]->metadata, "language", nullptr, 0);
            stream.id = i;
            stream.title = title ? title->value : "N/A";
            stream.language = language ? language->value : "N/A";
            stream.codec = avcodec_get_name(codec->codec_id);
            media.subtitles.push_back(stream);
            printf("get_media_info: found subtitle stream: %s\n", stream.title.c_str());
            //dump_metadata("subtitle stream", ctx->streams[i]->metadata);
        }
    }

    media.serialize(cachePath);
    avformat_close_input(&ctx);
#if 0
    // TODO: extract thumbnail
        std::string p = cachePath + ".png";
        const char *argv[] = {
                "ffmpeg", "-i", mediaPath.c_str(), "-ss", "00:00:02", "-vframes", "1", p.c_str()
        };
        ffmpeg_main(8, argv);
#endif
    mediaThread->getMain()->getStatus()->hide();

    return media;
}

static int media_info_thread(void *ptr) {

    auto *mediaThread = (MediaThread *) ptr;

    printf("media_thread: start\n");

    while (mediaThread->isRunning()) {

        if (!mediaThread->isRunning()) {
            break;
        }

        if (mediaThread->getMain()->getPlayer() && mediaThread->getMain()->getPlayer()->isFullscreen()) {
            mediaThread->getMain()->delay(100);
            continue;
        }

        if (mediaThread->mediaList.empty()) {
            mediaThread->getMain()->delay(100);
            continue;
        }

        get_media_info(mediaThread, mediaThread->mediaList[0]);

        // remove from list
        SDL_LockMutex(mediaThread->getMutex());
        mediaThread->mediaList.erase(mediaThread->mediaList.begin());
        SDL_UnlockMutex(mediaThread->getMutex());
    }

    printf("media_thread: end\n");

    return 0;
}

MediaThread::MediaThread(Main *m, const std::string &cp) {

    main = m;
    cachePath = cp;
    cache = main->getConfig()->getOption(OPT_CACHE_MEDIA_INFO)->getInteger() == 1;

    avformat_network_init();

    if (cache) {
        mutex = SDL_CreateMutex();
        thread = SDL_CreateThread(media_info_thread, "pplay_info", (void *) this);
    }
}

const MediaInfo MediaThread::getMediaInfo(const c2d::Io::File &file, bool fromCache) {

    MediaInfo media;

    if (pplay::Utility::isMedia(file)) {
        std::string cachePath = getMediaCachePath(file);
        if (main->getIo()->exist(cachePath)) {
            //printf("getMediaInfo::deserialize: %s\n", file.name.c_str());
            media.deserialize(cachePath);
        } else {
            // we don't want to scan media file
            if (fromCache) {
                return media;
            }
            // scan media file
            if (cache) {
                // media info not yet available, cache for later use
                SDL_LockMutex(mutex);
                auto f = find_if(mediaList.begin(), mediaList.end(), [&file](const c2d::Io::File &obj) {
                    return obj.path == file.path;
                });
                if (f == mediaList.end()) {
                    mediaList.emplace_back(file);
                }
                SDL_UnlockMutex(mutex);
            } else {
                return get_media_info(this, file);
            }
        }
    }

    return media;
}

Main *MediaThread::getMain() {
    return main;
}

const std::string MediaThread::getMediaCachePath(const c2d::Io::File &file) const {
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    std::string path = cachePath + hash;
    return path;
}

SDL_mutex *MediaThread::getMutex() {
    return mutex;
}

bool MediaThread::isRunning() {
    return running;
}

bool MediaThread::isCaching() {
    return cache;
}

MediaThread::~MediaThread() {

    int ret;

    running = false;
    if (cache) {
        SDL_WaitThread(thread, &ret);
        SDL_DestroyMutex(mutex);
    }

    avformat_network_deinit();
}
