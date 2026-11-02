//
// Created by cpasjuste on 22/10/18.
//

#include "main.h"
#include "pplay_config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(Main *main, int version)
        : Config("PPLAY", main->getIo()->getDataPath() + "pplay.cfg", version) {

    addOption({OPT_NETWORK, "http://samples.ffmpeg.org/"});
#ifdef __SWITCH__
    addOption({OPT_UMS_DEVICE, "ums0:/"});
#endif
    addOption({OPT_HOME_PATH, main->getIo()->getDataPath()});
    addOption({OPT_LAST_PATH, main->getIo()->getDataPath()});
    addOption({OPT_CACHE_MEDIA_INFO, (int) 1});
    //addOption({OPT_BUFFER, "Low"}); // Low, Medium, High, VeryHigh
#ifdef __SWITCH__
    addOption({OPT_CPU_BOOST, "Disabled"}); // Disabled, Enabled
#endif
    addOption({OPT_TMDB_LANGUAGE, "en-US"});
    addOption({OPT_HWDEC, "auto"});
    addOption({OPT_SW_SPEED, "quality"});
    // subtitles picked automatically: Latin American Spanish, Spanish, English, English (UK)
    addOption({OPT_SUB_AUTO, "on"});
    addOption({OPT_SUB_LANGS, "es-419,es,en,en-GB"});

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

    if (getOption(OPT_HWDEC)->getString().empty()) {
        getOption(OPT_HWDEC)->setString("auto");
    }

    {
        const std::string speed = getOption(OPT_SW_SPEED)->getString();
        if (speed != "quality" && speed != "balanced" && speed != "fast") {
            getOption(OPT_SW_SPEED)->setString("quality");
        }
    }

    if (getOption(OPT_SUB_AUTO)->getString() != "off") {
        getOption(OPT_SUB_AUTO)->setString("on");
    }

    if (getOption(OPT_SUB_LANGS)->getString().empty()) {
        getOption(OPT_SUB_LANGS)->setString("es-419,es,en,en-GB");
    }

    // save configuration, in case new options needs to be added
    save();
}
