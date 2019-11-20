//
// Created by cpasjuste on 29/03/19.
//

#ifndef PPLAY_SCRAPPER_H
#define PPLAY_SCRAPPER_H

#include <SDL2/SDL_thread.h>
#include "cross2d/skeleton/sfml/RectangleShape.hpp"

class Main;

namespace pplay {

    class Scrapper {

    public:

        explicit Scrapper(Main *main);

        ~Scrapper();

        int scrap(const std::string &path);

        Main *main;
        std::string path;
        SDL_mutex *mutex = nullptr;
        SDL_cond *cond = nullptr;
        SDL_Thread *thread = nullptr;
        bool scrapping = false;
        bool running = true;
    };
}

#endif //PPLAY_SCRAPPER_H
