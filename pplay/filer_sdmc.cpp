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

    // cache media infos
    main->getMediaThread()->cacheDir(p);

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
        return false;
    }

    _files.erase(std::remove_if(_files.begin(), _files.end(), [](Io::File file) {
        return file.type == Io::Type::File && !pplay::Utility::isMedia(file);
    }), _files.end());

#ifdef __SWITCH__
    files.emplace_back(Io::File{"..", "..", Io::Type::Directory, 0, COLOR_BLUE});
#endif

    for (auto &file : _files) {
        files.emplace_back(file, main->getMediaThread()->getMediaInfo(file));
    }

    setSelection(0);

    return true;
}

void FilerSdmc::enter() {

    MediaFile file = getSelection();

    if (file.name == "..") {
        exit();
        return;
    }

    if (path == "/") {
        getDir(path + file.name);
    } else {
        getDir(path + "/" + file.name);
    }
}

void FilerSdmc::exit() {

    if (path == "/" || path.find('/') == std::string::npos) {
        return;
    }

    while (path.back() != '/') {
        path.erase(path.size() - 1);
    }

    if (path.size() > 1 && Utility::endsWith(path, "/")) {
        path.erase(path.size() - 1);
    }

    getDir(path);
}
