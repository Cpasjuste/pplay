//
// Created by cpasjuste on 02/10/18.
//

#ifndef PPLAY_MAIN_H
#define PPLAY_MAIN_H

#include "cross2d/c2d.h"
#include "filer.h"
#include "player.h"
#include "pplay_config.h"
#include "menu.h"
#include "menu_main.h"
#include "menu_video.h"
#include "status_box.h"
#include "status_bar.h"
#include "scrapper.h"
#include "io.h"
#include "usbfs.h"

#define INPUT_DELAY 500
#define ICON_SIZE 24
#define BUTTON_HEIGHT 64

#define COLOR_BG            Color(50, 45, 45, 255)
#define COLOR_FONT          Color(170, 170, 170, 255)
#define COLOR_HIGHLIGHT     Color(255, 255, 255, 40)

#define COLOR_CLOUD         Color(0xecf0f1ff)
#define COLOR_GRAY          Color(0x636e72ff)
#define COLOR_GRAY_LIGHT    Color(0xb2bec3ff)
#define COLOR_GRAY_DARK     Color(0x2d3436ff)
#define COLOR_BLUE          Color(0x0984e3ff)
#define COLOR_BLUE_LIGHT    Color(0x74b9ffff)
#define COLOR_PURPLE        Color(0x6c5ce7ff)
#define COLOR_PURPLE_LIGHT  Color(0xa29bfeff)
#define COLOR_GREEN         Color(0x1abc9cff)
#define COLOR_GREEN_LIGHT   Color(0x55efc4ff)
#define COLOR_ORANGE        Color(0xd35400ff)
#define COLOR_ORANGE_LIGHT  Color(0xe67e22ff)
#define COLOR_RED           Color(0xff0000ff)

class Main : public c2d::C2DRenderer {

public:

    enum class MenuType {
        Home,
        Network,
        Current,
        USB
    };

    enum class FontSize {
        Small = 16,
        Medium = 22,
        Big = 26
    };

    explicit Main(const c2d::Vector2f &size);

    ~Main() override;

    void show(MenuType type);

    bool isExiting();

    bool isRunning();

    void setRunningStop();

    MenuMain *getMenuMain();

    MenuVideo *getMenuVideo();

    Player *getPlayer();

    Filer *getFiler();

    PPLAYConfig *getConfig();

    c2d::Font *getFont() override;

    c2d::MessageBox *getMessageBox();

    StatusBox *getStatus();

    StatusBar *getStatusBar();

    pplay::Scrapper *getScrapper();

    c2d::Io *getIo() override;

    float getScaling();

    unsigned int getFontSize(FontSize fontSize);

    void quit();

private:

    bool onInput(c2d::Input::Player *players) override;

    void onDraw(c2d::Transform &transform, bool draw = true) override;

    pplay::Io *pplayIo = nullptr;
    c2d::Font *font = nullptr;
    c2d::Clock *timer = nullptr;
    c2d::MessageBox *messageBox = nullptr;
    StatusBox *statusBox = nullptr;
    PPLAYConfig *config = nullptr;
    Filer *filer = nullptr;
    StatusBar *statusBar = nullptr;
    Player *player = nullptr;
    MenuMain *menu_main = nullptr;
    MenuVideo *menu_video = nullptr;
    pplay::Scrapper *scrapper = nullptr;
    float scaling = 1;

    bool exit = false;
    bool running = true;
};

#endif //PPLAY_MAIN_H
