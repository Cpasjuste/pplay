//
// Created by cpasjuste on 07/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu_video.h"

using namespace c2d;

MenuVideo::MenuVideo(Main *main, const c2d::FloatRect &rect, const std::vector<MenuItem> &items)
        : Menu(main, rect, "OPTIONS______", items, false) {}

void MenuVideo::onOptionSelection(MenuItem *item) {

    if (item->name == "Video") {
        if (main->getPlayer()->getMenuVideoStreams()) {
            main->getPlayer()->getMenuVideoStreams()->setVisibility(Visibility::Visible, true);
            setVisibility(Visibility::Hidden, true);
        } else {
            main->getStatus()->show("Information...", "No video streams found in media", false, false);
        }
    } else if (item->name == "Audio") {
        if (main->getPlayer()->getMenuAudioStreams()) {
            main->getPlayer()->getMenuAudioStreams()->setVisibility(Visibility::Visible, true);
            setVisibility(Visibility::Hidden, true);
        } else {
            main->getStatus()->show("Information...", "No audio streams found in media", false, false);
        }
    } else if (item->name == "Subtitles") {
        if (main->getPlayer()->getMenuSubtitlesStreams()) {
            main->getPlayer()->getMenuSubtitlesStreams()->setVisibility(Visibility::Visible, true);
            setVisibility(Visibility::Hidden, true);
        } else {
            main->getStatus()->show("Information...", "No subtitles streams found in media", false, false);
        }
    } else if (item->name == "Stop") {
        main->getPlayer()->stop();
        main->getPlayer()->setFullscreen(false);
        setVisibility(Visibility::Hidden, true);
    }
}

bool MenuVideo::onInput(c2d::Input::Player *players) {

    if (players[0].keys & Input::Left) {
        setVisibility(Visibility::Hidden, true);
        return true;
    }

    if (players[0].keys & Input::Right) {
        MenuItem *item = getSelection();
        if (item->name != "Stop") {
            onOptionSelection(item);
        }
        return true;
    }

    return Menu::onInput(players);
}
