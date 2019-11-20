//
// Created by cpasjuste on 22/10/18.
//

#ifndef PPLAY_CONFIG_H
#define PPLAY_CONFIG_H

#include "cross2d/skeleton/config.h"

#define OPT_NETWORK             "NETWORK"
#define OPT_HOME_PATH           "HOME_PATH"
#define OPT_LAST_PATH           "LAST_PATH"
#define OPT_CACHE_MEDIA_INFO    "CACHE_MEDIA_INFO"
//#define OPT_BUFFER              "BUFFER"
#define OPT_CPU_BOOST           "CPU_BOOST"
#define OPT_TMDB_LANGUAGE       "TMDB_LANGUAGE"

class Main;

class PPLAYConfig : public c2d::config::Config {

public:

    PPLAYConfig(Main *main, int version = 1);
};

#endif //PPLAY_CONFIG_H
