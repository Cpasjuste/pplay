//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_main.h"

using namespace c2d;

MenuMain::MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, "PPLAY", items, true) {

    // buffering options
    std::vector<MenuItem> it;
    it.emplace_back(OPT_CPU_BOOST, "cpu.png", MenuItem::Position::Top);
    it.emplace_back(OPT_BUFFER, "buffering.png", MenuItem::Position::Top);
    menuMainOptions = new MenuMainOptions(main, rect, it);
    menuMainOptions->setLayer(2);
    menuMainOptions->setVisibility(Visibility::Hidden, false);
    main->add(menuMainOptions);

    // Cpu Speed
    it.clear();
    it.emplace_back("Disabled", "", MenuItem::Position::Top);
    it.emplace_back("Enabled", "", MenuItem::Position::Top);
    menuMainOptionsCpu = new MenuMainOptionsSubmenu(main, rect, OPT_CPU_BOOST, it, OPT_CPU_BOOST);
    menuMainOptionsCpu->setLayer(2);
    menuMainOptionsCpu->setVisibility(Visibility::Hidden, false);
    menuMainOptionsCpu->setSelection(main->getConfig()->getOption(OPT_CPU_BOOST)->getString());
    main->add(menuMainOptionsCpu);

    // Buffering
    it.clear();
    it.emplace_back("Low", "", MenuItem::Position::Top);
    it.emplace_back("Medium", "", MenuItem::Position::Top);
    it.emplace_back("High", "", MenuItem::Position::Top);
    it.emplace_back("VeryHigh", "", MenuItem::Position::Top);
    menuMainOptionsBuffer = new MenuMainOptionsSubmenu(main, rect, OPT_BUFFER, it, OPT_BUFFER);
    menuMainOptionsBuffer->setLayer(2);
    menuMainOptionsBuffer->setVisibility(Visibility::Hidden, false);
    menuMainOptionsBuffer->setSelection(main->getConfig()->getOption(OPT_BUFFER)->getString());
    main->add(menuMainOptionsBuffer);

}

void MenuMain::onOptionSelection(MenuItem *item) {
    if (item->name == "Home") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Home);
    } else if (item->name == "Network") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Network);
    } else if (item->name == "Options") {
        setVisibility(Visibility::Hidden, true);
        menuMainOptions->setVisibility(Visibility::Visible);
    } else if (item->name == "Exit") {
        main->quit();
    }
}

bool MenuMain::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Right || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        return true;
    }

    if (players[0].keys & Input::Left) {
        MenuItem *item = getSelection();
        if (item->name == "Options") {
            onOptionSelection(item);
        }
        return true;
    }

    return Menu::onInput(players);
}

bool MenuMain::isMenuVisible() {
    return isVisible()
           || menuMainOptions->isVisible()
           || menuMainOptionsBuffer->isVisible()
           || menuMainOptionsCpu->isVisible();
}

MenuMainOptions *MenuMain::getMenuMainOptions() {
    return menuMainOptions;
}

MenuMainOptionsSubmenu *MenuMain::getMenuMainOptionsSubmenu(const std::string &name) {
    if (name == OPT_BUFFER) {
        return menuMainOptionsBuffer;
    }
    if (name == OPT_CPU_BOOST) {
        return menuMainOptionsCpu;
    }

    return nullptr;
}
