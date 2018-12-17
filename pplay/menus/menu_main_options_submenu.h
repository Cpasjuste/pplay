//
// Created by cpasjuste on 07/12/18.
//

#ifndef PPLAY_MENU_MAIN_OPTIONS_SUBMENU_H
#define PPLAY_MENU_MAIN_OPTIONS_SUBMENU_H

#include "menu.h"

class MenuMainOptionsSubmenu : public Menu {

public:

    MenuMainOptionsSubmenu(Main *main, const c2d::FloatRect &rect,
                           const std::string &_title, const std::vector<MenuItem> &items,
                           const std::string &optionName);

    bool onInput(c2d::Input::Player *players) override;

    void setSelection(const std::string &name);

private:

    void onOptionSelection(MenuItem *item) override;

    Highlight *highlight_selection;
    std::string option_name;
};

#endif //PPLAY_MENU_MAIN_OPTIONS_SUBMENU_H
