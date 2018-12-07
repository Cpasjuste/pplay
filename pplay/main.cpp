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
    config = new PPLAYConfig(this);

    // font
    font = new C2DFont();
    font->loadFromFile(getIo()->getDataReadPath() + "skin/font.ttf");
    font->setFilter(Texture::Filter::Point);
    font->setOffset({0, -4});

    // create a rect
    mainRect = new C2DRectangle(getSize());
    mainRect->setFillColor(Color::Transparent);

    // media info
    getIo()->create(getIo()->getDataWritePath() + "cache");
    mediaInfoThread = new MediaThread(this, getIo()->getDataWritePath() + "cache/");

    // create filers
    FloatRect filerRect = {0, 150, (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32 - 128};
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
    messageBox->setFillColor(COLOR_BG);
    messageBox->setAlpha(200);
    messageBox->setOutlineColor(COLOR_RED);
    messageBox->setOutlineThickness(2);
    messageBox->getTitleText()->setOutlineThickness(0);
    messageBox->getMessageText()->setOutlineThickness(0);
    add(messageBox);
}

void Main::onInput(c2d::Input::Player *players) {

    unsigned int keys = players[0].state;

    if (messageBox->isVisible()) {
        // don't handle input if message box is visible
        return;
    }

    if (keys > 0) {
        if (keys & EV_QUIT) {
            if (player->isFullscreen()) {
                setPlayerFullscreen(false);
                filer->setVisibility(Visibility::Visible, true);
            } else {
                quit();
            }
        } else if (keys & Input::Touch) {
            if (menu->getMenuButton()->getGlobalBounds().contains(players[0].touch)) {
                menu->setVisibility(Visibility::Visible, true);
                return;
            } else if (player->getGlobalBounds().contains(players[0].touch)) {
                if (player->isPlaying() && !player->isFullscreen()) {
                    setPlayerFullscreen(true);
                    return;
                }
            }
        }
    }

    Renderer::onInput(players);
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
            messageBox->show("Oups", "Could not open url (see config file?)", "OK");
            show(MenuType::Home);
        }
    }
}

void Main::setPlayerFullscreen(bool fs) {

    if (fs) {
        filer->setVisibility(Visibility::Hidden, true);
        player->getTweenPosition()->play(TweenDirection::Forward);
        player->getTweenScale()->play(TweenDirection::Forward);
    } else {
        player->getTweenPosition()->play(TweenDirection::Backward);
        player->getTweenScale()->play(TweenDirection::Backward);
        filer->setVisibility(Visibility::Visible, true);
    }
    player->setFullscreen(fs);
}

bool Main::isRunning() {
    return running;
}

void Main::quit() {

    // TODO: save network path
    if (filerSdmc->isVisible()) {
        config->getOption("LAST_PATH")->setString(filer->getPath());
        config->save();
    }

    running = false;
}

MediaThread *Main::getMediaThread() {
    return mediaInfoThread;
}

Player *Main::getPlayer() {
    return player;
}

Filer *Main::getFiler() {
    return filer;
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

c2d::MessageBox *Main::getMessageBox() {
    return messageBox;
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
