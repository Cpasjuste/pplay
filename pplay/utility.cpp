//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>
#include "cross2d/skeleton/utility.h"

#include "utility.h"

using namespace pplay;

bool Utility::isMedia(const std::string &name) {

    return c2d::Utility::endsWith(name, ".avi")
           || c2d::Utility::endsWith(name, ".mkv")
           || c2d::Utility::endsWith(name, ".flv")
           || c2d::Utility::endsWith(name, ".mp3");
}

std::string Utility::formatTime(double seconds) {

    int h((int) seconds / 3600);
    int min((int) seconds / 60 - h * 60);
    int sec((int) seconds - (h * 60 + min) * 60);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":";
    oss << std::setfill('0') << std::setw(2) << min << ":";
    oss << std::setfill('0') << std::setw(2) << sec;

    return oss.str();
}

std::string Utility::formatTimeShort(double seconds) {

    int h((int) seconds / 3600);
    int min((int) seconds / 60 - h * 60);
    int sec((int) seconds - (h * 60 + min) * 60);

    std::ostringstream oss;
    if (h > 0) {
        oss << std::setfill('0') << std::setw(2) << h << ":";
    }
    if (min > 0) {
        oss << std::setfill('0') << std::setw(2) << min << ":";
    }
    oss << std::setfill('0') << std::setw(2) << sec;

    return oss.str();
}
