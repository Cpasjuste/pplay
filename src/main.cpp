//
// Created by cpasjuste on 02/10/18.
//
#include "main.h"
#include "io.h"
#include "filer.h"
#include "menu_main.h"
#include "menu_video.h"
#include "scrapper.h"

#ifdef __SMB_SUPPORT__
#include "filers/filer_smb.h"
#endif

#ifdef __SWITCH__

static AppletHookCookie applet_hook_cookie;

static void on_applet_hook(AppletHookType hook, void *arg) {

    Main *main = (Main *) arg;

    switch (hook) {
        case AppletHookType_OnExitRequest:
            main->quit();
            break;
        case AppletHookType_OnFocusState:
            if (appletGetFocusState() == AppletFocusState_InFocus) {
                if (main->getPlayer()->getMpv()->isPaused()) {
                    main->getPlayer()->resume();
                }
            } else {
                if (!main->getPlayer()->getMpv()->isPaused()) {
                    main->getPlayer()->pause();
                }
            }
            break;
        case AppletHookType_OnPerformanceMode:
            break;

        default:
            break;
    }
}

#endif

using namespace c2d;
using namespace c2d::config;
using namespace pplay;

Main::Main(const c2d::Vector2f &size) : C2DRenderer(size) {
    // custom io
    pplayIo = new pplay::Io();

    // configure input
    getInput()->setRepeatDelay(INPUT_DELAY);

    // create a timer
    timer = new C2DClock();

    // init/load config file
    config = new PPLAYConfig(this);

    // scaling
    scaling = size.x / 1280.0f;

    // font
    font = new C2DFont();
    font->loadFromFile(getIo()->getRomFsPath() + "skin/font.ttf");
    font->setFilter(Texture::Filter::Point);
    font->setOffset({0, -4});

    statusBox = new StatusBox(this, {0, getSize().y - 16});
    statusBox->setOrigin(Origin::BottomLeft);
    statusBox->setLayer(10);
    add(statusBox);

    // media information cache
    getIo()->create(getIo()->getDataPath() + "cache");

    // create filer
    FloatRect filerRect = {0, 0, getSize().x, getSize().y};
    filer = new Filer(this, "/", filerRect);
    filer->setLayer(1);
    add(filer);
    filer->getDir(config->getOption(OPT_LAST_PATH)->getString());

    // status bar
    statusBar = new StatusBar(this);
    statusBar->setLayer(10);
    add(statusBar);

    // ffmpeg player
    player = new Player(this);
    player->setLayer(2);
    add(player);

    // main menu
    std::vector<MenuItem> items;
    items.emplace_back("Home", "home.png", MenuItem::Position::Top);
    items.emplace_back("USB", "usb.png", MenuItem::Position::Top);
    items.emplace_back("Network", "network.png", MenuItem::Position::Top);
    items.emplace_back("Options", "options.png", MenuItem::Position::Top);
    items.emplace_back("Exit", "exit.png", MenuItem::Position::Bottom);
    menu_main = new MenuMain(this, {-250 * scaling, 0, 250 * scaling, getSize().y}, items);
    menu_main->setVisibility(Visibility::Hidden, false);
    menu_main->setLayer(3);
    add(menu_main);

    // video menu
    items.clear();
    items.emplace_back("Video", "video.png", MenuItem::Position::Top);
    items.emplace_back("Audio", "audio.png", MenuItem::Position::Top);
    items.emplace_back("Subtitles", "subtitles.png", MenuItem::Position::Top);
    items.emplace_back("Stop", "exit.png", MenuItem::Position::Bottom);
    menu_video = new MenuVideo(this, {getSize().x, 0, 250 * scaling, getSize().y}, items);
    menu_video->setVisibility(Visibility::Hidden, false);
    menu_video->setLayer(3);
    add(menu_video);

    // a messagebox...
    float w = getSize().x / 3;
    float h = getSize().y / 3;
    messageBox = new MessageBox({getSize().x / 2, getSize().y / 2, w, h},
                                getInput(), getFont(), getFontSize(Main::FontSize::Medium));
    messageBox->setOrigin(Origin::Center);
    messageBox->setFillColor(COLOR_BG);
    messageBox->setAlpha(240);
    messageBox->setOutlineColor(COLOR_RED);
    messageBox->setOutlineThickness(2);
    messageBox->getTitleText()->setOutlineThickness(0);
    messageBox->getMessageText()->setOutlineThickness(0);
    add(messageBox);

    scrapper = new Scrapper(this);
    //scrapper->scrap("/home/cpasjuste/dev/multi/videos/");
    //scrapper->scrap("http://192.168.0.2/files/Videos");
}

Main::~Main() {
    delete (scrapper);
    delete (config);
    delete (timer);
    delete (font);
    delete (pplayIo);
}

bool Main::onInput(c2d::Input::Player *players) {

    if (messageBox->isVisible()) {
        // don't handle input if message box is visible
        return false;
    }

    unsigned int keys = players[0].keys;

    if (keys & EV_QUIT) {
        if (player->isFullscreen()) {
            player->setFullscreen(false);
            filer->setVisibility(Visibility::Visible, true);
        } else {
            quit();
        }
    } else if (keys & Input::Touch) {
#if 0
        if (player->getGlobalBounds().contains(players[0].touch)) {
            if (!player->getMpv()->isStopped() && !player->isFullscreen()) {
                player->setFullscreen(true);
                return true;
            }
        }
#endif
    }

    return Renderer::onInput(players);
}

void Main::onDraw(c2d::Transform &transform, bool draw) {

    unsigned int keys = getInput()->getKeys(0);

    if (keys != Input::Key::Delay) {
        if (keys && timer->getElapsedTime().asMilliseconds() > INPUT_DELAY) {
            getInput()->setRepeatDelay(INPUT_DELAY / 5);
        } else if (!keys) {
            getInput()->setRepeatDelay(INPUT_DELAY);
            timer->restart();
        }
    }

    C2DObject::onDraw(transform);
}

void Main::show(MenuType type) {

    if (player->getMpv()->isStopped() && player->isFullscreen()) {
        player->setFullscreen(false);
    }

    filer->setVisibility(Visibility::Visible, true);
    if (type == MenuType::Home) {
#ifdef __SWITCH__
        usbHsFsExit();
#endif
        std::string path = config->getOption(OPT_HOME_PATH)->getString();
        if (!filer->getDir(path)) {
            if (filer->getDir("/")) {
                filer->clearHistory();
            }
        }
#ifdef __SWITCH__
    } else if (type == MenuType::Usb) {
        usbInit();
        filer->getDir(config->getOption(OPT_UMS_DEVICE)->getString());
#endif
    } else {
#ifdef __SWITCH__
        usbHsFsExit();
#endif
        std::string path = config->getOption(OPT_NETWORK)->getString();
        if (!filer->getDir(path)) {
            messageBox->show("OOPS", filer->getError(), "OK");
            show(MenuType::Home);
        } else {
            filer->clearHistory();
        }
    }
}

bool Main::isExiting() {
    return exit;
}

bool Main::isRunning() {
    return running;
}

void Main::setRunningStop() {
    printf("Main::setRunningStop()\n");
    running = false;
}

void Main::quit() {

    // TODO: save network path
    config->getOption(OPT_LAST_PATH)->setString(filer->getPath());
    config->save();
    exit = true;
    if (player->getMpv()->isStopped()) {
        running = false;
    } else {
        player->stop();
    }
}

Player *Main::getPlayer() {
    return player;
}

Filer *Main::getFiler() {
    return filer;
}

MenuMain *Main::getMenuMain() {
    return menu_main;
}

MenuVideo *Main::getMenuVideo() {
    return menu_video;
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

StatusBox *Main::getStatus() {
    return statusBox;
}

float Main::getScaling() {
    return scaling;
}

unsigned int Main::getFontSize(FontSize fontSize) {
    return (unsigned int) ((float) fontSize * scaling);
}

StatusBar *Main::getStatusBar() {
    return statusBar;
}

pplay::Scrapper *Main::getScrapper() {
    return scrapper;
}

c2d::Io *Main::getIo() {
    return (c2d::Io *) pplayIo;
}

int main() {

    Vector2f size = {1280, 720};

#ifdef __SWITCH__
#ifndef __NET_DEBUG__
    socketInitializeDefault();
#endif
    appletMainLoop();
    if (appletGetOperationMode() == AppletOperationMode_Console) {
        size = {1920, 1080};
    }
#endif

    Main *main = new Main(size);

#ifdef __SWITCH__
    appletLockExit();
    appletHook(&applet_hook_cookie, on_applet_hook, main);
    appletSetFocusHandlingMode(AppletFocusHandlingMode_NoSuspend);
#endif

    while (main->isRunning()) {
        main->flip();
    }

    delete (main);

#ifdef __SWITCH__
    usbHsFsExit();
    appletUnhook(&applet_hook_cookie);
    appletUnlockExit();
#ifndef __NET_DEBUG__
    socketExit();
#endif
#endif

    return 0;
}
