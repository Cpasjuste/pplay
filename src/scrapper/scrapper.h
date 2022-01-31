//
// Created by cpasjuste on 29/03/19.
//

#ifndef PPLAY_SCRAPPER_H
#define PPLAY_SCRAPPER_H

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
        c2d::C2DMutex *mutex = nullptr;
        c2d::C2DCond *cond = nullptr;
        c2d::C2DThread *thread = nullptr;
        bool scrapping = false;
        bool running = true;
    };
}

#endif //PPLAY_SCRAPPER_H
