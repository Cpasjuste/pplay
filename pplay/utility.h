//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_UTILITY_H
#define PPLAY_UTILITY_H

#include <string>

namespace pplay {

    class Utility {

    public:

        static bool isMedia(const c2d::Io::File &file);

        static std::string formatTime(double seconds);

        static std::string formatTimeShort(double seconds);
    };
}

#endif //PPLAY_UTILITY_H
