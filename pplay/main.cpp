//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filers/filer_sdmc.h"
#include "filers/filer_http.h"
#include "filers/filer_ftp.h"
#include "menus/menu_main.h"
#include "menus/menu_video.h"
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
            if (appletGetFocusState() == AppletFocusState_Focused) {
                if (main->getPlayer()->isPaused()) {
                    main->getPlayer()->resume();
                }
            } else {
                if (!main->getPlayer()->isPaused()) {
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

Main::Main(const c2d::Vector2f &size) : C2DRenderer(size) {

    setClearColor(Color::Black);

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
    font->loadFromFile(getIo()->getDataReadPath() + "skin/font.ttf");
    font->setFilter(Texture::Filter::Point);
    font->setOffset({0, -4});

    statusBox = new StatusBox(this, {10, getSize().y - 16});
    statusBox->setOrigin(Origin::BottomLeft);
    statusBox->setLayer(10);
    add(statusBox);

    // media info
    getIo()->create(getIo()->getDataWritePath() + "cache");
    mediaInfoThread = new MediaThread(this, getIo()->getDataWritePath() + "cache/");

    // create filers
    // sdmc
    FloatRect filerRect = {0, 0, (getSize().x / 2) - 16, getSize().y - 32 - 64};
    filerSdmc = new FilerSdmc(this, "/", filerRect);
    filerSdmc->setLayer(1);
    add(filerSdmc);
    // http
    filerHttp = new FilerHttp(this, filerRect);
    filerHttp->setLayer(1);
    filerHttp->setVisibility(Visibility::Hidden);
    add(filerHttp);
    // ftp
    filerFtp = new FilerFtp(this, filerRect);
    filerFtp->setLayer(1);
    filerFtp->setVisibility(Visibility::Hidden);
    add(filerFtp);
#ifdef __SMB_SUPPORT__
    // smb
    filerSmb = new FilerSmb(this, filerRect);
    filerSmb->setLayer(1);
    filerSmb->setVisibility(Visibility::Hidden);
    add(filerSmb);
#endif
    filer = filerSdmc;
    filer->getDir(config->getOption(OPT_LAST_PATH)->getString());

    // status bar
    statusBar = new StatusBar(this);
    statusBar->setLayer(10);
    add(statusBar);

    // title image
    title = new C2DTexture(getIo()->getDataReadPath() + "skin/pplay.png");
    title->setOrigin(Origin::BottomRight);
    title->setPosition(getSize().x - 16, getSize().y - 16);
    title->add(new TweenAlpha(0, 255, 0.5f));
    add(title);

    // ffmpeg player
    player = new Player(this);
    add(player);

    // main menu
    std::vector<MenuItem> items;
    items.emplace_back("Home", "home.png", MenuItem::Position::Top);
    items.emplace_back("Network", "network.png", MenuItem::Position::Top);
    items.emplace_back("Options", "options.png", MenuItem::Position::Top);
    items.emplace_back("Exit", "exit.png", MenuItem::Position::Bottom);
    menu_main = new MenuMain(this, {-250 * scaling, 0, 250 * scaling, getSize().y}, items);
    menu_main->setVisibility(Visibility::Hidden, false);
    menu_main->setLayer(2);
    add(menu_main);

    // video menu
    items.clear();
    items.emplace_back("Video", "video.png", MenuItem::Position::Top);
    items.emplace_back("Audio", "audio.png", MenuItem::Position::Top);
    items.emplace_back("Subtitles", "subtitles.png", MenuItem::Position::Top);
    items.emplace_back("Stop", "exit.png", MenuItem::Position::Bottom);
    menu_video = new MenuVideo(this, {getSize().x, 0, 250 * scaling, getSize().y}, items);
    menu_video->setVisibility(Visibility::Hidden, false);
    menu_video->setLayer(2);
    add(menu_video);

    // a messagebox...
    float w = getSize().x / 2;
    float h = getSize().y / 3;
    messageBox = new MessageBox({getSize().x / 2, getSize().y / 2, w, h},
                                getInput(), getFont(), getFontSize(Main::FontSize::Medium));
    messageBox->setOrigin(Origin::Center);
    messageBox->setFillColor(COLOR_BG);
    messageBox->setAlpha(200);
    messageBox->setOutlineColor(COLOR_RED);
    messageBox->setOutlineThickness(2);
    messageBox->getTitleText()->setOutlineThickness(0);
    messageBox->getMessageText()->setOutlineThickness(0);
    add(messageBox);
}

Main::~Main() {
    delete (mediaInfoThread);
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
    } else if (keys & Input::Touch) {
        if (player->getGlobalBounds().contains(players[0].touch)) {
            if (!player->isStopped() && !player->isFullscreen()) {
                player->setFullscreen(true);
                return true;
            }
        }
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

// TODO: move this in menu_main
void Main::show(MenuType type) {

    if (player->isStopped() && player->isFullscreen()) {
        player->setFullscreen(false);
    }

    if (type == MenuType::Home) {
        filerSdmc->setVisibility(Visibility::Visible);
        filerHttp->setVisibility(Visibility::Hidden);
        filerFtp->setVisibility(Visibility::Hidden);
#ifdef __SMB_SUPPORT__
        filerSmb->setVisibility(Visibility::Hidden);
#endif
        filer = filerSdmc;
        if (!filer->getDir(config->getOption(OPT_HOME_PATH)->getString())) {
            if (filer->getDir("/")) {
                filer->clearHistory();
            }
        }
        return;
    }

    filerSdmc->setVisibility(Visibility::Hidden);

    std::string net_path = config->getOption(OPT_NETWORK)->getString();
    if (Utility::startWith(net_path, "http:")) {
        filerFtp->setVisibility(Visibility::Hidden);
#ifdef __SMB_SUPPORT__
        filerSmb->setVisibility(Visibility::Hidden);
#endif
        filerHttp->setVisibility(Visibility::Visible);
        filer = filerHttp;
    } else if (Utility::startWith(net_path, "ftp:")) {
        filerHttp->setVisibility(Visibility::Hidden);
#ifdef __SMB_SUPPORT__
        filerSmb->setVisibility(Visibility::Hidden);
#endif
        filerFtp->setVisibility(Visibility::Visible);
        filer = filerFtp;
#ifdef __SMB_SUPPORT__
    } else if (Utility::startWith(net_path, "smb:")) {
        filerHttp->setVisibility(Visibility::Hidden);
        filerFtp->setVisibility(Visibility::Hidden);
        filerSmb->setVisibility(Visibility::Visible);
        filer = filerSmb;
    } else {
#else
    } else {
#endif
        messageBox->show("OOPS", "NETWORK path is wrong (see pplay.cfg)", "OK");
        show(MenuType::Home);
        return;
    }

    if (!filer->getDir(net_path)) {
        messageBox->show("OOPS", filer->getError(), "OK");
        show(MenuType::Home);
    } else {
        filer->clearHistory();
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
    if (filerSdmc->isVisible()) {
        config->getOption(OPT_LAST_PATH)->setString(filer->getPath());
        config->save();
    }

    exit = true;
    if (player->isStopped()) {
        running = false;
    } else {
        player->stop();
    }
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

c2d::Texture *Main::getTitle() {
    return title;
}

StatusBar *Main::getStatusBar() {
    return statusBar;
}

int main() {

    Vector2f size = {1280, 720};

#ifdef __SWITCH__
#ifndef __NET_DEBUG__
    socketInitializeDefault();
#endif
    appletMainLoop();
    if (appletGetOperationMode() == AppletOperationMode_Docked) {
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
    appletUnhook(&applet_hook_cookie);
    appletUnlockExit();
#ifndef __NET_DEBUG__
    socketExit();
#endif
#endif

    return 0;
}
