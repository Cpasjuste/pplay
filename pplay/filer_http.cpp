//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_http.h"
#include "Browser/Browser.hpp"

using namespace c2d;

HttpFiler::HttpFiler(c2d::Io *io, const std::string &path, const c2d::Font &font,
                     int fontSize, const c2d::FloatRect &rect) : C2DRectangle(rect) {

    this->io = io;
    this->setFillColor(Color::Transparent);

    // create current path box
    pathRect = new C2DRectangle({rect.width, fontSize + 10});
    pathRect->setFillColor(COLOR_GRAY_DARK);
    pathRect->setOutlineColor(COLOR_BLUE);
    pathRect->setOutlineThickness(2);
    pathText = new C2DText("CURRENT PATH: /", font, (unsigned int) fontSize);
    pathText->setOutlineThickness(2);
    pathText->setOrigin(Origin::Left);
    pathText->setPosition(4, (pathRect->getSize().y / 2));
    pathText->setSizeMax(rect.width - 8, 0);
    pathRect->add(pathText);
    add(pathRect);

    C2DRectangle *border = new C2DRectangle(pathRect->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(pathRect->getSize().x + 4, pathRect->getSize().y + 4);
    pathRect->add(border);

    float y = pathRect->getGlobalBounds().top + pathRect->getGlobalBounds().height;
    FloatRect r = {0, y + 8, rect.width, rect.height - y - 8};
    listBox = new ListBox(font, fontSize, r, std::vector<Io::File>());
    listBox->setFillColor(COLOR_GRAY_DARK);
    listBox->setOutlineColor(COLOR_BLUE);
    listBox->setOutlineThickness(2);
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
    border->setSize(r.width + 4, r.height + 4);
    listBox->add(border);

    add(listBox);

    // http "Browser"
    browser = new Browser();
    browser->set_handle_gzip(true);
    browser->set_handle_redirect(true);
    browser->set_handle_ssl(false);
    browser->fetch_forms(false);

    getDir(path);
}

bool HttpFiler::getDir(const std::string &p, bool reopen) {

    printf("getDir(%s)\n", p.c_str());

    if (reopen) {
        browser->open(p, 5);
        if (browser->error() || browser->links.size() < 1) {
            return false;
        }
    }

    index = 0;
    files.clear();

    // add up/back ("..")
    files.emplace_back("..", "..", Io::Type::Directory, COLOR_BLUE_LIGHT);

    for (int i = 0; i < browser->links.size(); i++) {

        // skip apache2 stuff
        if (browser->links[i].name() == "Name"
            || browser->links[i].name() == "Last modified"
            || browser->links[i].name() == "Size"
            || browser->links[i].name() == "Description"
            || browser->links[i].name() == "Parent Directory") {
            continue;
        }

        Io::Type type = Utility::endsWith(browser->links[i].url(), "/") ?
                        Io::Type::Directory : Io::Type::File;
        Color color = type == Io::Type::Directory ?
                      COLOR_BLUE_LIGHT : Color::White;
        files.emplace_back(Utility::removeLastSlash(browser->links[i].name()),
                           browser->links[i].url(), type, color);
    }

    listBox->setFiles(files);
    listBox->setSelection(0);
    pathText->setString(browser->unescape(browser->geturl()));

    return true;
}

std::string HttpFiler::getPath() {
    return browser->geturl();
}

bool HttpFiler::step(unsigned int keys) {

    if (keys & Input::Key::KEY_UP) {
        up();
    } else if (keys & Input::Key::KEY_DOWN) {
        down();
    } else if (keys & Input::Key::KEY_RIGHT) {
        right();
    } else if (keys & Input::Key::KEY_LEFT) {
        left();
    } else if (keys & Input::Key::KEY_FIRE1) {
        if (getSelection().type == Io::Type::File) {
            return true;
        }
        enter();
    } else if (keys & Input::Key::KEY_FIRE2) {
        exit();
    }

    return false;
}

c2d::Io::File HttpFiler::getSelection() {
    return listBox->getSelection();
}

void HttpFiler::down() {
    index++;
    if (index >= (int) listBox->getFiles().size()) {
        index = 0;
    }
    listBox->setSelection(index);
}

void HttpFiler::up() {
    index--;
    if (index < 0)
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

void HttpFiler::left() {
    index -= listBox->getMaxLines();
    if (index < 0)
        index = 0;
    listBox->setSelection(index);
}

void HttpFiler::right() {
    index += listBox->getMaxLines();
    if (index >= (int) listBox->getFiles().size())
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

void HttpFiler::enter() {

    Io::File file = listBox->getSelection();

    if (file.name == "..") {
        exit();
        return;
    }

    if (file.type == Io::Type::File) {
        return;
    }

    printf("follow: %s\n", browser->unescape(file.path).c_str());
    browser->follow_link(browser->unescape(file.path));
    getDir(browser->geturl(), false);
}

void HttpFiler::exit() {

    if (browser->get_history().size() > 1) {
        browser->back(10);
        getDir(browser->geturl(), false);
    }
}

HttpFiler::~HttpFiler() {
    files.clear();
    delete (browser);
}
