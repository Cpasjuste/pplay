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
    font->setFilter(Texture::Filter::Point);
    font->setOffset({0, -4});

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

    // media info
    getIo()->create(getIo()->getHomePath() + "cache");
    mediaInfoThread = new MediaThread(this, getIo()->getHomePath() + "cache/");

    // create filers
    FloatRect filerRect = {0, 128, (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32 - 128};
    filerSdmc = new FilerSdmc(this, "/", filerRect);
    mainRect->add(filerSdmc);
    filerHttp = new FilerHttp(this, filerRect);
    filerHttp->setVisibility(Visibility::Hidden);
    mainRect->add(filerHttp);
    filer = filerSdmc;
    filer->getDir(config->getOption("LAST_PATH")->getString());

    // add all this crap
    add(mainRect);

    // ffmpeg player
    player = new Player(this);
    add(player);

    // menu
    menu = new OptionMenu(this, {0, 0, 250, mainRect->getSize().y});
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

    if (keys > 0) {
        if (keys & EV_QUIT) {
            if (player->isPlaying()) {
                player->stop();
            } else {
                running = false;
            }
            return;
        }
        if (keys & Input::Touch) {
            if (menu->getMenuButton()->getGlobalBounds().contains(players[0].touch)) {
                menu->setVisibility(Visibility::Visible, true);
                return;
            } else if (player->getGlobalBounds().contains(players[0].touch)) {
                if (player->isPlaying() && !player->isFullscreen()) {
                    setPlayerFullscreen(true);
                }
                return;
            }
        }
    }

    C2DObject::onInput(players);
}

void Main::show(MenuType type) {

    if (player->isPlaying() && player->isFullscreen()) {
        setPlayerFullscreen(false);
    }

    filerSdmc->setVisibility(type == MenuType::Home ? Visibility::Visible : Visibility::Hidden);
    filerHttp->setVisibility(type == MenuType::Home ? Visibility::Hidden : Visibility::Visible);
    filer = type == MenuType::Home ? filerSdmc : filerHttp;
    if (type == MenuType::Home) {
        if (!filer->getDir(config->getOption("HOME_PATH")->getString())) {
            filer->getDir("/");
        }
    } else {
        if (!filer->getDir(config->getOption("NETWORK")->getString())) {
            messageBox->show("OUPS", "Could not browse URL");
            show(MenuType::Home);
        }
    }
}

void Main::setPlayerFullscreen(bool fs) {
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

    // TODO: save network path
    if (filerSdmc->isVisible()) {
        config->getOption("LAST_PATH")->setString(filer->getPath());
    }
    config->save();

    running = false;
}

MediaThread *Main::getMediaThread() {
    return mediaInfoThread;
}

Player *Main::getPlayer() {
    return player;
}

OptionMenu *Main::getMenu() {
    return menu;
}

c2d::RectangleShape *Main::getMainRect() {
    return mainRect;
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
