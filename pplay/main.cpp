//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filer.h"
#include "data/skin/default.h"

using namespace c2d;

Main::Main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setFillColor(COLOR_BG_0);

    // create a font
    font = new Font();
    font->loadFromMemory(font_default, font_default_length);
    font->setYOffset(-5);

    // create io
    io = new C2DIo();

    // create input
    input = new C2DInput();
    input->setJoystickMapping(0, C2D_DEFAULT_JOY_KEYS, 0);
    input->setKeyboardMapping(C2D_DEFAULT_KB_KEYS);

    // create a timer
    timer = new C2DClock();

    // create a rect
    Rectangle *rect = new C2DRectangle({renderer->getSize().x - 4, renderer->getSize().y - 4});
    rect->setPosition(2, 2);
    rect->setFillColor(Color::Transparent);
    rect->setOutlineColor(Color::Orange);
    rect->setOutlineThickness(2);

    filer = new Filer(io, "/", *font, FONT_SIZE,
                      {rect->getPosition().x + 16, rect->getPosition().y + 16,
                       (rect->getSize().x / 2) - 16, rect->getSize().y - 32});
    rect->add(filer);

    // add all this crap to the renderer
    renderer->add(rect);

    input->setRepeatEnable(true);
    input->setRepeatDelay(100);
}

void Main::run() {

    while (true) {

        // handle input
        unsigned int keys = input->update()[0].state;
        if (keys > 0) {

            if (keys & EV_QUIT) { // SDL2 quit event
                break;
            }

            if (keys & Input::Key::KEY_FIRE2) {
                // TODO: ask confirmation to exit
                //break;
            }

            filer->processInput(keys);
        }

        renderer->flip();
    }
}

Main::~Main() {

    delete (timer);
    delete (input);
    delete (io);
    delete (font);
    // will delete widgets recursively
    delete (renderer);
}

int main() {

    Main *main = new Main();
    main->run();
    delete (main);

    return 0;
}
