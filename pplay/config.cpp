//
// Created by cpasjuste on 22/10/18.
//

#include "config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(const std::string &name, const std::string &path, int version)
        : Config(name, path, version) {

    addOption({"NETWORK", "http://samples.ffmpeg.org/"});
    addOption({"HOME_PATH", ""});
    addOption({"LAST_PATH", ""});

    // load the configuration from file, overwriting default values
    if (!load()) {
        // file doesn't exist or is malformed, (re)create it
        save();
    }
}
