//
// Created by cpasjuste on 22/10/18.
//

#ifndef PPLAY_CONFIG_H
#define PPLAY_CONFIG_H

#include "cross2d/skeleton/config.h"

#define OPT_NETWORK             "NETWORK"
#define OPT_UMS_DEVICE          "UMS_DEVICE"
#define OPT_HOME_PATH           "HOME_PATH"
#define OPT_LAST_PATH           "LAST_PATH"
#define OPT_CACHE_MEDIA_INFO    "CACHE_MEDIA_INFO"
//#define OPT_BUFFER              "BUFFER"
#define OPT_CPU_BOOST           "CPU_BOOST"
#define OPT_TMDB_LANGUAGE       "TMDB_LANGUAGE"
#define OPT_SW_SPEED            "SW_DECODE_SPEED" // quality, balanced, fast (software decoding: Hi10, HEVC...)
#define OPT_HWDEC               "HWDEC" // auto, no, force, or any mpv hwdec name
#define OPT_SUB_AUTO            "SUB_AUTO" // on, off: choose the subtitles when a video is loaded
#define OPT_REMOTE              "REMOTE" // on, off: phone remote control (web page served on the LAN)
#define OPT_REMOTE_PORT         "REMOTE_PORT" // TCP port of the remote control page
#define OPT_SUB_LANGS           "SUB_LANGS" // order of preference, see subtitle_pref.h

class Main;

class PPLAYConfig : public c2d::config::Config {

public:

    explicit PPLAYConfig(Main *main, int version = 1);
};

#endif //PPLAY_CONFIG_H
