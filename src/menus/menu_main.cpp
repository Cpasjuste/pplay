//
// Created by cpasjuste on 07/12/18.
//

#include "main.h"
#include "menu_main.h"

using namespace c2d;

MenuMain::MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, "PPLAY", items, true) {

    std::vector<MenuItem> it;

    it.emplace_back(OPT_CPU_BOOST, "cpu.png", MenuItem::Position::Top);
#ifdef __SWITCH__
    it.emplace_back(OPT_UMS_DEVICE, "usb.png", MenuItem::Position::Top);
#endif
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

#ifdef __SWITCH__
    std::string umsPath;
    it.clear();
    for (int i = 0; i <= 9; i++) {
        umsPath = "ums" + std::to_string(i) + ":/";
        it.emplace_back(umsPath, "", MenuItem::Position::Top);
    }
    menuMainOptionsUsb = new MenuMainOptionsSubmenu(main, rect, OPT_UMS_DEVICE, it, OPT_UMS_DEVICE);
    menuMainOptionsUsb->setLayer(2);
    menuMainOptionsUsb->setVisibility(Visibility::Hidden, false);
    menuMainOptionsUsb->setSelection(main->getConfig()->getOption(OPT_UMS_DEVICE)->getString());
    main->add(menuMainOptionsUsb);
#endif
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
#ifdef __SWITCH__
    } else if (item->name == "Usb") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Usb);
#endif
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
           || menuMainOptionsCpu->isVisible()
           #ifdef __SWITCH__
           || menuMainOptionsUsb->isVisible()
#endif
            ;
}

MenuMainOptions *MenuMain::getMenuMainOptions() {
    return menuMainOptions;
}

MenuMainOptionsSubmenu *MenuMain::getMenuMainOptionsSubmenu(const std::string &name) {
    if (name == OPT_CPU_BOOST) {
        return menuMainOptionsCpu;
    }
#ifdef __SWITCH__
    if (name == OPT_UMS_DEVICE) {
        return menuMainOptionsUsb;
    }
#endif
    return nullptr;
}
