//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
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
#ifndef NO_KEYBOARD
    input->setKeyboardMapping(C2D_DEFAULT_KB_KEYS);
#endif

    // create a timer
    timer = new C2DClock();

    // create a rect
    mainRect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    mainRect->setPosition(4, 4);
    mainRect->setFillColor(Color::Transparent);
    mainRect->setOutlineColor(COLOR_BLUE);
    mainRect->setOutlineThickness(4);

    filer = new Filer(io, "./medias", *font, FONT_SIZE,
                      {mainRect->getPosition().x + 16, mainRect->getPosition().y + 16,
                       (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 40});
    mainRect->add(filer);

    // add all this crap
    renderer->add(mainRect);

    input->setRepeatEnable(true);
    input->setRepeatDelay(INPUT_DELAY);

    // ffmpeg player
    player = new Player(this);
    player->setVisibility(C2DObject::Hidden);
    renderer->add(player);
}

void Main::run() {

    while (true) {

        // handle input
        unsigned int keys = input->update()[0].state;
        if (keys > 0) {

            if (keys & EV_QUIT) { // SDL2 quit event
                break;
            }

            if (keys & Input::Key::KEY_START || keys & Input::Key::KEY_COIN) {
                // TODO: ask confirmation to exit
                break;
            }

            int fire_press = (keys & Input::Key::KEY_FIRE1);
            Io::File file = filer->processInput(keys);
            if (fire_press && file.type == Io::Type::File) {
                printf("file: %s\n", file.path.c_str());
                // TODO: if supported..
                if (player->load(file)) {
                    renderer->setFillColor(Color::Black);
                    mainRect->setVisibility(C2DObject::Hidden);
                    player->setVisibility(C2DObject::Visible);
                    player->run();
                    player->setVisibility(C2DObject::Hidden);
                    mainRect->setVisibility(C2DObject::Visible);
                    renderer->setFillColor(COLOR_GRAY);
                }
            }
        }

        renderer->flip();
    }
}

c2d::Renderer *Main::getRenderer() {
    return renderer;
}

c2d::Io *Main::getIo() {
    return io;
}

c2d::Font *Main::getFont() {
    return font;
}

c2d::Input *Main::getInput() {
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
