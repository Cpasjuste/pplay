//
// Created by cpasjuste on 12/12/18.
//

#include "main.h"
#include "utility.h"
#include "player_osd.h"

using namespace c2d;

#define OSD_HEIGHT      100.0f
#define OSD_HIDE_TIME   4.0f

PlayerOSD::PlayerOSD(Main *m) : Rectangle({64, 64}) {

    main = m;

    setSize(main->getSize().x - 200, OSD_HEIGHT);
    setPosition(main->getSize().x / 2, main->getSize().y + OSD_HEIGHT);
    setOrigin(Origin::Bottom);

    highlight = new Highlight({OSD_HEIGHT, 64});
    highlight->setOrigin(Origin::Left);
    highlight->setRotation(90);
    highlight->setPosition(64, 0);
    add(highlight);

    progress = new Progress({0, 0, getSize().x, 6});
    progress->setFgColor(COLOR_RED);
    progress->setBgColor(COLOR_GRAY);
    progress->setOutlineThickness(0);
    add(progress);

    auto btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_backward_10.png");
    btn->setSize(32, 32);
    btn->setPosition(64 * 1, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_backward_1.png");
    btn->setSize(32, 32);
    btn->setPosition(64 * 2, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_pause.png");
    btn->setSize(32, 32);
    btn->setPosition(64 * 3, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_forward_1.png");
    btn->setSize(32, 32);
    btn->setPosition(64 * 4, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_seek_forward_10.png");
    btn->setSize(32, 32);
    btn->setPosition(64 * 5, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);

    /*
    btn = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_stop.png");
    btn->setPosition(64 * 2, getSize().y / 2);
    btn->setOrigin(Origin::Center);
    add(btn);
    buttons.push_back(btn);
    */

    //
    btn_play = new C2DTexture(main->getIo()->getDataReadPath() + "skin/btn_play.png");
    btn_play->setSize(32, 32);
    btn_play->setPosition(buttons.at((size_t) ButtonID::Pause)->getPosition().x, getSize().y / 2);
    btn_play->setOrigin(Origin::Center);
    btn_play->setVisibility(Visibility::Hidden);
    add(btn_play);


    add(new TweenPosition({getPosition().x, getPosition().y},
                          {getPosition().x, getPosition().y - OSD_HEIGHT}, 0.5f));

    clock.restart();
}

void PlayerOSD::setVisibility(c2d::Visibility visibility, bool tweenPlay) {

    clock.restart();
    C2DObject::setVisibility(visibility, tweenPlay);
}

void PlayerOSD::onDraw(c2d::Transform &transform) {

    if (clock.getElapsedTime().asSeconds() >= OSD_HIDE_TIME) {
        setVisibility(Visibility::Hidden);
    }

    if (main->getPlayer()->isPlaying()) {
        position = (float) Kit_GetPlayerPosition(main->getPlayer()->getKitPlayer());
        duration = (float) Kit_GetPlayerDuration(main->getPlayer()->getKitPlayer());
        progress->setProgress(position / duration);
    }

    C2DObject::onDraw(transform);
}

bool PlayerOSD::onInput(c2d::Input::Player *players) {

    unsigned int keys = players[0].keys;

    if ((keys & Input::Key::Up) || keys & Input::Key::Fire2) {
        setVisibility(Visibility::Hidden, true);
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
                main->getPlayer()->pause();
            } else {
                main->getPlayer()->resume();
            }
        } else if (index == (int) ButtonID::SeekForward1) {
            Kit_PlayerSeek(main->getPlayer()->getKitPlayer(), position + 60.0);
        } else if (index == (int) ButtonID::SeekForward10) {
            Kit_PlayerSeek(main->getPlayer()->getKitPlayer(), position + (60.0 * 10.0));
        } else if (index == (int) ButtonID::SeekBackward1) {
            Kit_PlayerSeek(main->getPlayer()->getKitPlayer(), position - 60.0);
        } else if (index == (int) ButtonID::SeekBackward10) {
            Kit_PlayerSeek(main->getPlayer()->getKitPlayer(), position - (60.0 * 10.0));
        } else if (index == (int) ButtonID::Stop) {
            setVisibility(Visibility::Hidden);
            main->getPlayer()->stop();
            main->getPlayer()->setFullscreen(false);
        }

        clock.restart();
    }

    return true;
}
