//
// Created by cpasjuste on 02/10/18.
//
#include "main.h"
#include "io.h"
#include "filer.h"
#include "menu_main.h"
#include "menu_video.h"
#include "scrapper.h"

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
        default:
            break;
    }
}

#elif __PS4__

#include <orbis/Sysmodule.h>

extern "C" int sceSystemServiceLoadExec(const char *path, const char *args[]);
#endif

using namespace c2d;
using namespace c2d::config;
using namespace pplay;

Main::Main(const c2d::Vector2f &size) : C2DRenderer(size) {

#ifndef NDEBUG
    //Renderer::setPrintStats(true);
#endif

    // custom io
    pplayIo = new pplay::Io();
    Main::setIo(pplayIo);

    // configure input
    Main::getInput()->setRepeatDelay(INPUT_DELAY);

    // create a timer
    timer = new C2DClock();

    // init/load config file
    config = new PPLAYConfig(this);

    // scaling
    scaling = {size.x / 1280.0f, size.y / 720.0f};

    // font
    font = new Font();
    font->loadFromFile(Main::getIo()->getRomFsPath() + "skin/font.ttf");
    font->setFilter(Texture::Filter::Point);
    font->setOffset({0, -4.0f});

    statusBox = new StatusBox(this, {0, Main::getSize().y - 16});
    statusBox->setOrigin(Origin::BottomLeft);
    statusBox->setLayer(10);
    Main::add(statusBox);

    // media information cache
    Main::getIo()->create(Main::getIo()->getDataPath() + "cache");

    // create filer
    FloatRect filerRect = {0, 0, Main::getSize().x, Main::getSize().y};
    filer = new Filer(this, "/", filerRect);
    filer->setLayer(1);
    Main::add(filer);
    filer->getDir(config->getOption(OPT_LAST_PATH)->getString());

    // status bar
    statusBar = new StatusBar(this);
    statusBar->setLayer(10);
    Main::add(statusBar);

    // ffmpeg player
    player = new Player(this);
    player->setLayer(2);
    Main::add(player);

    // main menu
    std::vector<MenuItem> items;
    items.emplace_back("Home", "home.png", MenuItem::Position::Top);
#ifdef __SWITCH__
    items.emplace_back("Usb", "usb.png", MenuItem::Position::Top);
#endif
    items.emplace_back("Network", "network.png", MenuItem::Position::Top);
    items.emplace_back("Options", "options.png", MenuItem::Position::Top);
    items.emplace_back("Exit", "exit.png", MenuItem::Position::Bottom);
    menu_main = new MenuMain(this, {-250 * scaling.x, 0, 250 * scaling.x, Main::getSize().y}, items);
    menu_main->setVisibility(Visibility::Hidden, false);
    menu_main->setLayer(3);
    Main::add(menu_main);

    // video menu
    items.clear();
    items.emplace_back("Video", "video.png", MenuItem::Position::Top);
    items.emplace_back("Audio", "audio.png", MenuItem::Position::Top);
    items.emplace_back("Subtitles", "subtitles.png", MenuItem::Position::Top);
    items.emplace_back("Stop", "exit.png", MenuItem::Position::Bottom);
    menu_video = new MenuVideo(this, {Main::getSize().x, 0, 250 * scaling.x, Main::getSize().y}, items);
    menu_video->setVisibility(Visibility::Hidden, false);
    menu_video->setLayer(3);
    Main::add(menu_video);

    // a messagebox...
    float w = Main::getSize().x / 3;
    float h = Main::getSize().y / 3;
    messageBox = new MessageBox({Main::getSize().x / 2, Main::getSize().y / 2, w, h},
                                Main::getInput(), Main::getFont(), (int) getFontSize(Main::FontSize::Medium));
    messageBox->setOrigin(Origin::Center);
    messageBox->setFillColor(COLOR_BG);
    messageBox->setAlpha(240);
    messageBox->setOutlineColor(COLOR_RED);
    messageBox->setOutlineThickness(2);
    messageBox->getTitleText()->setOutlineThickness(0);
    messageBox->getMessageText()->setOutlineThickness(0);
    messageBox->getButton(0)->setOutlineThickness(3);
    messageBox->getButton(1)->setOutlineThickness(3);
    Main::add(messageBox);

    scrapper = new Scrapper(this);
}

Main::~Main() {
    delete (scrapper);
    delete (config);
    delete (timer);
    delete (font);
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
    }

    return Renderer::onInput(players);
}

void Main::onUpdate() {
    unsigned int keys = getInput()->getKeys(0);
    if (keys != Input::Key::Delay) {
        bool changed = (oldKeys ^ keys) != 0;
        oldKeys = keys;
        if (!changed) {
            if (timer->getElapsedTime().asSeconds() > 5) {
                getInput()->setRepeatDelay(INPUT_DELAY / 20);
            } else if (timer->getElapsedTime().asSeconds() > 3) {
                getInput()->setRepeatDelay(INPUT_DELAY / 8);
            } else if (timer->getElapsedTime().asSeconds() > 1) {
                getInput()->setRepeatDelay(INPUT_DELAY / 4);
            }
        } else {
            getInput()->setRepeatDelay(INPUT_DELAY);
            timer->restart();
        }
    }

    C2DRenderer::onUpdate();
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

Vector2f Main::getScaling() {
    return scaling;
}

unsigned int Main::getFontSize(FontSize fontSize) {
    return (unsigned int) ((float) fontSize * scaling.y);
}

StatusBar *Main::getStatusBar() {
    return statusBar;
}

pplay::Scrapper *Main::getScrapper() {
    return scrapper;
}

int main() {

    Vector2f size = {C2D_SCREEN_WIDTH, C2D_SCREEN_HEIGHT};

#ifdef __SWITCH__
#ifdef NDEBUG
    socketInitializeDefault();
#endif
    appletMainLoop();
    if (appletGetOperationMode() == AppletOperationMode_Console) {
        size = {1920, 1080};
    }
#elif __PS4__
    sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NET);
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
#ifdef NDEBUG
    socketExit();
#endif
#elif __PS4__
    sceSystemServiceLoadExec((char *) "exit", nullptr);
    while (true) {}
#endif

    return 0;
}
