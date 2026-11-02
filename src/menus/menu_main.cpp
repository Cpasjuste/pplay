//
// Created by cpasjuste on 07/12/18.
//

#include "main.h"
#include "menu_main.h"

using namespace c2d;

namespace {

    // an entry of the "Options" menu and the submenu it opens
    struct OptionEntry {
        MenuItem entry;            // line shown in the "Options" menu
        std::string title;         // title of the submenu
        std::string optionName;    // option of pplay.cfg edited by the submenu
        std::vector<MenuItem> values;
    };

    // a selectable value: shown as "label", stored in pplay.cfg as "value"
    MenuItem choice(const std::string &label, const std::string &value) {
        return {label, "", MenuItem::Position::Top, 0, value};
    }
}

MenuMain::MenuMain(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, "PPLAY", items, true) {

    std::vector<OptionEntry> entries;
    const MenuItem::Position top = MenuItem::Position::Top;

#ifdef __SWITCH__
    entries.push_back({{OPT_CPU_BOOST, "cpu.png", top}, OPT_CPU_BOOST, OPT_CPU_BOOST,
                       {choice("Disabled", "Disabled"), choice("Enabled", "Enabled")}});

    std::vector<MenuItem> ums;
    for (int i = 0; i <= 9; i++) {
        std::string umsPath = "ums" + std::to_string(i) + ":/";
        ums.push_back(choice(umsPath, umsPath));
    }
    entries.push_back({{OPT_UMS_DEVICE, "usb.png", top}, OPT_UMS_DEVICE, OPT_UMS_DEVICE, ums});
#endif

#ifdef __PS4__
    // applied the next time pplay starts (the player is created at startup)
    entries.push_back({{"HW decoding", "video.png", top}, "HW DECODING", OPT_HWDEC,
                       {choice("Auto", "auto"),
                        choice("Off", "no"),
                        choice("Force (retry)", "force")}});
#endif

    // software decoding (Hi10, HEVC, ...): picture quality versus speed
    entries.push_back({{"SW speed", "cpu.png", top}, "SW SPEED", OPT_SW_SPEED,
                       {choice("Quality", "quality"),
                        choice("Balanced", "balanced"),
                        choice("Fast (less CPU)", "fast")}});

    // subtitles shown when a video starts, in the order of SUB_LANGS (pplay.cfg)
    entries.push_back({{"Auto subtitles", "subtitles.png", top}, "AUTO SUBS", OPT_SUB_AUTO,
                       {choice("On", "on"), choice("Off", "off")}});

    // language of the scrapper (movie/tv show information)
    entries.push_back({{"TMDB language", "network.png", top}, "TMDB LANG", OPT_TMDB_LANGUAGE,
                       {choice("English (en-US)", "en-US"),
                        choice("Spanish (es-ES)", "es-ES"),
                        choice("Spanish Latam (es-MX)", "es-MX"),
                        choice("French (fr-FR)", "fr-FR"),
                        choice("German (de-DE)", "de-DE"),
                        choice("Italian (it-IT)", "it-IT"),
                        choice("Portuguese (pt-BR)", "pt-BR"),
                        choice("Portuguese (pt-PT)", "pt-PT"),
                        choice("Dutch (nl-NL)", "nl-NL"),
                        choice("Polish (pl-PL)", "pl-PL"),
                        choice("Russian (ru-RU)", "ru-RU"),
                        choice("Turkish (tr-TR)", "tr-TR"),
                        choice("Japanese (ja-JP)", "ja-JP"),
                        choice("Korean (ko-KR)", "ko-KR"),
                        choice("Chinese (zh-CN)", "zh-CN")}});

    std::vector<MenuItem> it;
    for (auto &e: entries) {
        it.push_back(e.entry);
    }
    menuMainOptions = new MenuMainOptions(main, rect, it);
    menuMainOptions->setLayer(2);
    menuMainOptions->setVisibility(Visibility::Hidden, false);
    main->add(menuMainOptions);

    for (auto &e: entries) {
        auto *submenu = new MenuMainOptionsSubmenu(main, rect, e.title, e.values, e.optionName);
        submenu->setLayer(2);
        submenu->setVisibility(Visibility::Hidden, false);
        auto *option = main->getConfig()->getOption(e.optionName);
        if (option != nullptr) {
            submenu->setSelection(option->getString());
        }
        main->add(submenu);
        optionsSubmenus.emplace_back(e.entry.name, submenu);
    }
}

void MenuMain::onOptionSelection(MenuItem *item) {
    if (item->name == "Home") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Home);
    } else if (item->name == "Network") {
        setVisibility(Visibility::Hidden, true);
        main->show(Main::MenuType::Network);
    } else if (item->name == "Options" && !menuMainOptions->getButtons().empty()) {
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
    if (isVisible() || menuMainOptions->isVisible()) {
        return true;
    }
    for (auto &submenu: optionsSubmenus) {
        if (submenu.second->isVisible()) {
            return true;
        }
    }
    return false;
}

MenuMainOptions *MenuMain::getMenuMainOptions() {
    return menuMainOptions;
}

MenuMainOptionsSubmenu *MenuMain::getMenuMainOptionsSubmenu(const std::string &name) {
    for (auto &submenu: optionsSubmenus) {
        if (submenu.first == name) {
            return submenu.second;
        }
    }
    return nullptr;
}
