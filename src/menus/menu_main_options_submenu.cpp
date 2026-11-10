//
// Created by cpasjuste on 07/12/18.
//

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
    highlight_selection = new Highlight({MenuMainOptionsSubmenu::getSize().x, BUTTON_HEIGHT * main->getScaling().y},
                                        Highlight::CursorPosition::Left);
    highlight_selection->setFillColor(COLOR_BLUE);
    highlight_selection->setAlpha(60);
    highlight_selection->setCursorColor(COLOR_RED);
    highlight_selection->setOrigin(Origin::Left);
    highlight_selection->setPosition(0, 200 * main->getScaling().y);
    highlight_selection->setLayer(-1);
    MenuMainOptionsSubmenu::add(highlight_selection);
}

void MenuMainOptionsSubmenu::setSelection(const std::string &value) {
    selected = value;
    onScroll();
}

// puts the "active entry" marker on its button (or hides it when it is scrolled out of the menu)
void MenuMainOptionsSubmenu::onScroll() {
    if (highlight_selection == nullptr) {
        return;
    }

    highlight_selection->setVisibility(Visibility::Hidden);
    const std::string wanted = Utility::toLower(selected);
    for (auto &button: buttons) {
        if (Utility::toLower(button->item.getValue()) == wanted) {
            highlight_selection->snapTo(button->getPosition());
            highlight_selection->setVisibility(button->isVisible() ? Visibility::Visible : Visibility::Hidden);
            break;
        }
    }
}

void MenuMainOptionsSubmenu::setVisibility(c2d::Visibility visibility, bool tweenPlay) {
    if (visibility == Visibility::Visible) {
        // open on the active entry, in the middle of the menu when the list is long
        const std::string wanted = Utility::toLower(selected);
        for (size_t i = 0; i < buttons.size(); i++) {
            if (Utility::toLower(buttons[i]->item.getValue()) == wanted) {
                setIndex((int) i);
                break;
            }
        }
    }
    if (option_name == OPT_REMOTE) {
        // the address to open on the phone stays on screen while this list is open
        if (visibility == Visibility::Visible) {
            main->getStatus()->show("Phone remote", main->getRemoteInfo(), true);
        } else {
            main->getStatus()->hide();
        }
    }
    Menu::setVisibility(visibility, tweenPlay);
}

void MenuMainOptionsSubmenu::onOptionSelection(MenuItem *item) {
    const std::string value = item->getValue();

#ifdef __SWITCH__
    if (option_name == OPT_CPU_BOOST) {
        if (value == "Enabled") {
            pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Max);
        } else {
            pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
        }
    }
#endif

    auto *option = main->getConfig()->getOption(option_name);
    if (option == nullptr) {
        return;
    }
    option->setString(value);
    main->getConfig()->save();
    setSelection(value);

    if (option_name == OPT_REMOTE) {
        // start/stop the server now and show where to connect
        main->applyRemoteOption();
        main->getStatus()->show("Phone remote", main->getRemoteInfo(), true);
    }

    if (option_name == OPT_HWDEC || option_name == OPT_SW_SPEED) {
        // the player (and libmpv) is configured once, when pplay starts
        main->getStatus()->show("Information...", "Restart pplay to apply this setting");
    }
}

bool MenuMainOptionsSubmenu::onInput(c2d::Input::Player *players) {
    if (players[0].keys & Input::Right || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        main->getMenuMain()->getMenuMainOptions()->setVisibility(Visibility::Visible, true);
        return true;
    }

    return Menu::onInput(players);
}
