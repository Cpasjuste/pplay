//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_ftp.h"
#include "utility.h"

using namespace c2d;

#define TIMEOUT 3

FilerFtp::FilerFtp(Main *main, const FloatRect &rect) : Filer(main, "", rect) {
    FtpInit();
}

bool FilerFtp::getDir(const std::string &p) {

    printf("getDir(%s)\n", p.c_str());

    std::string srv_address = main->getConfig()->getOption(OPT_NETWORK)->getString();

    if (!FtpConnect(srv_address.c_str(), &ftp_con)) {
        error = "Could not connect to ftp server";
        return false;
    }

    if (!FtpLogin("cpasjuste", "test", ftp_con)) {
        error = "Could not connect to ftp server";
        FtpQuit(ftp_con);
        return false;
    }

    item_index = 0;
    files.clear();
    path = p;

    std::vector<Io::File> _files = FtpDirList("dev/videos", ftp_con);
    _files.insert(_files.begin(), Io::File("..", "..", Io::Type::Directory, 0, COLOR_BLUE));

    _files.erase(std::remove_if(_files.begin(), _files.end(), [](Io::File file) {
        return file.type == Io::Type::File && !pplay::Utility::isMedia(file);
    }), _files.end());

    for (auto &file : _files) {
        if (file.path != "..") {
            file.path = std::string("ftp://") + "cpasjuste" + ":" + "test" + srv_address + "/" + file.path;
        }
        bool fromCache = main->getMediaThread()->isCaching() == 0;
        files.emplace_back(file, main->getMediaThread()->getMediaInfo(file, fromCache));
    }

    FtpQuit(ftp_con);

    return true;
}

void FilerFtp::enter(int prev_index) {

    MediaFile file = getSelection();
    bool success;

    if (file.name == "..") {
        exit();
        return;
    }

    if (path == "/") {
        success = getDir(path + file.name);
    } else {
        success = getDir(path + "/" + file.name);
    }
    if (success) {
        Filer::enter(prev_index);
    }
}

void FilerFtp::exit() {

    std::string path_new = path;

    if (path_new == "/" || path_new.find('/') == std::string::npos) {
        return;
    }

    while (path_new.back() != '/') {
        path_new.erase(path_new.size() - 1);
    }

    if (path_new.size() > 1 && Utility::endsWith(path_new, "/")) {
        path_new.erase(path_new.size() - 1);
    }

    if (getDir(path_new)) {
        Filer::exit();
    }
}

const std::string FilerFtp::getError() {
    return error;
}

FilerFtp::~FilerFtp() {
}
