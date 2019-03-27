//
// Created by cpasjuste on 12/04/18.
//

#ifdef __SMB_SUPPORT__
#include <iostream>
#include <arpa/inet.h>

extern "C" {
#include <bdsm/bdsm.h>
}

#include "main.h"
#include "filer_smb.h"
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

FilerSmb::FilerSmb(Main *main, const FloatRect &rect) : Filer(main, "", rect) {

}

bool FilerSmb::getDir(const std::string &p) {

    struct sockaddr_in addr{};
    smb_session *smb_session;
    char **smb_share_list;
    smb_file *smb_files;
    smb_stat smb_st;

    printf("getDir(%s)\n", p.c_str());

    // split user/pwd/host/path
    // TODO: check for nullptr etc..
    size_t colon_2 = find_Nth(p, 2, ":");
    size_t at = p.find_last_of('@');
    size_t last_slash = find_Nth(p, 3, "/");
    std::string user = p.substr(6, colon_2 - 6);
    std::string pwd = p.substr(colon_2 + 1, at - colon_2 - 1);
    std::string host = p.substr(at + 1, last_slash - at - 1);
    std::string new_path = p.substr(last_slash, p.length() - last_slash);
    if (Utility::startWith(new_path, "/")) {
        new_path.erase(0, 1);
    }
    if (Utility::endsWith(new_path, "\\")) {
        new_path.erase(new_path.length() - 1, 1);
    }

    //printf("user: %s, pwd: %s, host: %s, path: %s\n",
    //       user.c_str(), pwd.c_str(), host.c_str(), new_path.c_str());

    inet_aton(host.c_str(), &addr.sin_addr);

    smb_session = smb_session_new();
    if (smb_session_connect(smb_session, host.c_str(), addr.sin_addr.s_addr, SMB_TRANSPORT_TCP) != DSM_SUCCESS) {
        error = "Could not connect to smb server";
        smb_session_destroy(smb_session);
        return false;
    }

    // TODO: add domain
    smb_session_set_creds(smb_session, "localhost", user.c_str(), pwd.c_str());
    if (smb_session_login(smb_session) != DSM_SUCCESS) {
        error = "Could not authenticate to smb server";
        smb_session_destroy(smb_session);
        return false;
    }

    if (new_path.empty()) {
        if (smb_share_get_list(smb_session, &smb_share_list, nullptr) != DSM_SUCCESS) {
            error = "Could not list smb shares";
            smb_session_destroy(smb_session);
            return false;
        }
        // OK
        item_index = 0;
        files.clear();
        path = p;
        if (!Utility::endsWith(path, "/")) {
            path += "/";
        }
        for (size_t i = 0; smb_share_list[i] != nullptr; i++) {
            std::string name = smb_share_list[i];
            Io::File file(name, path + name, Io::Type::Directory, 0, COLOR_BLUE);
            files.emplace_back(file, MediaInfo{});
        }
        smb_share_list_destroy(smb_share_list);
        smb_session_destroy(smb_session);
    } else {
        std::string share_path = new_path;
        std::string file_path = "\\*";
        // split real path from share if needed
        size_t pos = new_path.find_first_of('\\');
        if (pos != std::string::npos) {
            share_path = new_path.substr(0, pos);
            file_path = new_path.substr(pos, new_path.length() - pos) + "\\*";
        }
        printf("share: %s, path: %s\n", share_path.c_str(), file_path.c_str());

        // connect to share
        smb_tid tid;
        if (smb_tree_connect(smb_session, share_path.c_str(), &tid) != DSM_SUCCESS) {
            error = "Could not connect smb share";
            smb_session_destroy(smb_session);
            return false;
        }
        // list files
        smb_files = smb_find(smb_session, tid, file_path.c_str());
        size_t files_count = smb_stat_list_count(smb_files);
        if (files_count <= 0) {
            error = "Could not list smb files";
            smb_session_destroy(smb_session);
            return false;
        } else {
            // OK
            item_index = 0;
            files.clear();
            path = p;
            if (!Utility::endsWith(path, "\\")) {
                path += "\\";
            }
            for (size_t i = 0; i < files_count; i++) {
                smb_st = smb_stat_list_at(smb_files, i);
                if (smb_st == nullptr) {
                    error = "Could not stat smb files";
                    smb_stat_list_destroy(smb_files);
                    smb_session_destroy(smb_session);
                    return false;
                }
                std::string name = smb_stat_name(smb_st);
                if (name == ".") {
                    continue;
                }
                size_t size = smb_stat_get(smb_st, SMB_STAT_SIZE);
                Io::Type type = smb_stat_get(smb_st, SMB_STAT_ISDIR) ? Io::Type::Directory : Io::Type::File;
                Io::File file(name, path + name, type, size);
                printf("file.path: %s\n", file.path.c_str());
                files.emplace_back(file, MediaInfo{});
            }
            std::sort(files.begin(), files.end(), Io::compare);
        }
        smb_stat_list_destroy(smb_files);
        smb_session_destroy(smb_session);
    }

    return true;
}

void FilerSmb::enter(int prev_index) {

    MediaFile file = getSelection();

    if (file.name == "..") {
        exit();
        return;
    }

    if (getDir(file.path)) {
        Filer::enter(prev_index);
    }
}

void FilerSmb::exit() {

    if (item_index_prev.empty()) {
        return;
    }

    std::string path_new = path;
    if (Utility::endsWith(path_new, "\\")) {
        path_new.erase(path_new.length() - 1, 1);
    }

    if (path_new.find('\\') == std::string::npos) {
        return;
    }

    while (path_new.back() != '\\') {
        path_new.erase(path_new.size() - 1);
    }

    if (getDir(path_new)) {
        Filer::exit();
    }
}

const std::string FilerSmb::getError() {
    return error;
}

FilerSmb::~FilerSmb() {
}
#endif // __SMB_SUPPORT__
