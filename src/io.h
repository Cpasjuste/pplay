//
// Created by cpasjuste on 31/03/19.
//

#ifndef PPLAY_IO_H
#define PPLAY_IO_H

#include "cross2d/c2d.h"

#ifdef __SMB2__

#include "smb2.h"
#include "libsmb2.h"
#include "libsmb2-raw.h"

#endif

class Browser;

namespace pplay {

    class Io : public c2d::C2DIo {

    public:

        Io();

        ~Io() override;

        enum class DeviceType {
            Local, Http, Ftp, Smb
        };

        std::vector<Io::File> getDirList(const DeviceType &type, const std::vector<std::string> &extensions,
                                         const std::string &path, bool sort = false, bool showHidden = false);

        DeviceType getDeviceType(const std::string &path);

#ifdef __PS4__

        std::string getHomePath() override {
            return "/data/pplay/";
        }

        std::string getDataPath() override {
            return "/data/pplay/";
        }

#ifndef NDEBUG
        std::string getRomFsPath() override {
            return "/data/pplay/";
        }
#else

        std::string getRomFsPath() override {
            return "/app0/";
        }

#endif

#endif

    private:

        Browser *browser;
#ifdef __SMB2__
        smb2_context *smb2;
#endif

    };
}

#endif //PPLAY_IO_H
