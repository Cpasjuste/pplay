//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_THREAD_H
#define PPLAY_MEDIA_THREAD_H

#include <string>
#include <SDL_thread.h>
#include "cross2d/skeleton/io.h"

class Main;

class Media;

class MediaThread {

public:

    explicit MediaThread(Main *main, const std::string &cachePath);

    ~MediaThread();

    Main *getMain();

    const std::string getMediaCachePath(const std::string &mediaPath) const;

    const Media getMediaInfo(const c2d::Io::File &file);

    void cacheDir(const std::string &dir);

    SDL_mutex *getMutex();

    bool isRunning();

    std::vector<std::string> mediaList;

private:

    Main *main;
    std::string cache;
    SDL_Thread *thread;
    SDL_mutex *mutex;
    bool running = true;
};

#endif //PPLAY_MEDIA_INFO_H
