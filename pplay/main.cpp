//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filer.h"
#include "data/skin/default.h"

#define INPUT_DELAY 100
#define FONT_SIZE 25

using namespace c2d;

Renderer *renderer;
Font *font;
Io *io;
Input *input;
Clock *timer;
Filer *filer;

int C2D_JOY_KEYS[]{
        // UP, DOWN, LEFT, RIGHT, COINS (SELECT), START, ..., // QUIT
        KEY_JOY_UP_DEFAULT, KEY_JOY_DOWN_DEFAULT, KEY_JOY_LEFT_DEFAULT, KEY_JOY_RIGHT_DEFAULT,
        KEY_JOY_COIN1_DEFAULT, KEY_JOY_START1_DEFAULT,
        KEY_JOY_FIRE1_DEFAULT, KEY_JOY_FIRE2_DEFAULT, KEY_JOY_FIRE3_DEFAULT,
        KEY_JOY_FIRE4_DEFAULT, KEY_JOY_FIRE5_DEFAULT, KEY_JOY_FIRE6_DEFAULT,
        0
};

int C2D_KB_KEYS[]{
        // UP, DOWN, LEFT, RIGHT, COINS (SELECT), START, ..., // QUIT
        KEY_KB_UP_DEFAULT, KEY_KB_DOWN_DEFAULT, KEY_KB_LEFT_DEFAULT, KEY_KB_RIGHT_DEFAULT,
        KEY_KB_COIN1_DEFAULT, KEY_KB_START1_DEFAULT,
        KEY_KB_FIRE1_DEFAULT, KEY_KB_FIRE2_DEFAULT, KEY_KB_FIRE3_DEFAULT,
        KEY_KB_FIRE4_DEFAULT, KEY_KB_FIRE5_DEFAULT, KEY_KB_FIRE6_DEFAULT,
        0
        // 13, 15, 12, 14, 10, 11, 0, 1, 2, 3, 7, 6, 0 // QUIT
};

int main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setFillColor(COLOR_BG_0);

    // create a font
    font = new Font();
    font->loadFromMemory(pfba_font, pfba_font_length);
    font->setYOffset(-5);

    // create io
    io = new C2DIo();

    // create input
    input = new C2DInput();
    input->setJoystickMapping(0, C2D_JOY_KEYS, 0);
    input->setKeyboardMapping(C2D_KB_KEYS);

    // create a timer
    timer = new C2DClock();

    // create a rect
    Rectangle *rect = new C2DRectangle(Vector2f(renderer->getSize().x - 4, renderer->getSize().y - 4));
    rect->setPosition(2, 2);
    rect->setFillColor(Color::Transparent);
    rect->setOutlineColor(Color::Orange);
    rect->setOutlineThickness(2);

    filer = new Filer(io, "/", *font, FONT_SIZE,
                      FloatRect(rect->getPosition().x + 16, rect->getPosition().y + 16,
                                (rect->getSize().x / 2) - 16, rect->getSize().y - 32));
    rect->add(filer);

    // add all this crap to the renderer
    renderer->add(rect);

    while (true) {

        // handle input
        unsigned int key = input->update()[0].state;
        if (key > 0) {

            if (key & EV_QUIT) { // SDL2 quit event
                break;
            }

            if (key & Input::Key::KEY_FIRE2) {
                // TODO: ask confirmation to exit
                //break;
            }

            if (key & Input::Key::KEY_UP) {
                filer->up();
            } else if (key & Input::Key::KEY_DOWN) {
                filer->down();
            } else if (key & Input::Key::KEY_RIGHT) {
                filer->right();
            } else if (key & Input::Key::KEY_LEFT) {
                filer->left();
            } else if (key & Input::Key::KEY_FIRE1) {
                filer->enter();
            } else if (key & Input::Key::KEY_FIRE2) {
                filer->exit();
            }

            if (timer->getElapsedTime().asSeconds() > 10) {
                renderer->delay(INPUT_DELAY / 8);
            } else if (timer->getElapsedTime().asSeconds() > 6) {
                renderer->delay(INPUT_DELAY / 5);
            } else if (timer->getElapsedTime().asSeconds() > 2) {
                renderer->delay(INPUT_DELAY / 2);
            } else {
                renderer->delay(INPUT_DELAY);
            }
        } else {
            timer->restart();
        }

        renderer->flip();
    }

    delete (timer);
    delete (input);
    delete (io);
    delete (font);
    // will delete widgets recursively
    delete (renderer);

    return 0;
}
