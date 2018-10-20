//
// Created by cpasjuste on 02/10/18.
//

#include "main.h"
#include "filer_sdmc.h"

using namespace c2d;
using namespace c2d::config;

Main::Main() {

    // create main renderer
    renderer = new C2DRenderer(Vector2f(1280, 720));
    renderer->setClearColor(COLOR_GRAY_LIGHT);

    // configure input
    getInput()->setRepeatEnable(true);
    getInput()->setRepeatDelay(INPUT_DELAY);

    // create a timer
    timer = new C2DClock();

    // init/load config file
    config = new Config("PPLAY", getIo()->getHomePath() + "pplay.cfg");
    initConfig();

    // create a rect
    mainRect = new C2DRectangle({renderer->getSize().x - 8, renderer->getSize().y - 8});
    mainRect->setFillColor(Color::Transparent);

    // create filers
    FloatRect filerRect = {16, 16,
                           (mainRect->getSize().x / 2) - 16, mainRect->getSize().y - 32};
    filerSdmc = new FilerSdmc(renderer->getIo(), ".", getFont(), FONT_SIZE, filerRect);
    mainRect->add(filerSdmc);
    filerHttp = new FilerHttp(getFont(), FONT_SIZE, filerRect);
    filerHttp->setVisibility(Visibility::Hidden);
    mainRect->add(filerHttp);
    filer = filerSdmc;

    // add all this crap
    renderer->add(mainRect);

    // ffmpeg player
    player = new Player(this);
    renderer->add(player);

    float w = renderer->getSize().x / 3;
    float h = renderer->getSize().y / 3;
    messageBox = new MessageBox({w, h, w, h},
                                renderer->getInput(), getFont(), FONT_SIZE);
    messageBox->setFillColor(COLOR_BLUE_LIGHT);
    messageBox->setOutlineColor(COLOR_GRAY);
    messageBox->setOutlineThickness(2);
    renderer->add(messageBox);
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
                } else if (!httpServerList.empty()) {
                    filer = filerHttp;
                    filer->getDir(httpServerList[0]);
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

void Main::initConfig() {

    //http://divers.klikissi.fr/telechargements/
    Group group("HTTP_SERVERS");
    group.addOption({"SERVER0", ""});
    group.addOption({"SERVER1", ""});
    group.addOption({"SERVER2", ""});
    group.addOption({"SERVER3", ""});
    group.addOption({"SERVER4", ""});
    config->addGroup(group);

    // load the configuration from file, overwriting default values
    if (!config->load()) {
        // file doesn't exist or is malformed, (re)create it
        config->save();
    }

    for (Option &option : *config->getGroup("HTTP_SERVERS")->getOptions()) {
        if (!option.getString().empty()) {
            httpServerList.emplace_back(option.getString());
            printf("server: %s\n", httpServerList[httpServerList.size() - 1].c_str());
        }
    }
}

c2d::Renderer *Main::getRenderer() {
    return renderer;
}

c2d::Io *Main::getIo() {
    return renderer->getIo();
}

c2d::Font *Main::getFont() {
    return renderer->getFont();
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
