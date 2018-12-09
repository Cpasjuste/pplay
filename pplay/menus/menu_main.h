//
// Created by cpasjuste on 07/12/18.
//

#ifndef PPLAY_MENU_MAIN_H
#define PPLAY_MENU_MAIN_H

#include "menu.h"

class MenuMain : public Menu {

public:

    MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items);

private:

    bool onInput(c2d::Input::Player *players) override;

    void onOptionSelection(MenuItem *item) override;
};


#endif //PPLAY_MENU_MAIN_H
