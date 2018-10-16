//
// Created by cpasjuste on 02/10/18.
//

#ifndef PPLAY_MAIN_H
#define PPLAY_MAIN_H

#include "cross2d/c2d.h"
#include "filer.h"
#include "filer_http.h"
#include "player.h"
#include "player_osd.h"

#define INPUT_DELAY 250
#define FONT_SIZE 22

#define COLOR_CLOUD         Color(0xecf0f1ff)
#define COLOR_GRAY          Color(0x636e72ff)
#define COLOR_GRAY_LIGHT    Color(0xb2bec3ff)
#define COLOR_GRAY_DARK     Color(0x2d3436ff)
#define COLOR_BLUE          Color(0x0984e3ff)
#define COLOR_BLUE_LIGHT    Color(0x74b9ffff)
#define COLOR_PURPLE        Color(0x6c5ce7ff)
#define COLOR_PURPLE_LIGHT  Color(0xa29bfeff)
#define COLOR_GREEN         Color(0x1abc9cff)
#define COLOR_GREEN_LIGHT   Color(0x55efc4ff)
#define COLOR_ORANGE        Color(0xd35400ff)
#define COLOR_ORANGE_LIGHT  Color(0xe67e22ff)

class Main {

public:
    Main();

    ~Main();

    void run();

    c2d::C2DRenderer *getRenderer();

    c2d::C2DIo *getIo();

    c2d::C2DFont *getFont();

    c2d::C2DInput *getInput();

private:
    c2d::C2DRenderer *renderer = nullptr;
    c2d::C2DRectangle *mainRect = nullptr;
    c2d::C2DFont *font = nullptr;
    c2d::C2DIo *io = nullptr;
    c2d::C2DInput *input = nullptr;
    c2d::C2DClock *timer = nullptr;
    HttpFiler *filer = nullptr;
    Player *player = nullptr;
};

#endif //PPLAY_MAIN_H
