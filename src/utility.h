//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_UTILITY_H
#define PPLAY_UTILITY_H

#include <string>
#include "cross2d/skeleton/io.h"

namespace pplay {

    class Utility {

    public:

        enum class CpuClock {
            Min = 0,
            Max = 1
        };

        static std::string getMediaInfoPath(const c2d::Io::File &file);

        static std::string getMediaScrapPath(const c2d::Io::File &file);

        static std::string getMediaPosterPath(const c2d::Io::File &file);

        static std::string getMediaBackdropPath(const c2d::Io::File &file);

        static std::vector<std::string> getMediaExtensions();

        static bool isMedia(const c2d::Io::File &file);

        static std::string formatTime(double seconds);

        static std::string formatTimeShort(double seconds);

        static std::string formatSize(size_t size);

        static void setCpuClock(const CpuClock &clock);
    };
}

#endif //PPLAY_UTILITY_H
