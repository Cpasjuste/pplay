//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"

using namespace c2d;

Filer::Filer(c2d::Io *io, const std::string &path, const c2d::Font &font,
             int fontSize, const c2d::FloatRect &rect) : C2DRectangle(rect) {

    this->io = io;
    this->path = path;
    this->setFillColor(Color::Transparent);

    // create current path box
    pathRect = new C2DRectangle({rect.width, fontSize + 10});
    pathRect->setFillColor(COLOR_GRAY_DARK);
    pathRect->setOutlineColor(COLOR_BLUE);
    pathRect->setOutlineThickness(4);
    pathText = new C2DText("CURRENT PATH: /", font, (unsigned int) fontSize);
    pathText->setOutlineThickness(2);
    pathText->setOrigin(Origin::Left);
    pathText->setPosition(4, (pathRect->getSize().y / 2));
    pathText->setSizeMax(rect.width - 8, 0);
    pathRect->add(pathText);

    C2DRectangle *border = new C2DRectangle(pathRect->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(pathRect->getSize().x + 8, pathRect->getSize().y + 8);
    pathRect->add(border);

    border = new C2DRectangle(pathRect->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(pathRect->getSize().x - 2, pathRect->getSize().y - 2);
    border->move(5, 5);
    pathRect->add(border);

    add(pathRect);

    float y = pathRect->getGlobalBounds().top + pathRect->getGlobalBounds().height;
    FloatRect r = {0, y + 8, rect.width, rect.height - y - 8};
    listBox = new ListBox(font, fontSize, r, std::vector<Io::File>());
    listBox->setFillColor(COLOR_GRAY_DARK);
    listBox->setOutlineColor(COLOR_BLUE);
    listBox->setOutlineThickness(4);
    listBox->setTextOutlineThickness(2);
    listBox->setHighlightThickness(2);
    listBox->setHighlightColor(COLOR_GRAY_LIGHT);
    listBox->setHighlightOutlineColor(COLOR_BLUE);
    listBox->setHighlightUseFileColor(false);
    auto *tween = new TweenAlpha(80, 150, 0.6f, TweenLoop::PingPong);
    listBox->setHighlightTween(tween);

    border = new RectangleShape(listBox->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(r.width + 8, r.height + 8);
    listBox->add(border);

    border = new RectangleShape(listBox->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(r.width - 2, r.height - 2);
    border->move(5, 5);
    listBox->add(border);

    add(listBox);

    getDir(path);
}

bool Filer::getDir(const std::string &p) {

    if (io->getType(p) != Io::Type::Directory) {
        return false;
    }

    printf("getDir(%s)\n", p.c_str());

    path = p;
    index = 0;
    files = io->getDirList(path, true);
    if (files.empty()) {
        Io::File file;
        file.type = Io::Type::Directory;
        file.name = "..";
        files.push_back(file);
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

std::string Filer::getPath() {
    return path;
}

Io::File Filer::processInput(unsigned int keys) {

    if (keys & Input::Key::KEY_UP) {
        up();
    } else if (keys & Input::Key::KEY_DOWN) {
        down();
    } else if (keys & Input::Key::KEY_RIGHT) {
        right();
    } else if (keys & Input::Key::KEY_LEFT) {
        left();
    } else if (keys & Input::Key::KEY_FIRE1) {
        enter();
    } else if (keys & Input::Key::KEY_FIRE2) {
        exit();
    }

    return listBox->getSelection();
}

c2d::Io::File Filer::getSelection() {
    return listBox->getSelection();
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

bool Filer::endWith(std::string const &str, std::string const &ending) {
    if (str.length() >= ending.length()) {
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

Filer::~Filer() {
    files.clear();
}
