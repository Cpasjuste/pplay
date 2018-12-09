//
// Created by cpasjuste on 02/10/18.
//

#ifndef PPLAY_MAIN_H
#define PPLAY_MAIN_H

#include "cross2d/c2d.h"
#include "filers/filer.h"
#include "filers/filer_http.h"
#include "player/player.h"
#include "player/player_osd.h"
#include "config.h"
#include "media_thread.h"
#include "menus/menu.h"
#include "status_box.h"

#define INPUT_DELAY 250
#define FONT_SIZE 22

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
        Current
    };

    Main(const c2d::Vector2f &size);

    ~Main();

    void show(MenuType type);

    void setPlayerFullscreen(bool fs);

    bool isRunning();

    Menu *getMenuMain();

    Menu *getMenuVideo();

    MediaThread *getMediaThread();

    Player *getPlayer();

    Filer *getFiler();

    PPLAYConfig *getConfig();

    c2d::Font *getFont() override;

    c2d::MessageBox *getMessageBox();

    StatusBox *getStatus();

    void quit();

private:

    bool onInput(c2d::Input::Player *players) override;

    c2d::Font *font = nullptr;
    c2d::Clock *timer = nullptr;
    c2d::MessageBox *messageBox = nullptr;
    StatusBox *statusBox = nullptr;
    PPLAYConfig *config = nullptr;
    Filer *filerSdmc = nullptr;
    Filer *filerHttp = nullptr;
    Filer *filer = nullptr;
    Player *player = nullptr;
    Menu *menu_main = nullptr;
    Menu *menu_video = nullptr;
    MediaThread *mediaInfoThread = nullptr;

    bool running = true;
};

#endif //PPLAY_MAIN_H
