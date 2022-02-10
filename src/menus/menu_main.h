//
// Created by cpasjuste on 07/12/18.
//

#ifndef PPLAY_MENU_MAIN_H
#define PPLAY_MENU_MAIN_H

#include "menu.h"
#include "menu_main_options.h"
#include "menu_main_options_submenu.h"

class MenuMain : public Menu {

public:

    MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items);

    MenuMainOptions *getMenuMainOptions();

    MenuMainOptionsSubmenu *getMenuMainOptionsSubmenu(const std::string &name);

    bool isMenuVisible();

private:

    bool onInput(c2d::Input::Player *players) override;

    void onOptionSelection(MenuItem *item) override;

    MenuMainOptions *menuMainOptions;
#ifdef __SWITCH__
    MenuMainOptionsSubmenu *menuMainOptionsCpu;
    MenuMainOptionsSubmenu *menuMainOptionsUsb;
#endif
};


#endif //PPLAY_MENU_MAIN_H
