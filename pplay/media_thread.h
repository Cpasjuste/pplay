//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_THREAD_H
#define PPLAY_MEDIA_THREAD_H

#include <string>
#include <SDL_thread.h>
#include "cross2d/skeleton/io.h"

class Main;

class MediaInfo;

class MediaThread {

public:

    explicit MediaThread(Main *main, const std::string &cachePath);

    ~MediaThread();

    Main *getMain();

    const std::string getMediaCachePath(const std::string &mediaPath) const;

    const MediaInfo getMediaInfo(const c2d::Io::File &file, bool fromCache = false);

    SDL_mutex *getMutex();

    bool isRunning();

    bool isCaching();

    std::vector<c2d::Io::File> mediaList;

private:

    Main *main;
    std::string cachePath;
    SDL_Thread *thread;
    SDL_mutex *mutex;
    bool running = true;
    bool cache = true;
};

#endif //PPLAY_MEDIA_INFO_H
