//
// Created by cpasjuste on 31/03/19.
//

#include <regex>
#include "io.h"
#include "media_info.h"
#include "Browser/Browser.hpp"

using namespace pplay;

static size_t find_Nth(const std::string &str, unsigned n, const std::string &find) {

    size_t pos = std::string::npos, from = 0;
    unsigned i = 0;

    if (0 == n) {
        return std::string::npos;
    }

    while (i < n) {
        pos = str.find(find, from);
        if (std::string::npos == pos) { break; }
        from = pos + 1;
        ++i;
    }
    return pos;
}

Io::Io() : c2d::C2DIo() {

    // http io
    browser = new Browser();
    browser->set_handle_gzip(true);
    browser->set_handle_redirect(true);
    browser->set_handle_ssl(false);
    browser->fetch_forms(false);
}

std::vector<c2d::Io::File> Io::getDirList(const pplay::Io::DeviceType &type, const std::vector<std::string> &extensions,
                                          const std::string &path, bool sort, bool showHidden) {

    std::vector<c2d::Io::File> files;

    printf("Io::getDir(%s)\n", path.c_str());

    if (type == DeviceType::Sdmc) {
        files = c2d::C2DIo::getDirList(path, sort, showHidden);
    } else if (type == DeviceType::Http) {
        std::string http_path = path;
        if (!c2d::Utility::endsWith(http_path, "/")) {
            http_path += "/";
        }
        // extract home from path
        size_t pos = find_Nth(http_path, 3, "/");
        std::string home = http_path.substr(0, pos + 1);
        std::string dir = browser->escape(http_path.substr(pos + 1, http_path.length() - 1));
        dir = std::regex_replace(dir, std::regex("%2F"), "/");
        //printf("home: %s | dir: %s\n", home.c_str(), dir.c_str());
        browser->open(home + dir, 3);
        if (browser->error() || browser->links.size() < 1) {
            return files;
        }
        // add up/back ("..")
        files.emplace_back("..", "..", Io::Type::Directory, 0, c2d::Color::Blue);

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

            Io::Type t = c2d::Utility::endsWith(browser->links[i].name(), "/") ?
                         Io::Type::Directory : Io::Type::File;
            std::string name = browser->unescape(browser->links[i].name());
            if (c2d::Utility::endsWith(name, "/")) {
                name = c2d::Utility::removeLastSlash(name);
            }
            files.emplace_back(name, http_path + name, t);
        }
        if (sort) {
            std::sort(files.begin(), files.end(), compare);
        }
    }

    // remove items by extensions, if provided
    if (!extensions.empty()) {
        files.erase(
                std::remove_if(files.begin(), files.end(), [extensions](const Io::File& file) {
                    for (auto &ext : extensions) {
                        if (c2d::Utility::endsWith(file.name, ext, false)) {
                            return false;
                        }
                    }
                    return file.type == c2d::Io::Type::File;
                }), files.end());
    }

    return files;
}

Io::DeviceType Io::getType(const std::string &path) const {

    Io::DeviceType type = Io::DeviceType::Sdmc;

    if (c2d::Utility::startWith(path, "http://")) {
        type = pplay::Io::DeviceType::Http;
    } else if (c2d::Utility::startWith(path, "ftp://")) {
        type = pplay::Io::DeviceType::Ftp;
    }

    return type;
}

Io::~Io() {
    delete (browser);
}
