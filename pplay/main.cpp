//
// Created by cpasjuste on 02/10/18.
//

#ifdef __SWITCH__
extern "C" {
//#include <pthread.h>
}
#endif

#include "main.h"
#include "filer_sdmc.h"
#include "filer_paths.h"

using namespace c2d;
using namespace c2d::config;

Main::Main(const c2d::Vector2f &size) : C2DRenderer(size) {

    setClearColor(Color::Black);

    // configure input
    getInput()->setRepeatEnable(true);
    getInput()->setRepeatDelay(INPUT_DELAY);

    // create a timer
    timer = new C2DClock();

    // init/load config file
    config = new PPLAYConfig("PPLAY", getIo()->getHomePath() + "pplay.cfg");

    // font
    font = new C2DFont();
    font->loadFromFile(getIo()->getDataPath() + "skin/LiberationSans-Regular.ttf");
    font->setOffset({0, -5});

    // create a rect
    mainRect = new C2DRectangle(getSize());
    mainRect->setFillColor(Color::Transparent);

    if (!getIo()->exist(config->getOption("HOME_PATH")->getString())) {
        config->getOption("HOME_PATH")->setString(getIo()->getHomePath());
        config->save();
    }

    if (!getIo()->exist(config->getOption("LAST_PATH")->getString())) {
        config->getOption("LAST_PATH")->setString(getIo()->getHomePath());
        config->save();
    }

    // create filers
    FloatRect filerRect = {16, 16,
                           (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32};
    filerSdmc = new FilerSdmc(getIo(), "/", getFont(), FONT_SIZE, filerRect);
    mainRect->add(filerSdmc);
    filerHttp = new FilerHttp(getFont(), FONT_SIZE, filerRect);
    filerHttp->setVisibility(Visibility::Hidden);
    mainRect->add(filerHttp);
    filerRect.top += FONT_SIZE + 16;
    filerRect.height = FONT_SIZE * 5 + 16 * 5;
    filerPaths = new FilerPaths(config, getFont(), FONT_SIZE, filerRect);
    filerPaths->setVisibility(Visibility::Hidden);
    mainRect->add(filerPaths);
    filer = filerSdmc;
    filer->getDir(config->getOption("LAST_PATH")->getString());

    // add all this crap
    add(mainRect);

    // ffmpeg player
    player = new Player(this);
    add(player);

    // media info
    getIo()->create(getIo()->getHomePath() + "cache");
    mediaInfoThread = new MediaThread(this, getIo()->getHomePath() + "cache/");
    mediaInfoThread->cacheDir(filer->getPath());

    // create menu
    menu = new OptionMenu(this, {0, 0, mainRect->getSize().x / 4, mainRect->getSize().y});
    menu->setVisibility(Visibility::Hidden);
    menu->setLayer(100);
    add(menu);

    // a messagebox...
    float w = getSize().x / 2;
    float h = getSize().y / 2;
    messageBox = new MessageBox({w, h, w, h}, getInput(), getFont(), FONT_SIZE);
    messageBox->setOrigin(Origin::Center);
    messageBox->setFillColor(COLOR_GRAY_DARK);
    messageBox->setOutlineColor(COLOR_ORANGE);
    messageBox->setOutlineThickness(2);
    messageBox->setLayer(101);
    add(messageBox);
}

void Main::onInput(c2d::Input::Player *players) {

    unsigned int keys = players[0].state;

    if (menu->isVisible()) {
        C2DObject::onInput(players);
        return;
    }

    if (keys > 0) {

        if (keys & EV_QUIT) {
            if (player->isPlaying()) {
                player->stop();
            } else {
                running = false;
            }
        }

        if (keys & c2d::Input::KEY_START
            || keys & c2d::Input::KEY_COIN) {
            menu->setVisibility(Visibility::Visible, true);
        }

        if (keys & c2d::Input::KEY_FIRE3) {
            if (player->isPlaying()) {
                setPlayerSize(!player->isFullscreen());
            }
        }

        if (!player->isFullscreen()) {

#if 0
            if (keys & c2d::Input::KEY_FIRE5 || keys & c2d::Input::KEY_FIRE6) {
                // handle local/http file browser selection
                if (filerPaths->isVisible()) {
                    filerPaths->setVisibility(Visibility::Hidden, true);
                } else {
                    filerPaths->setVisibility(Visibility::Visible, true);
                };
            }
#endif
            if (filerPaths->isVisible()) {
                // handle local/http file browser selection
                if (filerPaths->step(keys)) {
                    Io::File *file = filerPaths->getSelection();
                    if (Utility::startWith(file->name, "http:")) {
                        filerPaths->setVisibility(Visibility::Hidden, true);
                        filerSdmc->setVisibility(Visibility::Hidden);
                        filerHttp->setVisibility(Visibility::Visible);
                        filer = filerHttp;
                        filer->getDir(file->name);
                    } else if (Utility::startWith(file->name, "sdmc:")) {
                        filerPaths->setVisibility(Visibility::Hidden, true);
                        filerHttp->setVisibility(Visibility::Hidden);
                        filerSdmc->setVisibility(Visibility::Visible);
                        filer = filerSdmc;
                    } else {
                        messageBox->show("Oups", "This doesn't look like a valid link...\n"
                                                 "Maybe a bad config file ?", "OK");
                    }
                }
            } else {
                if (filer->step(keys)) {
                    if (player->load(filer->getSelection())) {
                        setPlayerSize(true);
                    }
                } else if (keys & c2d::Input::KEY_FIRE1) {
                    // cache media info on enter dir
                    printf("mediaInfo->cacheDir(%s)\n", filer->getPath().c_str());
                    mediaInfoThread->cacheDir(filer->getPath());
                }

                // load media info
                if (keys & c2d::Input::KEY_UP || keys & c2d::Input::KEY_DOWN
                    || keys & c2d::Input::KEY_LEFT || keys & c2d::Input::KEY_RIGHT) {
                    Io::File *file = filer->getSelection();
                    if (file && file->type == Io::Type::File) {
                        Media *info = mediaInfoThread->getMediaInfo(file->path);
                        if (info) {
                            info->debut_print();
                            delete (info);
                        }
                    }
                }
            }
        }
    } else {
        C2DObject::onInput(players);
    }
}

void Main::showHome() {

    filerSdmc->setVisibility(Visibility::Visible);
    filerHttp->setVisibility(Visibility::Hidden);
    filer = filerSdmc;
    if (!filer->getDir(config->getOption("HOME_PATH")->getString())) {
        filer->getDir("/");
    }
}

void Main::setPlayerSize(bool fs) {
    if (fs) {
        mainRect->setVisibility(Visibility::Visible);
        player->getTweenPosition()->play(TweenDirection::Forward);
        player->getTweenScale()->play(TweenDirection::Forward);
    } else {
        player->getTweenPosition()->play(TweenDirection::Backward);
        player->getTweenScale()->play(TweenDirection::Backward);
    }
    player->setFullscreen(fs);
}

bool Main::isRunning() {
    return running;
}

void Main::quit() {

    if (player->isPlaying()) {
        player->stop();
    }

    config->getOption("LAST_PATH")->setString(filer->getPath());
    config->save();

    running = false;
}

PPLAYConfig *Main::getConfig() {
    return config;
}

c2d::Font *Main::getFont() {
    return font;
}

Main::~Main() {

    delete (mediaInfoThread);
    delete (config);
    delete (timer);
    delete (font);
}

int main() {

#ifdef __SWITCH__
    socketInitializeDefault();
    //pthread_init();
#endif

    Main *main = new Main({C2D_SCREEN_WIDTH, C2D_SCREEN_HEIGHT});

    while (main->isRunning()) {
        main->flip();
    }

    delete (main);

#ifdef __SWITCH__
    //pthread_terminate();
    socketExit();
#endif

    return 0;
}
