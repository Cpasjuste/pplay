//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_THREAD_H
#define PPLAY_MEDIA_THREAD_H

#include <string>
#include <SDL_thread.h>

#include "cross2d/skeleton/renderer.h"
#include "media.h"

class MediaThread {

public:

    explicit MediaThread(c2d::Renderer *renderer, const std::string &cachePath);

    ~MediaThread();

    c2d::Renderer *getRenderer();

    const std::string getMediaCachePath(const std::string &mediaPath) const;

    Media *getMediaInfo(const std::string &mediaPath);

    void cacheDir(const std::string &dir);

    SDL_mutex *getMutex();

    bool isRunning();

    std::vector<std::string> mediaList;

private:

    c2d::Renderer *renderer;
    std::string cache;
    SDL_Thread *thread;
    SDL_mutex *mutex;
    bool running = true;

};

#endif //PPLAY_MEDIA_INFO_H
