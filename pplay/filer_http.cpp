//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_http.h"
#include "Browser/Browser.hpp"
#include "utility.h"

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
        browser->open(p, 3);
        if (browser->error() || browser->links.size() < 1) {
            return false;
        }
    }

    item_index = 0;
    files.clear();
    path = browser->unescape(browser->geturl());

    // add up/back ("..")
    files.emplace_back(Io::File("..", "..", Io::Type::Directory, 0, COLOR_BLUE_LIGHT));

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
            std::string str = browser->geturl() + browser->links[i].url();
            Io::File file(Utility::removeLastSlash(browser->links[i].name()), str, type, 0, color);
            if (pplay::Utility::isMedia(file)) {
                files.emplace_back(file);
            }
        } else {
            Color color = COLOR_BLUE_LIGHT;
            files.emplace_back(Io::File(Utility::removeLastSlash(browser->links[i].name()),
                                        browser->links[i].url(), type, 0, color));
        }
    }

    setSelection(0);

    return true;
}

void FilerHttp::enter() {

    MediaFile file = getSelection();

    if (file.name == "..") {
        exit();
        return;
    }

    browser->follow_link(browser->unescape(file.path));
    getDir(browser->geturl());
}

void FilerHttp::exit() {

    if (browser->get_history().size() > 1) {
        browser->back(10);
        getDir(browser->geturl());
    }
}

FilerHttp::~FilerHttp() {
    delete (browser);
}
