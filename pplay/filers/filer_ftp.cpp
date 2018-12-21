//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_ftp.h"
#include "utility.h"

using namespace c2d;

#define TIMEOUT 3

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

FilerFtp::FilerFtp(Main *main, const FloatRect &rect) : Filer(main, "", rect) {
    FtpInit();
}

bool FilerFtp::getDir(const std::string &p) {

    printf("getDir(%s)\n", p.c_str());

    // split user/pwd/host/port/path
    // TODO: check for nullptr etc..
    size_t colon_2 = find_Nth(p, 2, ":");
    size_t colon_3 = p.find_last_of(':');
    size_t at = p.find_last_of('@');
    size_t last_slash = find_Nth(p, 3, "/");
    std::string user = p.substr(6, colon_2 - 6);
    std::string pwd = p.substr(colon_2 + 1, at - colon_2 - 1);
    std::string host = p.substr(at + 1, colon_3 - at - 1);
    std::string port = p.substr(colon_3 + 1, p.find('/', colon_3) - (colon_3 + 1));
    std::string host_port = host + ":" + port;
    std::string new_path = p.substr(last_slash, p.length() - last_slash);
    if (Utility::startWith(new_path, "/")) {
        new_path.erase(0, 1);
    }

    //printf("user: %s, pwd: %s, host: %s, port: %s, path: %s\n",
    //       user.c_str(), pwd.c_str(), host.c_str(), port.c_str(), new_path.c_str());

    if (!FtpConnect(host_port.c_str(), &ftp_con)) {
        error = "Could not connect to ftp server";
        return false;
    }

    if (!FtpLogin(user.c_str(), pwd.c_str(), ftp_con)) {
        error = "Could not connect to ftp server";
        FtpQuit(ftp_con);
        return false;
    }

    item_index = 0;
    files.clear();
    path = p;
    if (!Utility::endsWith(path, "/")) {
        path += "/";
    }

    std::vector<Io::File> _files = FtpDirList(new_path.c_str(), ftp_con);
    _files.insert(_files.begin(), Io::File("..", "..", Io::Type::Directory, 0, COLOR_BLUE));

    _files.erase(std::remove_if(_files.begin(), _files.end(), [](Io::File file) {
        return file.type == Io::Type::File && !pplay::Utility::isMedia(file);
    }), _files.end());

    for (auto &file : _files) {
        if (file.path != "..") {
            file.path = path + file.name;
        }
        files.emplace_back(file);
    }

    FtpQuit(ftp_con);

    return true;
}

void FilerFtp::enter(int prev_index) {

    MediaFile file = getSelection();

    if (file.name == "..") {
        exit();
        return;
    }

    if (getDir(file.path)) {
        Filer::enter(prev_index);
    }
}

void FilerFtp::exit() {

    if (item_index_prev.empty()) {
        return;
    }

    std::string path_new = Utility::removeLastSlash(path);
    while (path_new.back() != '/') {
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
