//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_http.h"
#include "Browser/Browser.hpp"

using namespace c2d;

FilerHttp::FilerHttp(Main *main, const FloatRect &rect) : Filer(main, "", rect) {

    // http Browser
    browser = new Browser();
    browser->set_handle_gzip(true);
    browser->set_handle_redirect(true);
    browser->set_handle_ssl(false);
    browser->fetch_forms(false);
}

bool FilerHttp::getDir(const std::string &p) {

    printf("getDir(%s)\n", p.c_str());

    if (p != browser->geturl()) {
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
        if (browser->links[i].name() == ".."
            || browser->links[i].name() == "../"
            || browser->links[i].name() == "Name"
            || browser->links[i].name() == "Last modified"
            || browser->links[i].name() == "Size"
            || browser->links[i].name() == "Description"
            || browser->links[i].name() == "Parent Directory") {
            continue;
        }

        Io::Type type = Utility::endsWith(browser->links[i].url(), "/") ?
                        Io::Type::Directory : Io::Type::File;

        if (type == Io::Type::File) {
            Color color = Color::White;
            std::string s = browser->geturl() + browser->links[i].url();
            files.emplace_back(Utility::removeLastSlash(browser->links[i].name()),
                               s, type, color);
        } else {
            Color color = COLOR_BLUE_LIGHT;
            files.emplace_back(Utility::removeLastSlash(browser->links[i].name()),
                               browser->links[i].url(), type, color);
        }
    }

    listBox->setFiles(files);
    listBox->setSelection(0);
    pathText->setString(browser->unescape(browser->geturl()));
    path = browser->unescape(browser->geturl());

    return true;
}

void FilerHttp::enter() {

    Io::File *file = listBox->getSelection();
    if (!file) {
        return;
    }

    if (file->name == "..") {
        exit();
        return;
    }

    browser->follow_link(browser->unescape(file->path));
    getDir(browser->geturl());
}

void FilerHttp::exit() {

    if (browser->get_history().size() > 1) {
        browser->back(10);
        getDir(browser->geturl());
    }
}

Browser *FilerHttp::getBrowser() {
    return browser;
}

FilerHttp::~FilerHttp() {
    delete (browser);
}
