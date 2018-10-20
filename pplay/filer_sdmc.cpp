//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer_sdmc.h"

using namespace c2d;

FilerSdmc::FilerSdmc(Io *io, const std::string &path,
                     Font *font, int fontSize, const FloatRect &rect)
        : Filer(path, font, fontSize, rect) {

    this->io = io;

    if (!getDir(path)) {
        getDir("/");
    }
}

bool FilerSdmc::getDir(const std::string &p) {

    if (io->getType(p) != Io::Type::Directory) {
        return false;
    }

    printf("getDir(%s)\n", p.c_str());

    path = p;
    index = 0;
    files = io->getDirList(path, true);
    if (files.empty()) {
        // add up/back ("..")
        files.emplace_back("..", "..", Io::Type::Directory, COLOR_BLUE_LIGHT);
        return false;
    }

    for (auto &file : files) {
        file.color = file.type == Io::Type::Directory ?
                     COLOR_BLUE_LIGHT : Color::White;
    }

    listBox->setFiles(files);
    listBox->setSelection(0);
    pathText->setString(this->path);

    return true;
}

void FilerSdmc::enter() {

    Io::File file = listBox->getSelection();

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
