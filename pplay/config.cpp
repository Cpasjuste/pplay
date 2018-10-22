//
// Created by cpasjuste on 22/10/18.
//

#include "config.h"

using namespace c2d;

PPLAYConfig::PPLAYConfig(const std::string &name, const std::string &path, int version)
        : Config(name, path, version) {

    //http://divers.klikissi.fr/telechargements/
    Group group("HTTP_SERVERS");
    group.addOption({"SERVER0", "http://divers.klikissi.fr/telechargements/"});
    group.addOption({"SERVER1", "http://divers.klikissi.fr/telechargements/"});
    group.addOption({"SERVER2", "http://divers.klikissi.fr/telechargements/"});
    group.addOption({"SERVER3", "http://divers.klikissi.fr/telechargements/"});
    group.addOption({"SERVER4", "http://divers.klikissi.fr/telechargements/"});
    addGroup(group);

    // load the configuration from file, overwriting default values
    if (!load()) {
        // file doesn't exist or is malformed, (re)create it
        save();
    }
}
