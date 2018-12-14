//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_video_submenu.h"

using namespace c2d;

MenuVideoSubmenu::MenuVideoSubmenu(
        Main *main, const c2d::FloatRect &rect, const std::string &_title,
        const std::vector<MenuItem> &items, int type)
        : Menu(main, rect, _title, items, false) {

    this->type = type;

    // highlight
    highlight = new Highlight({getSize().x, BUTTON_HEIGHT * main->getScaling()}, Highlight::CursorPosition::Left);
    highlight->setFillColor(COLOR_BLUE);
    highlight->setAlpha(40);
    highlight->setCursorColor(COLOR_RED);
    highlight->setOrigin(Origin::Left);
    highlight->setPosition(0, 200);
    highlight->setLayer(-1);
    add(highlight);
}

void MenuVideoSubmenu::onOptionSelection(MenuItem *item) {

    if (item->position == MenuItem::Position::Top) {
        if (type == MENU_VIDEO_TYPE_VID) {
            main->getStatus()->show("Video...",
                                    "Loading video: " + item->name + ". This can take a few seconds...");
            main->getPlayer()->setVideoStream(item->id);
            // update checked/current state
            int stream_id = main->getPlayer()->getVideoStreams()->getCurrentStream();
            for (auto &button : buttons) {
                if (button->item.id == stream_id) {
                    highlight->setPosition(button->getPosition());
                }
            }
        } else if (type == MENU_VIDEO_TYPE_AUD) {
            main->getStatus()->show("Audio...",
                                    "Loading audio: " + item->name + ". This can take a few seconds...");
            main->getPlayer()->setAudioStream(item->id);
            // update checked/current state
            int stream_id = main->getPlayer()->getAudioStreams()->getCurrentStream();
            for (auto &button : buttons) {
                if (button->item.id == stream_id) {
                    highlight->setPosition(button->getPosition());
                }
            }
        }
        if (type == MENU_VIDEO_TYPE_SUB) {
            main->getStatus()->show("Subtitles...",
                                    "Loading subtitle: " + item->name + ". This can take a few seconds...");
            main->getPlayer()->setSubtitleStream(item->id);
            // update checked/current state
            int stream_id = main->getPlayer()->getSubtitlesStreams()->getCurrentStream();
            for (auto &button : buttons) {
                if (button->item.id == stream_id) {
                    highlight->setPosition(button->getPosition());
                }
            }
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
