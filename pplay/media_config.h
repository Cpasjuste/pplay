//
// Created by cpasjuste on 22/10/18.
//

#ifndef MEDIA_CONFIG_H
#define MEDIA_CONFIG_H

#include "cross2d/skeleton/config.h"

#define OPT_STREAM_VID      "VID_STREAM"
#define OPT_STREAM_AUD      "AUD_STREAM"
#define OPT_STREAM_SUB      "SUB_STREAM"
#define OPT_POSITION        "POSITION"

class Main;

class MediaConfig : public c2d::config::Config {

public:

    explicit MediaConfig(const std::string &path, int version = 1);

    int getStream(const std::string &optName);

    void setStream(const std::string &optName, int stream);

    int getPosition();

    void setPosition(int position);
};

#endif //MEDIA_CONFIG_H
