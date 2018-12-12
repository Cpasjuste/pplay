//
// Created by cpasjuste on 07/12/18.
//

#ifndef PPLAY_MENU_VIDEO_SUBMENU_H
#define PPLAY_MENU_VIDEO_SUBMENU_H

#include "menu.h"

#define MENU_VIDEO_TYPE_VID 0
#define MENU_VIDEO_TYPE_AUD 1
#define MENU_VIDEO_TYPE_SUB 2

class MenuVideoSubmenu : public Menu {

public:

    MenuVideoSubmenu(Main *main, const c2d::FloatRect &rect,
                     const std::string &_title, const std::vector<MenuItem> &items, int type);

    bool onInput(c2d::Input::Player *players) override;

    int type;
    Highlight *highlight;

private:

    void onOptionSelection(MenuItem *item) override;
};

#endif //PPLAY_MENU_VIDEO_SUBMENU_H
