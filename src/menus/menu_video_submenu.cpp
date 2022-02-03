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

void MenuVideoSubmenu::setSelection(int streamType) {
    int stream_id = -1;
    if (streamType == MENU_VIDEO_TYPE_VID) {
        stream_id = main->getPlayer()->getVideoStream();
    } else if (streamType == MENU_VIDEO_TYPE_AUD) {
        stream_id = main->getPlayer()->getAudioStream();
    } else if (streamType == MENU_VIDEO_TYPE_SUB) {
        stream_id = main->getPlayer()->getSubtitleStream();
    }

    for (auto &button: buttons) {
        if (button->item.id == stream_id) {
            highlight_selection->tweenTo(button->getPosition());
            break;
        }
    }
}

void MenuVideoSubmenu::onOptionSelection(MenuItem *item) {
    if (item->position == MenuItem::Position::Top) {
        std::string name = item->name;
        std::replace(name.begin(), name.end(), '\n', ' ');
        if (type == MENU_VIDEO_TYPE_VID) {
            main->getStatus()->show("Please Wait...",
                                    "Loading video stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setVideoStream(item->id);
            setSelection(MENU_VIDEO_TYPE_VID);
        } else if (type == MENU_VIDEO_TYPE_AUD) {
            main->getStatus()->show("Please Wait...",
                                    "Loading audio stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setAudioStream(item->id);
            setSelection(MENU_VIDEO_TYPE_AUD);
        } else if (type == MENU_VIDEO_TYPE_SUB) {
            main->getStatus()->show("Please Wait...",
                                    "Loading subtitle stream: " + name + ".\nThis can take a few seconds...");
            main->getPlayer()->setSubtitleStream(item->id);
            setSelection(MENU_VIDEO_TYPE_SUB);
        }
    }
}

bool MenuVideoSubmenu::onInput(c2d::Input::Player *players) {
    if (players[0].keys & Input::Left || players[0].keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        main->getMenuVideo()->setVisibility(Visibility::Visible, true);
        return true;
    }

    return Menu::onInput(players);
}
