//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "pplay_config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(Main *main, int version)
        : Config("PPLAY", main->getIo()->getDataPath() + "pplay.cfg", version) {

    addOption({OPT_NETWORK, "http://samples.ffmpeg.org/"});
    addOption({OPT_HOME_PATH, main->getIo()->getDataPath()});
    addOption({OPT_LAST_PATH, main->getIo()->getDataPath()});
    addOption({OPT_CACHE_MEDIA_INFO, (int) 1});
    //addOption({OPT_BUFFER, "Low"}); // Low, Medium, High, VeryHigh
    addOption({OPT_CPU_BOOST, "Disabled"}); // Disabled, Enabled
    addOption({OPT_TMDB_LANGUAGE, "en-US"});

    // load the configuration from file, overwriting default values
    load();

    if (!main->getIo()->exist(getOption(OPT_HOME_PATH)->getString())) {
        getOption(OPT_HOME_PATH)->setString(main->getIo()->getDataPath());
    }

    if (!main->getIo()->exist(getOption(OPT_LAST_PATH)->getString())) {
        getOption(OPT_LAST_PATH)->setString(main->getIo()->getDataPath());
    }

    if (getOption(OPT_TMDB_LANGUAGE)->getString().empty()) {
        getOption(OPT_TMDB_LANGUAGE)->setString("en-US");
    }

    // save configuration, in case new options needs to be added
    save();
}
