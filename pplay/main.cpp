//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filer_sdmc.h"

using namespace c2d;

Main::Main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setClearColor(COLOR_GRAY_LIGHT);

    // create a font
    font = new Font();
    font->load();

    // create io
    io = new C2DIo();

    // create input
    input = new C2DInput();
    input->setJoystickMapping(0, C2D_DEFAULT_JOY_KEYS, 0);
    input->setKeyboardMapping(C2D_DEFAULT_KB_KEYS);
    input->setRepeatEnable(true);
    input->setRepeatDelay(INPUT_DELAY);

    // create a timer
    timer = new C2DClock();

    // create a rect
    mainRect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    mainRect->setFillColor(Color::Transparent);

    // create filers
    FloatRect filerRect = {16, 16,
                           (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32};
    filerSdmc = new FilerSdmc(io, ".", *font, FONT_SIZE, filerRect);
    mainRect->add(filerSdmc);
    filerHttp = new FilerHttp(*font, FONT_SIZE, filerRect);
    // "http://divers.klikissi.fr/telechargements/"
    filerHttp->setVisibility(Visibility::Hidden);
    mainRect->add(filerHttp);
    filer = filerSdmc;

    // add all this crap
    renderer->add(mainRect);

    // ffmpeg player
    player = new Player(this);
    renderer->add(player);
}

void Main::setPlayerSize(bool fs) {
    if (fs) {
        mainRect->setVisibility(Visibility::Visible);
        player->getTweenPosition()->play(TweenDirection::Forward);
        player->getTweenScale()->play(TweenDirection::Forward);
    } else {
        //mainRect->setVisibility(Visibility::Hidden);
        player->getTweenPosition()->play(TweenDirection::Backward);
        player->getTweenScale()->play(TweenDirection::Backward);
    }
    player->setFullscreen(fs);
}

void Main::run() {

    while (true) {

        unsigned int keys = input->update()[0].state;

        player->step(player->isFullscreen() ? keys : 0);

        if (keys > 0) {
            if (((keys & c2d::Input::Key::KEY_START)
                 || (keys & c2d::Input::Key::KEY_COIN)
                 || (keys & EV_QUIT))) {
                if (player->isPlaying()) {
                    player->stop();
                    //mainRect->setVisibility(Visibility::Visible);
                } else {
                    break;
                }
            }

            if (keys & c2d::Input::KEY_FIRE5 || keys & c2d::Input::KEY_FIRE6) {
                if (filer == filerHttp) {
                    filer = filerSdmc;
                } else {
                    filer = filerHttp;
                }
            }

            if (keys & c2d::Input::KEY_FIRE3) {
                if (player->isPlaying()) {
                    setPlayerSize(!player->isFullscreen());
                }
            }

            if (!player->isFullscreen()) {
                if (filer->step(keys)) {
                    Io::File file = filer->getSelection();
                    if (filer == filerHttp) {
                        file.path = filerHttp->getPath() + file.path;
                    }
                    if (player->load(file)) {
                        setPlayerSize(true);
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

#ifdef __SWITCH__
    socketInitializeDefault();
#endif

    Main *main = new Main();
    main->run();
    delete (main);

#ifdef __SWITCH__
    socketExit();
#endif

    return 0;
}
