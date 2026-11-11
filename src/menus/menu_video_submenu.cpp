//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_video_submenu.h"

using namespace c2d;

MenuVideoSubmenu::MenuVideoSubmenu(
        Main *main, const c2d::FloatRect &rect, const std::string &_title,
        const std::vector<MenuItem> &items, int type) : Menu(main, rect, _title, items, false) {

    this->type = type;

    // highlight
    highlight_selection = new Highlight({MenuVideoSubmenu::getSize().x, BUTTON_HEIGHT * main->getScaling().y},
                                        Highlight::CursorPosition::Left);
    highlight_selection->setFillColor(COLOR_BLUE);
    highlight_selection->setAlpha(60);
    highlight_selection->setCursorColor(COLOR_RED);
    highlight_selection->setOrigin(Origin::Left);
    highlight_selection->setPosition(0, 200 * main->getScaling().y);
    highlight_selection->setLayer(-1);
    MenuVideoSubmenu::add(highlight_selection);
}

void MenuVideoSubmenu::setSelection(int streamId) {
    selectedId = streamId;
    for (auto &button: buttons) {
        if (button->item.id == streamId) {
            highlight_selection->snapTo(button->getPosition());
            // the active stream may be scrolled out of the menu
            highlight_selection->setVisibility(button->isVisible() ? c2d::Visibility::Visible
                                                                   : c2d::Visibility::Hidden);
            break;
        }
    }
}

void MenuVideoSubmenu::onScroll() {
    if (highlight_selection == nullptr) {
        return;
    }
    for (auto &button: buttons) {
        if (button->item.id == selectedId) {
            highlight_selection->snapTo(button->getPosition());
            highlight_selection->setVisibility(button->isVisible() ? c2d::Visibility::Visible
                                                                   : c2d::Visibility::Hidden);
            break;
        }
    }
}

void MenuVideoSubmenu::onOptionSelection(MenuItem *item) {
    if (item->position == MenuItem::Position::Top) {
        std::string name = item->name;
        std::replace(name.begin(), name.end(), '\n', ' ');
        if (type == MENU_VIDEO_TYPE_VID && main->getPlayer()->getVideoStream() != item->id) {
            main->getStatus()->show("Please Wait...",
                                    "Loading video stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setVideoStream(item->id);
            setSelection(item->id);
        } else if (type == MENU_VIDEO_TYPE_AUD && main->getPlayer()->getAudioStream() != item->id) {
            main->getStatus()->show("Please Wait...",
                                    "Loading audio stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setAudioStream(item->id);
            setSelection(item->id);
        } else if (type == MENU_VIDEO_TYPE_SUB && main->getPlayer()->getSubtitleStream() != item->id) {
            main->getStatus()->show("Please Wait...",
                                    "Loading subtitle stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setSubtitleStream(item->id);
            setSelection(item->id);
        }
    }
}

void MenuVideoSubmenu::setVisibility(c2d::Visibility visibility, bool tweenPlay) {
    int activeId = -2;
    if (type == MENU_VIDEO_TYPE_VID) {
        activeId = main->getPlayer()->getVideoStream();
    } else if (type == MENU_VIDEO_TYPE_AUD) {
        activeId = main->getPlayer()->getAudioStream();
    } else if (type == MENU_VIDEO_TYPE_SUB) {
        activeId = main->getPlayer()->getSubtitleStream();
    }

    if (visibility == Visibility::Visible) {
        // open the list with the cursor on the stream in use, in the middle of the menu
        for (size_t i = 0; i < buttons.size(); i++) {
            if (buttons[i]->item.position == MenuItem::Position::Top && buttons[i]->item.id == activeId) {
                setIndex((int) i);
                break;
            }
        }
    }

    setSelection(activeId);
    Menu::setVisibility(visibility, tweenPlay);
}

bool MenuVideoSubmenu::onInput(c2d::Input::Player *players) {
    if (players[0].keys & Input::Left || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        main->getMenuVideo()->setVisibility(Visibility::Visible, true);
        return true;
    }

    return Menu::onInput(players);
}