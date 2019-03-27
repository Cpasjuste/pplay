//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer_sdmc.h"
#include "filer_item.h"
#include "utility.h"

using namespace c2d;

FilerSdmc::FilerSdmc(Main *main, const std::string &path, const FloatRect &rect) : Filer(main, path, rect) {
}

bool FilerSdmc::getDir(const std::string &p) {

    if (main->getIo()->getType(p) != Io::Type::Directory) {
        return false;
    }

    printf("getDir(%s)\n", p.c_str());

    item_index = 0;
    files.clear();
    path = p;
    if (path.size() > 1) {
        path = Utility::removeLastSlash(path);
    }

    std::vector<Io::File> _files = main->getIo()->getDirList(path, true);
    if (_files.empty()) {
        // add up/back ("..")
        _files.emplace_back("..", "..", Io::Type::Directory, 0, COLOR_BLUE);
        //return true;
    }

    _files.erase(std::remove_if(_files.begin(), _files.end(), [](Io::File file) {
        return file.type == Io::Type::File && !pplay::Utility::isMedia(file);
    }), _files.end());

#ifdef __SWITCH__
    Io::File file("..", "..", Io::Type::Directory, 0, COLOR_BLUE);
    files.emplace_back(file, MediaInfo(file));
#endif
    for (auto &file : _files) {
        files.emplace_back(file, MediaInfo(file));
    }

    setSelection(0);

    return true;
}

void FilerSdmc::enter(int index) {

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
        Filer::enter(index);
    }
}

void FilerSdmc::exit() {

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
