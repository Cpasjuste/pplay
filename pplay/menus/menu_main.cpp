//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_main.h"

using namespace c2d;

MenuMain::MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, items, true) {

}

void MenuMain::onOptionSelection(MenuItem *item) {
    if (item->name == "Home") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Home);
    } else if (item->name == "Network") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Network);
    } else if (item->name == "Exit") {
        main->quit();
    }
}

bool MenuMain::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Right) {
        setVisibility(Visibility::Hidden, true);
        return true;
    }

    return Menu::onInput(players);
}
