//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "cross2d/c2d.h"

#ifdef __SWITCH__

#include "cross2d/platforms/switch/switch_sys.h"

#endif

#include "utility.h"

using namespace pplay;

std::string Utility::getMediaInfoPath(const c2d::Io::File &file) {
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    return c2d_renderer->getIo()->getDataPath() + "cache/" + hash + ".info";
}

std::string Utility::getMediaScrapPath(const c2d::Io::File &file) {
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    return c2d_renderer->getIo()->getDataPath() + "cache/" + hash + ".scrap";
}

std::string Utility::getMediaPosterPath(const c2d::Io::File &file) {
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    return c2d_renderer->getIo()->getDataPath() + "cache/" + hash + "-poster.jpg";
}

std::string Utility::getMediaBackdropPath(const c2d::Io::File &file) {
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    return c2d_renderer->getIo()->getDataPath() + "cache/" + hash + "-backdrop.jpg";
}

std::vector<std::string> Utility::getMediaExtensions() {
    return {
            ".8svx",
            ".aac",
            ".ac3",
            ".aif",
            ".asf",
            ".avi",
            ".dv",
            ".flv",
            ".m2ts",
            ".m2v",
            ".m4a",
            ".mkv",
            ".mov",
            ".mp3",
            ".mp4",
            ".mpeg",
            ".mpg",
            ".mts",
            ".ogg",
            ".rmvb",
            ".swf",
            ".ts",
            ".vob",
            ".wav",
            ".wma",
            ".wmv",
            ".m3u",
            ".m3u8"
    };
}

bool Utility::isMedia(const c2d::Io::File &file) {

    if (file.type == c2d::Io::Type::File) {
        std::vector<std::string> extensions = getMediaExtensions();
        for (auto &ext: extensions) {
            if (c2d::Utility::endsWith(file.name, ext, false)) {
                return true;
            }
        }
    }

    return false;
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

c2d::Vector2f Utility::ceil(const c2d::Vector2f &v) {
    return {std::ceil(v.x), std::ceil(v.y)};
}

c2d::Vector2f Utility::ceil(float x, float y) {
    return ceil({x, y});
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

void Utility::setCpuClock(const CpuClock &clock) {
#ifdef __SWITCH__
    if (clock == CpuClock::Min) {
        if (c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu) !=
            c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu, true)) {
            int clock_old = c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu);
            c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Cpu, (int) c2d::SwitchSys::CPUClock::Stock);
            c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Gpu, (int) c2d::SwitchSys::GPUClock::Stock);
            c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Emc, (int) c2d::SwitchSys::EMCClock::Stock);
            printf("restoring cpu speed (old: %i, new: %i)\n",
                   clock_old, c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu));
        }
    } else {
        int clock_old = c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu);
        c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Cpu, (int) c2d::SwitchSys::CPUClock::Max);
        c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Gpu, (int) c2d::SwitchSys::GPUClock::Max);
        c2d::SwitchSys::setClock(c2d::SwitchSys::Module::Emc, (int) c2d::SwitchSys::EMCClock::Max);
        printf("setting max cpu speed (old: %i, new: %i)\n",
               clock_old, c2d::SwitchSys::getClock(c2d::SwitchSys::Module::Cpu));
    }
#endif
}


