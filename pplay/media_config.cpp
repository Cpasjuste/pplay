//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "media_config.h"

using namespace c2d;

MediaConfig::MediaConfig(const std::string &path, int version) : Config("PPLAY", path, version) {

    addOption({OPT_STREAM_VID, -1});
    addOption({OPT_STREAM_AUD, -1});
    addOption({OPT_STREAM_SUB, -1});
    addOption({OPT_POSITION, (float) 0});

    // load the configuration from file, overwriting default values
    load();
    // save configuration, in case new options needs to be added
    save();
}

int MediaConfig::getStream(const std::string &optName) {
    return getOption(optName)->getInteger();
}

void MediaConfig::setStream(const std::string &optName, int stream) {
    getOption(optName)->setInteger(stream);
    save();
}

float MediaConfig::getPosition() {
    return getOption(OPT_POSITION)->getFloat();
}

void MediaConfig::setPosition(float position) {
    getOption(OPT_POSITION)->setFloat(position);
    save();
}
