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

void MenuVideoSubmenu::setSelection(int streamType) {

    // TODO
    int stream_id = -1;
    if (streamType == MENU_VIDEO_TYPE_VID) {
        //stream_id = main->getPlayer()->getVideoStreams()->getCurrent();
    } else if (streamType == MENU_VIDEO_TYPE_AUD) {
        //stream_id = main->getPlayer()->getAudioStreams()->getCurrent();
    } else if (streamType == MENU_VIDEO_TYPE_SUB && main->getPlayer()->isSubtitlesEnabled()) {
        //stream_id = main->getPlayer()->getSubtitlesStreams()->getCurrent();
    }

    for (auto &button : buttons) {
        if (button->item.id == stream_id) {
            highlight_selection->setPosition(button->getPosition());
            break;
        }
    }
}

void MenuVideoSubmenu::onOptionSelection(MenuItem *item) {

    if (item->position == MenuItem::Position::Top) {
        if (type == MENU_VIDEO_TYPE_VID) {
            main->getStatus()->show("Please Wait...",
                                    "Loading video stream: " + item->name + ". This can take a few seconds...");
            main->getPlayer()->setVideoStream(item->id);
            setSelection(MENU_VIDEO_TYPE_VID);
        } else if (type == MENU_VIDEO_TYPE_AUD) {
            main->getStatus()->show("Please Wait...",
                                    "Loading audio stream: " + item->name + ". This can take a few seconds...");
            main->getPlayer()->setAudioStream(item->id);
            setSelection(MENU_VIDEO_TYPE_AUD);
        }
        if (type == MENU_VIDEO_TYPE_SUB) {
            main->getStatus()->show("Please Wait...",
                                    "Loading subtitle stream: " + item->name + ". This can take a few seconds...");
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
