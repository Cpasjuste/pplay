//
// Created by cpasjuste on 22/10/18.
//

#include "config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(const std::string &name, const std::string &path, int version)
        : Config(name, path, version) {

    addOption({"HOME_PATH", ""});
    addOption({"LAST_PATH", ""});

    Group group("HTTP_SERVERS");
    group.addOption({"SERVER0", "http://samples.ffmpeg.org/"});
    group.addOption({"SERVER1", ""});
    group.addOption({"SERVER2", ""});
    group.addOption({"SERVER3", ""});
    group.addOption({"SERVER4", ""});
    addGroup(group);

    // load the configuration from file, overwriting default values
    if (!load()) {
        // file doesn't exist or is malformed, (re)create it
        save();
    }
}
