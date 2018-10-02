//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"

using namespace c2d;

Filer::Filer(c2d::Io *io, const std::string &path, const c2d::Font &font,
             int fontSize, const c2d::FloatRect &rect) : Rectangle(rect) {

    this->io = io;
    this->path = path;
    this->setFillColor(Color::Transparent);

    // create current path box
    pathRect = new C2DRectangle(FloatRect(0, 0, rect.width, fontSize + 10));
    pathRect->setFillColor(COLOR_BG_1);
    pathRect->setOutlineColor(Color::Orange);
    pathRect->setOutlineThickness(2);
    pathText = new C2DText("CURRENT PATH: /", font, (unsigned int) fontSize);
    pathText->setOutlineThickness(2);
    pathText->setOrigin(0, pathText->getLocalBounds().height / 2);
    pathText->setPosition(4, (pathRect->getSize().y / 2) - 2);
    pathText->setSizeMax(rect.width - 8, 0);
    pathRect->add(pathText);
    add(pathRect);

    float y = pathRect->getGlobalBounds().top + pathRect->getGlobalBounds().height;
    FloatRect listBoxRect = {0, y, rect.width, rect.height - y};
    listBox = new ListBox(font, fontSize, listBoxRect, std::vector<Io::File>());
    add(listBox);

    getDir(path);
}

bool Filer::getDir(const std::string &p) {

    if (io->getType(p) != Io::Type::Directory) {
        printf("getDir(%s): not a directory\n", p.c_str());
        return false;
    }

    printf("getDir(%s)\n", p.c_str());

    path = p;
    index = 0;
    files = io->getDirList(path, true);
    // change colors...
    for (auto &file : files) {
        file.color = file.type ==
                     Io::Type::Directory ? Color::Red : Color::Blue;
    }

    listBox->setFiles(files);
    listBox->setSelection(0);

    pathText->setString(this->path);

    return true;
}

std::string Filer::getPath() {
    return path;
}

void Filer::down() {
    index++;
    if (index >= (int) listBox->getFiles().size()) {
        index = 0;
    }
    listBox->setSelection(index);
}

void Filer::up() {
    index--;
    if (index < 0)
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

void Filer::left() {
    index -= listBox->getMaxLines();
    if (index < 0)
        index = 0;
    listBox->setSelection(index);
}

void Filer::right() {
    index += listBox->getMaxLines();
    if (index >= (int) listBox->getFiles().size())
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

void Filer::enter() {

    if (listBox->getSelection().name == "..") {
        exit();
        return;
    }

    if (path == "/") {
        getDir(path + listBox->getSelection().name);
    } else {
        getDir(path + "/" + listBox->getSelection().name);
    }
}

void Filer::exit() {

    if (path == "/" || path.find('/') == std::string::npos) {
        return;
    }

    while (path.back() != '/') {
        path.erase(path.size() - 1);
    }

    if (path.size() > 1 && endWith(path, "/")) {
        path.erase(path.size() - 1);
    }

    getDir(path);
}

bool Filer::endWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

Filer::~Filer() {
    files.clear();
}
