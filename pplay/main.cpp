//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"

using namespace c2d;

Main::Main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setClearColor(COLOR_GRAY_LIGHT);

    // create a font
    font = new Font();
    font->load();
    font->setOffset({0, -4});

    // create io
    io = new C2DIo();

    // create input
    input = new C2DInput();
    input->setJoystickMapping(0, C2D_DEFAULT_JOY_KEYS, 0);
    input->setKeyboardMapping(C2D_DEFAULT_KB_KEYS);

    // create a timer
    timer = new C2DClock();

    // create a rect
    mainRect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    mainRect->setFillColor(Color::Transparent);

    filer = new Filer(io, "./medias", *font, FONT_SIZE,
                      {16, 16,
                       (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32});
    mainRect->add(filer);

    // add all this crap
    renderer->add(mainRect);

    input->setRepeatEnable(true);
    input->setRepeatDelay(INPUT_DELAY);

    // ffmpeg player
    player = new Player(this);
    player->setVisibility(Visibility::Hidden);
    renderer->add(player);
}

void Main::run() {

    while (true) {

        unsigned int keys = input->update()[0].state;

        player->step(player->isMaximized() ? keys : 0);

        if (keys > 0) {

            if (((keys & c2d::Input::Key::KEY_START)
                 || (keys & c2d::Input::Key::KEY_COIN)
                 || (keys & EV_QUIT))) {
                if (player->isPlaying()) {
                    player->stop();
                } else {
                    break;
                }
            }

            if (!player->isMaximized()) {
                Io::File file = filer->step(keys);
                if (keys & Input::Key::KEY_FIRE1 && file.type == Io::Type::File) {
                    if (player->load(file)) {
                        player->setScale(0.4f, 0.4f);
                        player->setPosition(renderer->getSize().x * 0.55f, renderer->getSize().y * 0.55f);
                    }
                }
            }
        }

        renderer->flip();
    }
}

c2d::C2DRenderer *Main::getRenderer() {
    return renderer;
}

c2d::C2DIo *Main::getIo() {
    return io;
}

c2d::C2DFont *Main::getFont() {
    return font;
}

c2d::C2DInput *Main::getInput() {
    return input;
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
