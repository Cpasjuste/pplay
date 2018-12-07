//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(Main *main, int version)
        : Config("PPLAY", main->getIo()->getDataWritePath() + "pplay.cfg", version) {

    addOption({"NETWORK", "http://samples.ffmpeg.org/"});
    addOption({"HOME_PATH", main->getIo()->getDataWritePath()});
    addOption({"LAST_PATH", main->getIo()->getDataWritePath()});

    // load the configuration from file, overwriting default values
    if (!load()) {
        // file doesn't exist or is malformed, (re)create it
        save();
    }

    if (!main->getIo()->exist(getOption("HOME_PATH")->getString())) {
        getOption("HOME_PATH")->setString(main->getIo()->getDataWritePath());
        save();
        printf("HOME_PATH: %s\n", getOption("HOME_PATH")->getString().c_str());
    }

    if (!main->getIo()->exist(getOption("LAST_PATH")->getString())) {
        getOption("LAST_PATH")->setString(main->getIo()->getDataWritePath());
        save();
        printf("LAST_PATH: %s\n", getOption("LAST_PATH")->getString().c_str());
    }
}
