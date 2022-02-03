//
// Created by cpasjuste on 02/04/19.
//

#include <SDL_video.h>
#include "mpv.h"

static void *get_proc_address_mpv(void *unused, const char *name) {
    return SDL_GL_GetProcAddress(name);
}

Mpv::Mpv(const std::string &configPath, bool initRender) {

    handle = mpv_create();
    if (!handle) {
        printf("Mpv::Mpv: error: mpv_create\n");
        return;
    }

    mpv_set_option_string(handle, "config", "yes");
    mpv_set_option_string(handle, "config-dir", configPath.c_str());
#ifndef NDEBUG
    mpv_set_option_string(handle, "terminal", "yes");
    mpv_set_option_string(handle, "msg-level", "all=v");
#endif
#ifdef __SWITCH__
    mpv_set_option_string(handle, "vd-lavc-threads", "4");
    // TODO: test this
    mpv_set_option_string(handle, "opengl-pbo", "yes");
#else
    mpv_set_option_string(handle, "vd-lavc-threads", "6");
#endif
    mpv_set_option_string(handle, "audio-channels", "stereo");
#ifdef FULL_TEXTURE_TEST
    mpv_set_option_string(handle, "video-unscaled", "yes");
#endif
    mpv_set_option_string(handle, "fbo-format", "rgba8");
    //TODO: should add this as option
    //mpv_set_option_string(handle, "vd-lavc-skiploopfilter", "all");
    //mpv_set_option_string(handle, "vd-lavc-fast", "yes");

#if defined(__LINUX__) && defined(NDEBUG)
    mpv_set_option_string(handle, "hwdec", "auto-safe");
#endif

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

int Mpv::pause() {
    return mpv_command_string(handle, "set pause yes");
}

int Mpv::resume() {
    return mpv_command_string(handle, "set pause no");
}

int Mpv::stop() {
    return mpv_command_string(handle, "stop");
}

int Mpv::seek(double position) {
    std::string cmd = "no-osd seek " + std::to_string(position) + " absolute";
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
    if (id > -1) {
        std::string cmd = "no-osd set vid " + std::to_string(id);
        return mpv_command_string(handle, cmd.c_str());
    }
    return -1;
}

int Mpv::setAid(int id) {
    if (id > -1) {
        std::string cmd = "no-osd set aid " + std::to_string(id);
        return mpv_command_string(handle, cmd.c_str());
    }
    return -1;
}

int Mpv::setSid(int id) {
    std::string cmd = "no-osd set sid ";
    cmd += id < 0 ? "no" : std::to_string(id);
    return mpv_command_string(handle, cmd.c_str());
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
