//
// Created by cpasjuste on 02/10/18.
//

#ifndef PPLAY_MAIN_H
#define PPLAY_MAIN_H

#include "c2d.h"
#include "filer.h"

#define INPUT_DELAY 150
#define FONT_SIZE 25

#define COLOR_BG_0  Color(0xbdc3c7ff)
#define COLOR_BG_1  Color(0xecf0f1ff)

class Main {

public:
    Main();

    ~Main();

    void run();

private:
    c2d::Renderer *renderer;
    c2d::Font *font;
    c2d::Io *io;
    c2d::Input *input;
    c2d::Clock *timer;
    Filer *filer;
};

#endif //PPLAY_MAIN_H
