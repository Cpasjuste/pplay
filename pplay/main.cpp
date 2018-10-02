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
    renderer->setFillColor(COLOR_GRAY);

    // create a font
    font = new Font();
    font->setFiltering(C2D_TEXTURE_FILTER_POINT);
    font->loadFromFile("./skin/OpenSans-CondBold.ttf");
    //font->loadFromFile("./skin/OpenSans-Regular.ttf");
    font->setYOffset(-3);

    // create io
    io = new C2DIo();

    // create input
    input = new C2DInput();
    input->setJoystickMapping(0, C2D_DEFAULT_JOY_KEYS, 0);
    input->setKeyboardMapping(C2D_DEFAULT_KB_KEYS);

    // create a timer
    timer = new C2DClock();

    // create a rect
    Rectangle *rect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    rect->setPosition(4, 4);
    rect->setFillColor(Color::Transparent);
    rect->setOutlineColor(COLOR_BLUE);
    rect->setOutlineThickness(4);

    filer = new Filer(io, "/", *font, FONT_SIZE,
                      {rect->getPosition().x + 16, rect->getPosition().y + 16,
                       (rect->getSize().x / 2) - 16, rect->getSize().y - 40});
    rect->add(filer);

    // add all this crap to the renderer
    renderer->add(rect);

    input->setRepeatEnable(true);
    input->setRepeatDelay(INPUT_DELAY);
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

            int fire_press = (keys & Input::Key::KEY_FIRE1);
            Io::File file = filer->processInput(keys);
            if (fire_press && file.type == Io::Type::File) {
                printf("file: %s\n", file.path.c_str());
            }
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
