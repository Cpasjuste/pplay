//
// Created by cpasjuste on 22/10/18.
//

#ifndef PPLAY_CONFIG_H
#define PPLAY_CONFIG_H

#include "cross2d/skeleton/config.h"

class Main;

class PPLAYConfig : public c2d::config::Config {

public:

    PPLAYConfig(Main *main, int version = 1);
};

#endif //PPLAY_CONFIG_H
