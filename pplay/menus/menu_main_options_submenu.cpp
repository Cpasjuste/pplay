//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "utility.h"
#include "menu_main_options_submenu.h"

using namespace c2d;

MenuMainOptionsSubmenu::MenuMainOptionsSubmenu(
        Main *main, const c2d::FloatRect &rect, const std::string &_title,
        const std::vector<MenuItem> &items, const std::string &optionName)
        : Menu(main, rect, _title, items, true) {

    option_name = optionName;

    // highlight
    highlight_selection = new Highlight({getSize().x, BUTTON_HEIGHT * main->getScaling()},
                                        Highlight::CursorPosition::Left);
    highlight_selection->setFillColor(COLOR_BLUE);
    highlight_selection->setAlpha(40);
    highlight_selection->setCursorColor(COLOR_RED);
    highlight_selection->setOrigin(Origin::Left);
    highlight_selection->setPosition(0, 200);
    highlight_selection->setLayer(-1);
    add(highlight_selection);
}

void MenuMainOptionsSubmenu::setSelection(const std::string &name) {

    for (auto btn : buttons) {
        if (Utility::toLower(btn->item.name) == Utility::toLower(name)) {
            highlight_selection->setPosition(btn->getPosition());
        }
    }
}

void MenuMainOptionsSubmenu::onOptionSelection(MenuItem *item) {

    if (option_name == OPT_CPU_BOOST) {
        if (item->name == "Enabled") {
            pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Max);
        } else {
            pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
        }
    }

    main->getConfig()->getOption(option_name)->setString(item->name);
    main->getConfig()->save();
    setSelection(item->name);
}

bool MenuMainOptionsSubmenu::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Right || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        main->getMenuMain()->getMenuMainOptions()->setVisibility(Visibility::Visible, true);
        return true;
    }

    return Menu::onInput(players);
}
