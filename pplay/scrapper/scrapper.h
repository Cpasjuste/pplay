//
// Created by cpasjuste on 29/03/19.
//

#ifndef PPLAY_SCRAPPER_H
#define PPLAY_SCRAPPER_H

#include <SDL_thread.h>
#include "cross2d/skeleton/sfml/RectangleShape.hpp"

namespace pplay {

    class Scrapper {

    public:

        explicit Scrapper(const std::string &path);

        void scrap();

        std::string path;
        bool scrapping = false;
        SDL_Thread *thread = nullptr;
    };
}

#endif //PPLAY_SCRAPPER_H
