//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_THREAD_H
#define PPLAY_MEDIA_THREAD_H

#include <string>
#include <SDL_thread.h>

#include "cross2d/skeleton/io.h"
#include "media_info.h"

class MediaThread {

public:

    MediaThread();

    ~MediaThread();

    MediaInfo *getInfo(c2d::Io *io, const std::string &srcName, const std::string &srcPath, const std::string &dstPath);

    SDL_mutex *getMutex();

    SDL_cond *getCond();

    bool isRunning();

private:

    SDL_Thread *thread;
    SDL_mutex *mutex;
    SDL_cond *cond;
    bool running = true;

};

#endif //PPLAY_MEDIA_INFO_H
