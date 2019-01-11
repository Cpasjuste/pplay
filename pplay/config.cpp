//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(Main *main, int version)
        : Config("PPLAY", main->getIo()->getDataWritePath() + "pplay.cfg", version) {

    addOption({OPT_NETWORK, "http://samples.ffmpeg.org/"});
    addOption({OPT_HOME_PATH, main->getIo()->getDataWritePath()});
    addOption({OPT_LAST_PATH, main->getIo()->getDataWritePath()});
    addOption({OPT_CACHE_MEDIA_INFO, (int) 1});
    addOption({OPT_BUFFER, "Low"}); // Low, Medium, High, VeryHigh
    addOption({OPT_CPU_BOOST, "Enabled"}); // Disabled, Enabled

    // load the configuration from file, overwriting default values
    load();

    if (!main->getIo()->exist(getOption(OPT_HOME_PATH)->getString())) {
        getOption(OPT_HOME_PATH)->setString(main->getIo()->getDataWritePath());
        //printf("HOME_PATH: %s\n", getOption("HOME_PATH")->getString().c_str());
    }

    if (!main->getIo()->exist(getOption(OPT_LAST_PATH)->getString())) {
        getOption(OPT_LAST_PATH)->setString(main->getIo()->getDataWritePath());
        //printf("LAST_PATH: %s\n", getOption("LAST_PATH")->getString().c_str());
    }

    // save configuration, in case new options needs to be added
    save();
}
