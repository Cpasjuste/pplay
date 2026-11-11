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

    // mark the entry holding this value (as stored in pplay.cfg) as the active one
    void setSelection(const std::string &value);

    // the list opens with the cursor on the active entry
    void setVisibility(c2d::Visibility visibility, bool tweenPlay = true) override;

private:

    void onOptionSelection(MenuItem *item) override;

    void onScroll() override;

    Highlight *highlight_selection = nullptr;
    std::string option_name;
    std::string selected;
};

#endif //PPLAY_MENU_MAIN_OPTIONS_SUBMENU_H
