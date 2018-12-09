//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_video_submenu.h"

using namespace c2d;

MenuVideoSubmenu::MenuVideoSubmenu(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items, int type)
        : Menu(main, rect, items, false) {
    this->type = type;
}

void MenuVideoSubmenu::onOptionSelection(MenuItem *item) {

    if (item->position == MenuItem::Position::Top) {
        if (type == MENU_VIDEO_TYPE_VID) {
            main->getPlayer()->setVideoStream(item->id);
            main->getStatus()->show("Video...",
                                    "Loading video: " + item->name + ". This can take a few seconds...");
        } else if (type == MENU_VIDEO_TYPE_AUD) {
            main->getPlayer()->setAudioStream(item->id);
            main->getStatus()->show("Audio...",
                                    "Loading audio: " + item->name + ". This can take a few seconds...");
        }
        if (type == MENU_VIDEO_TYPE_SUB) {
            main->getPlayer()->setSubtitleStream(item->id);
            main->getStatus()->show("Subtitles...",
                                    "Loading subtitle: " + item->name + ". This can take a few seconds...");
        }
    }
}

bool MenuVideoSubmenu::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Left) {
        setVisibility(Visibility::Hidden, true);
        main->getMenuVideo()->setVisibility(Visibility::Visible, true);
        return true;
    }

    return Menu::onInput(players);
}
