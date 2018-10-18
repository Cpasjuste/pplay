//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filer_sdmc.h"

using namespace c2d;
using namespace c2d::config;

void Main::initConfig() {

    Section section("HTTP_SERVERS");
    section.add({"SERVER0", "http://divers.klikissi.fr/telechargements/"});
    section.add({"SERVER1", ""});
    section.add({"SERVER2", ""});
    section.add({"SERVER3", ""});
    section.add({"SERVER4", ""});
    config->add(section);

    // load the configuration from file, overwriting default values
    if (!config->load()) {
        // file doesn't exist or is malformed, (re)create it
        config->save();
    }
}

Main::Main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setClearColor(COLOR_GRAY_LIGHT);

    // configure input
    getInput()->setRepeatEnable(true);
    getInput()->setRepeatDelay(INPUT_DELAY);

    // create a font
    font = new Font();
    font->load();

    // create a timer
    timer = new C2DClock();

    // init/load config file
    config = new Config("PPLAY", getIo()->getHomePath() + "/pplay.cfg");
    initConfig();

    // create a rect
    mainRect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    mainRect->setFillColor(Color::Transparent);

    // create filers
    FloatRect filerRect = {16, 16,
                           (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32};
    filerSdmc = new FilerSdmc(renderer->getIo(), ".", *font, FONT_SIZE, filerRect);
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

        unsigned int keys = getInput()->getKeys();

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
                filer->setVisibility(Visibility::Hidden);
                if (filer == filerHttp) {
                    filer = filerSdmc;
                } else {
                    filer = filerHttp;
                }
                filer->setVisibility(Visibility::Visible);
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

c2d::Renderer *Main::getRenderer() {
    return renderer;
}

c2d::Io *Main::getIo() {
    return renderer->getIo();
}

c2d::Font *Main::getFont() {
    return font;
}

c2d::Input *Main::getInput() {
    return renderer->getInput();
}

c2d::config::Config *Main::getConfig() {
    return config;
}

Main::~Main() {

    delete (config);
    delete (timer);
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
