//
// Created by cpasjuste on 12/12/18.
//

#include "main.h"
#include "utility.h"
#include "player_osd.h"

using namespace c2d;

#define OSD_HEIGHT      80.0f
#define OSD_HIDE_TIME   4.0f

PlayerOSD::PlayerOSD(Main *m) : Rectangle({64, 64}) {

    main = m;

    setSize(main->getSize().x - (200 * main->getScaling()), OSD_HEIGHT * main->getScaling());
    setPosition(main->getSize().x / 2, main->getSize().y + getSize().y);
    setOrigin(Origin::Bottom);

    highlight = new Highlight({getSize().y, 64 * main->getScaling()});
    highlight->setOrigin(Origin::Left);
    highlight->setRotation(90);
    highlight->setPosition(64 * main->getScaling(), 0);
    add(highlight);

    progress = new Progress({0, 0, getSize().x, 6 * main->getScaling()});
    progress->setFgColor(COLOR_RED);
    progress->setBgColor(COLOR_GRAY);
    progress->setOutlineThickness(0);
    add(progress);

    progress_text = new Text("00:00:00", main->getFontSize(Main::FontSize::Medium), main->getFont());
    progress_text->setOrigin(Origin::Left);
    progress_text->setPosition((-100 + 20) * main->getScaling(), getSize().y / 2);
    add(progress_text);

    duration_text = new Text("00:00:00", main->getFontSize(Main::FontSize::Medium), main->getFont());
    duration_text->setOrigin(Origin::Left);
    duration_text->setPosition(getSize().x - (20 * main->getScaling()), getSize().y / 2);
    add(duration_text);

    auto btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_pause.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(64 * 1 * main->getScaling(), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    // left buttons
    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_backward_10.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(64 * 2 * main->getScaling(), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_backward_1.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(64 * 3 * main->getScaling(), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    // right buttons
    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_forward_1.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(getSize().x - (64 * 3 * main->getScaling()), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_forward_10.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(getSize().x - (64 * 2 * main->getScaling()), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_stop.png");
    btn->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn->setPosition(getSize().x - (64 * 1 * main->getScaling()), getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    // btn_play for btn_pause
    btn_play = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_play.png");
    btn_play->setSize(32 * main->getScaling(), 32 * main->getScaling());
    btn_play->setPosition(buttons.at((size_t) ButtonID::Pause)->getPosition().x, getSize().y / 2);
    btn_play->setOrigin(Origin::Center);
    btn_play->setVisibility(Visibility::Hidden);
    add(btn_play);

    // TITLE
    title = new Text("", main->getFontSize(Main::FontSize::Medium), main->getFont());
    title->setFillColor(COLOR_RED);
    title->setOrigin(Origin::Left);
    title->setPosition((64 * 3 + 50) * main->getScaling(), getSize().y / 2);
    title->setWidth(buttons.at((int) ButtonID::SeekForward1)->getPosition().x - title->getPosition().x - 32);

    add(title);


    add(new TweenPosition({getPosition().x, getPosition().y},
                          {getPosition().x, getPosition().y - getSize().y}, 0.5f));

    setVisibility(Visibility::Hidden);

    clock.restart();
}

void PlayerOSD::setVisibility(c2d::Visibility visibility, bool tweenPlay) {

    if (main->getPlayer()) {
        title->setString(main->getPlayer()->getTitle());
    }

    if (visibility == Visibility::Visible) {
        index = 0;
        highlight->setPosition(buttons.at((size_t) index)->getPosition().x, 0);
    }

    clock.restart();
    C2DObject::setVisibility(visibility, tweenPlay);
}

void PlayerOSD::onDraw(c2d::Transform &transform, bool draw) {

    if (main->getPlayer()->isStopped() || !main->getPlayer()->isFullscreen()) {
        setVisibility(Visibility::Hidden, false);
        return;
    }

    if (clock.getElapsedTime().asSeconds() >= OSD_HIDE_TIME) {
        setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
    }

    if (main->getPlayer()->isPlaying()) {
        position = (float) Kit_GetPlayerPosition(main->getPlayer()->getKitPlayer());
        duration = (float) Kit_GetPlayerDuration(main->getPlayer()->getKitPlayer());
        progress->setProgress(position / duration);
        progress_text->setString(pplay::Utility::formatTime(position));
        duration_text->setString(pplay::Utility::formatTime(duration));
    }

    C2DObject::onDraw(transform);
}

bool PlayerOSD::onInput(c2d::Input::Player *players) {

    if (main->getPlayer()->isStopped() || !main->getPlayer()->isFullscreen()) {
        setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
        return true;
    }

    unsigned int keys = players[0].keys;

    if ((keys & Input::Key::Up) || keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
    } else if (keys & Input::Key::Left) {
        index--;
        if (index < 0) {
            index = (int) buttons.size() - 1;
        }
        highlight->setPosition(buttons.at((size_t) index)->getPosition().x, 0);
        clock.restart();
    } else if (keys & Input::Key::Right) {
        index++;
        if (index >= (int) buttons.size()) {
            index = 0;
        }
        highlight->setPosition(buttons.at((size_t) index)->getPosition().x, 0);
        clock.restart();
    } else if (keys & Input::Key::Fire1) {
        if (index == (int) ButtonID::Pause) {
            bool pause = main->getPlayer()->isPlaying();
            btn_play->setVisibility(pause ? Visibility::Visible : Visibility::Hidden);
            buttons.at((int) ButtonID::Pause)->setVisibility(pause ? Visibility::Hidden : Visibility::Visible);
            if (pause) {
                main->getStatus()->show("Info...", "Pausing playback...");
                main->getPlayer()->pause();
            } else {
                main->getStatus()->show("Info...", "Resuming playback...");
                main->getPlayer()->resume();
            }
        } else if (index == (int) ButtonID::SeekForward1) {
            main->getPlayer()->seek(position + 60.0);
        } else if (index == (int) ButtonID::SeekForward10) {
            main->getPlayer()->seek(position + (60.0 * 10.0));
        } else if (index == (int) ButtonID::SeekBackward1) {
            main->getPlayer()->seek(position - 60.0);
        } else if (index == (int) ButtonID::SeekBackward10) {
            main->getPlayer()->seek(position - (60.0 * 10.0));
        } else if (index == (int) ButtonID::Stop) {
            setVisibility(Visibility::Hidden);
            main->getStatus()->show("Info...", "Stopping playback...");
            main->getPlayer()->stop();
            main->getPlayer()->setFullscreen(false);
        }

        clock.restart();
    }

    return true;
}

void PlayerOSD::reset() {
    btn_play->setVisibility(Visibility::Hidden);
    buttons.at((int) ButtonID::Pause)->setVisibility(Visibility::Visible);
    clock.restart();
}
