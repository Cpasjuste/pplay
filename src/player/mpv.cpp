//
// Created by cpasjuste on 02/04/19.
//

#include <cstring>
#include <SDL_video.h>
#include "mpv.h"

#ifdef __PS4__
extern "C" int ps4_mpv_use_precompiled_shaders;
extern "C" int ps4_mpv_dump_shaders;
#endif

static void *get_proc_address_mpv(void *unused, const char *name) {
    return SDL_GL_GetProcAddress(name);
}

#ifdef __PS4__
// true if "mpv.conf" sets this option (options given to libmpv before it starts win over the
// configuration file, so the defaults below are only applied when the user did not choose)
static bool confSets(const std::string &configPath, const std::string &key) {
    FILE *f = fopen((configPath + "/mpv.conf").c_str(), "r");
    if (f == nullptr) {
        return false;
    }

    bool found = false;
    char line[512];
    while (!found && fgets(line, sizeof(line), f) != nullptr) {
        const char *p = line;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (strncmp(p, key.c_str(), key.size()) == 0) {
            p += key.size();
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            found = (*p == '=' || *p == '\0' || *p == '\n' || *p == '\r');
        }
    }
    fclose(f);

    return found;
}
#endif

Mpv::Mpv(const std::string &configPath, bool initRender, const std::string &hwdec,
         const std::string &swSpeed) {
#ifdef __PS4__
    ps4_mpv_use_precompiled_shaders = 1;
    ps4_mpv_dump_shaders = 0;
#endif

    handle = mpv_create();
    if (!handle) {
        printf("Mpv::Mpv: error: mpv_create\n");
        return;
    }

    mpv_set_option_string(handle, "config", "yes");
    mpv_set_option_string(handle, "config-dir", configPath.c_str());
    mpv_set_option_string(handle, "osd-scale", "0.5");
#ifndef NDEBUG
    mpv_set_option_string(handle, "terminal", "yes");
    mpv_set_option_string(handle, "msg-level", "all=v");
#endif

#ifdef __SWITCH__
    mpv_set_option_string(handle, "vd-lavc-threads", "4");
    // TODO: test this
    mpv_set_option_string(handle, "fbo-format", "rgba8");
    mpv_set_option_string(handle, "opengl-pbo", "yes");
#else
    mpv_set_option_string(handle, "vd-lavc-threads", "6");
    mpv_set_option_string(handle, "video-sync", "audio");
#endif
    mpv_set_option_string(handle, "audio-channels", "stereo");
#ifdef __PS4__
    // libmpv defaults (150 MiB of packets ahead + 150 MiB kept behind) are far too much for the
    // memory a homebrew gets on the console: a big 1080p file could run it out of memory
    if (!confSets(configPath, "demuxer-max-bytes")) {
        mpv_set_option_string(handle, "demuxer-max-bytes", "64MiB");
    }
    if (!confSets(configPath, "demuxer-max-back-bytes")) {
        mpv_set_option_string(handle, "demuxer-max-back-bytes", "16MiB");
    }
    // mkv: do not read ahead looking for the duration/first timestamps of huge files
    if (!confSets(configPath, "demuxer-mkv-probe-video-duration")) {
        mpv_set_option_string(handle, "demuxer-mkv-probe-video-duration", "no");
    }
    // ASS fonts attached to the mkv are slow to load and heavy on the console memory
    if (!confSets(configPath, "embeddedfonts")) {
        mpv_set_option_string(handle, "embeddedfonts", "no");
    }
#endif
    // external subtitles are found (and converted to UTF-8, the PS4 libmpv has no
    // iconv) by pplay itself, see subtitles.h: do not let mpv add them a second time
    mpv_set_option_string(handle, "sub-auto", "no");
#ifdef __PS4__
    mpv_set_option_string(handle, "ignore-path-in-watch-later-config", "yes");
#endif

#ifdef FULL_TEXTURE_TEST
    mpv_set_option_string(handle, "video-unscaled", "yes");
#endif
    //TODO: should add this as option (big quality loss)
    //mpv_set_option_string(handle, "vd-lavc-skiploopfilter", "all");
    //mpv_set_option_string(handle, "vd-lavc-fast", "yes");

#if defined(__LINUX__) && defined(NDEBUG)
    mpv_set_option_string(handle, "hwdec", "auto-safe");
#endif

    // hardware decoding (PS4: "orbis-copy" is the libSceVideodec2 wrapper added
    // to ffmpeg, see ffmpeg-orbis/). If mpv does not know the method (ffmpeg built
    // without it) the option is refused and mpv keeps decoding in software.
    if (initRender && !hwdec.empty()) {
        int hres = mpv_set_option_string(handle, "hwdec", hwdec.c_str());
        printf("Mpv::Mpv: hwdec=%s: %s\n", hwdec.c_str(), mpv_error_string(hres));
        if (hres < 0) {
            mpv_set_option_string(handle, "hwdec", "no");
        }
    }

    // software decoding speed/quality trade-off (what is not handled by the
    // hardware decoder runs on the console CPU: Hi10 H.264, HEVC...)
    if (swSpeed == "balanced") {
        mpv_set_option_string(handle, "vd-lavc-skiploopfilter", "nonref");
        mpv_set_option_string(handle, "vd-lavc-fast", "yes");
    } else if (swSpeed == "fast") {
        mpv_set_option_string(handle, "vd-lavc-skiploopfilter", "all");
        mpv_set_option_string(handle, "vd-lavc-fast", "yes");
        mpv_set_option_string(handle, "framedrop", "decoder+vo");
    }

    if (!initRender) {
        mpv_set_option_string(handle, "vid", "no");
        mpv_set_option_string(handle, "aid", "no");
        mpv_set_option_string(handle, "sid", "no");
        mpv_set_option_string(handle, "vo", "null");
        mpv_set_option_string(handle, "ao", "null");
    }

    int res = mpv_initialize(handle);
    if (res) {
        printf("Mpv::Mpv: error: mpv_initialize: %s\n", mpv_error_string(res));
        mpv_terminate_destroy(handle);
        handle = nullptr;
        return;
    }

    if (initRender) {
        mpv_opengl_init_params gl_init_params{get_proc_address_mpv,
                                              nullptr,
                                              nullptr};
        mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE,           (void *) MPV_RENDER_API_TYPE_OPENGL},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                {MPV_RENDER_PARAM_INVALID,            nullptr}
        };

        if (mpv_render_context_create(&context, handle, params) < 0) {
            printf("error: mpv_render_context_create: %s\n", mpv_error_string(res));
            mpv_terminate_destroy(handle);
            handle = nullptr;
        }
    }
}

Mpv::~Mpv() {
    if (context) {
        mpv_render_context_free(context);
    }
    if (handle) {
        mpv_terminate_destroy(handle);
    }
}

int Mpv::load(const std::string &file, LoadType loadType, const std::string &options) {
    printf("Mpv::load(%s)\n", file.c_str());
    if (handle) {
        stop();
        std::string type = "replace";
        if (loadType == LoadType::Append) {
            type = "append";
        } else if (loadType == LoadType::AppendPlay) {
            type = "append-play";
        }
        const char *cmd[] = {"loadfile", file.c_str(), type.c_str(), options.c_str(), nullptr};
        return mpv_command(handle, cmd);
    }

    return -1;
}

int Mpv::save() {
    return mpv_command_string(handle, "write-watch-later-config");
}

int Mpv::addSubtitle(const std::string &path, const std::string &title, const std::string &lang) {
    if (!isAvailable()) {
        return -1;
    }
    // sub-add <url> [<flags> [<title> [<lang>]]]
    const char *cmd[] = {"sub-add", path.c_str(), "auto", title.c_str(), lang.c_str(), nullptr};
    return mpv_command(handle, cmd);
}

int Mpv::pause() {
    return mpv_command_string(handle, "set pause yes");
}

int Mpv::resume() {
    return mpv_command_string(handle, "set pause no");
}

int Mpv::stop() {
    save();
    return mpv_command_string(handle, "stop");
}

int Mpv::seek(double position) {
    std::string cmd = "no-osd seek " + std::to_string(position);
    return mpv_command_string(handle, cmd.c_str());
}

int Mpv::setSpeed(double speed) {
    std::string cmd = "set speed " + std::to_string(speed);
    return mpv_command_string(handle, cmd.c_str());
}

double Mpv::getSpeed() {
    double res = -1;
    mpv_get_property(handle, "speed", MPV_FORMAT_DOUBLE, &res);
    return res;
}

int Mpv::setVid(int id) {
    std::string cmd = "no-osd set vid ";
    cmd += id < 0 ? "no" : std::to_string(id);
    return mpv_command_string(handle, cmd.c_str());
}

int Mpv::setAid(int id) {
    std::string cmd = "no-osd set aid ";
    cmd += id < 0 ? "no" : std::to_string(id);
    return mpv_command_string(handle, cmd.c_str());
}

int Mpv::setSid(int id) {
    std::string cmd = "no-osd set sid ";
    cmd += id < 0 ? "no" : std::to_string(id);
    return mpv_command_string(handle, cmd.c_str());
}

int Mpv::getVid() {
    int64_t vid = -1;
    mpv_get_property(handle, "vid", MPV_FORMAT_INT64, &vid);
    return (int) vid;
}

int Mpv::getAid() {
    int64_t aid = -1;
    mpv_get_property(handle, "aid", MPV_FORMAT_INT64, &aid);
    return (int) aid;
}

int Mpv::getSid() {
    int64_t sid = -1;
    mpv_get_property(handle, "sid", MPV_FORMAT_INT64, &sid);
    return (int) sid;
}

std::string Mpv::getSubtitleCodec() {
    if (handle == nullptr) {
        return "";
    }
    // unavailable (NULL) when no subtitle track is selected
    char *codec = mpv_get_property_string(handle, "current-tracks/sub/codec");
    std::string str = codec ? codec : "";
    if (codec) {
        mpv_free(codec);
    }
    return str;
}

int Mpv::setBitmapSubtitleRendering(bool enable) {
    if (handle == nullptr) {
        return MPV_ERROR_UNINITIALIZED;
    }
    if (enable == bitmapSubFilter) {
        return 0;
    }

    // The "sub" video filter makes mpv blend all the subtitles into the frames with the CPU,
    // so the GPU does not draw any subtitle while it is in the filter chain. It is given
    // 8 bit planar frames ("format"): what the PS4 shaders use anyway (10 bit videos are
    // converted for them) and the best supported by the CPU blending code.
    int res = 0;
    if (enable) {
        res = mpv_command_string(handle, "vf add @pplayfmt:format=yuv420p");
        if (res >= 0) {
            res = mpv_command_string(handle, "vf add @pplaysub:sub");
            if (res < 0) {
                mpv_command_string(handle, "vf remove @pplayfmt");
            }
        }
    } else {
        // (an error here only means the filter was not there)
        mpv_command_string(handle, "vf remove @pplaysub");
        mpv_command_string(handle, "vf remove @pplayfmt");
    }
    printf("Mpv::setBitmapSubtitleRendering(%i): %s\n", enable, mpv_error_string(res));
    if (res >= 0) {
        bitmapSubFilter = enable;
    }

    return res;
}

bool Mpv::isBitmapSubtitleRendering() const {
    return bitmapSubFilter;
}

int Mpv::getVideoBitrate() {
    double bitrate = 0;
    mpv_get_property(handle, "video-bitrate", MPV_FORMAT_DOUBLE, &bitrate);
    return (int) bitrate;
}

int Mpv::getAudioBitrate() {
    double bitrate = 0;
    mpv_get_property(handle, "audio-bitrate", MPV_FORMAT_INT64, &bitrate);
    return (int) bitrate;
}

long Mpv::getDuration() {
    long duration = 0;
    mpv_get_property(handle, "duration", MPV_FORMAT_INT64, &duration);
    return duration;
}

long Mpv::getPosition() {
    long position = 0;
    mpv_get_property(handle, "playback-time", MPV_FORMAT_INT64, &position);
    return position;
}

bool Mpv::isAvailable() {
    return handle != nullptr;
}

bool Mpv::isStopped() {
    int res = 1;
    mpv_get_property(handle, "playback-abort", MPV_FORMAT_FLAG, &res);
    return res == 1;
}

bool Mpv::isPaused() {
    int res = -1;
    mpv_get_property(handle, "pause", MPV_FORMAT_FLAG, &res);
    return res == 1;
}

mpv_event *Mpv::getEvent() {
    return mpv_wait_event(handle, 0);
}

mpv_handle *Mpv::getHandle() {
    return handle;
}

mpv_render_context *Mpv::getContext() {
    return context;
}

MediaInfo Mpv::getMediaInfo(const c2d::Io::File &file) {
    MediaInfo mediaInfo(file);
    std::vector<MediaInfo::Track> streams;

    if (!isAvailable() || isStopped()) {
        return mediaInfo;
    }

    // load track list
    mpv_node node;
    mpv_get_property(handle, "track-list", MPV_FORMAT_NODE, &node);
    if (node.format == MPV_FORMAT_NODE_ARRAY) {
        for (int i = 0; i < node.u.list->num; i++) {
            if (node.u.list->values[i].format == MPV_FORMAT_NODE_MAP) {
                MediaInfo::Track stream{};
                for (int n = 0; n < node.u.list->values[i].u.list->num; n++) {
                    std::string key = node.u.list->values[i].u.list->keys[n];
                    if (key == "type") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.type = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "id") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.id = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "title") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.title = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "lang") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.language = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "codec") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.codec = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "demux-w") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.width = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-h") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.height = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-samplerate") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.sample_rate = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-channel-count") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.channels = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    }
                }
                streams.push_back(stream);
            }
        }
    }

    // set media info tracks
    mediaInfo.videos.clear();
    mediaInfo.audios.clear();
    mediaInfo.subtitles.clear();
    for (auto &stream: streams) {
        if (stream.type == "video") {
            mediaInfo.videos.push_back(stream);
        } else if (stream.type == "audio") {
            mediaInfo.audios.push_back(stream);
        } else if (stream.type == "sub") {
            mediaInfo.subtitles.push_back(stream);
        }
    }

    // set duration
    mediaInfo.duration = getDuration();

    return mediaInfo;
}
