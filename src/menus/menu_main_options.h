//
// Created by cpasjuste on 07/12/18.
//

#ifndef PPLAY_MENU_MAIN_OPTIONS_H
#define PPLAY_MENU_MAIN_OPTIONS_H

#include "menu.h"

class MenuMainOptions : public Menu {

public:

    MenuMainOptions(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items);

private:

    bool onInput(c2d::Input::Player *players) override;

    void onOptionSelection(MenuItem *item) override;
};


#endif //PPLAY_MENU_MAIN_OPTIONS_H
