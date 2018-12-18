//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "cross2d/skeleton/io.h"
#include "cross2d/skeleton/utility.h"
#include "utility.h"

using namespace pplay;

bool Utility::isMedia(const c2d::Io::File &file) {

    return file.type == c2d::Io::Type::File &&
           (c2d::Utility::endsWith(file.name, ".8svx", false)
            || c2d::Utility::endsWith(file.name, ".aac", false)
            || c2d::Utility::endsWith(file.name, ".ac3", false)
            || c2d::Utility::endsWith(file.name, ".aif", false)
            || c2d::Utility::endsWith(file.name, ".asf", false)
            || c2d::Utility::endsWith(file.name, ".avi", false)
            || c2d::Utility::endsWith(file.name, ".dv", false)
            || c2d::Utility::endsWith(file.name, ".flv", false)
            || c2d::Utility::endsWith(file.name, ".m2ts", false)
            || c2d::Utility::endsWith(file.name, ".m2v", false)
            || c2d::Utility::endsWith(file.name, ".m4a", false)
            || c2d::Utility::endsWith(file.name, ".mkv", false)
            || c2d::Utility::endsWith(file.name, ".mov", false)
            || c2d::Utility::endsWith(file.name, ".mp3", false)
            || c2d::Utility::endsWith(file.name, ".mp4", false)
            || c2d::Utility::endsWith(file.name, ".mpeg", false)
            || c2d::Utility::endsWith(file.name, ".mpg", false)
            || c2d::Utility::endsWith(file.name, ".mts", false)
            || c2d::Utility::endsWith(file.name, ".ogg", false)
            || c2d::Utility::endsWith(file.name, ".swf", false)
            || c2d::Utility::endsWith(file.name, ".ts", false)
            || c2d::Utility::endsWith(file.name, ".vob", false)
            || c2d::Utility::endsWith(file.name, ".wav", false)
            || c2d::Utility::endsWith(file.name, ".wma", false)
            || c2d::Utility::endsWith(file.name, ".wmv", false)
           );
}

std::string Utility::formatTime(double seconds) {

    if (seconds <= 0) {
        return "00:00:00";
    }

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

static std::string convertToString(double num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}

static double roundOff(double n) {
    double d = n * 100.0;
    int i = (int) lround(d);//(int) (d + 0.5);
    d = (float) i / 100.0;
    return d;
}

std::string Utility::formatSize(size_t size) {

    static const char *sizes[] = {"B", "KB", "MB", "GB"};
    int div = 0;
    size_t rem = 0;

    while (size >= 1024 && (size_t) div < (sizeof sizes / sizeof *sizes)) {
        rem = (size_t) (size % 1024);
        div++;
        size /= 1024;
    }

    double size_d = (float) size + (float) rem / 1024.0;
    return convertToString(roundOff(size_d)) + " " + sizes[div];
}
