//
// Created by cpasjuste on 03/10/18.
//

#include "main.h"
#include "utility.h"
#include "player_osd.h"

#define OSD_STATUS_HEIGHT   64.0f
#define OSD_HIDE_TIME       4.0f

using namespace c2d;

PlayerOSD::PlayerOSD(Player *p) : C2DRectangle(p->getSize()) {

    player = p;
    setFillColor(Color::Transparent);

    std::string path = p->getMain()->getIo()->getDataPath() + "skin/play-button.png";
    button_play = new C2DTexture(path);
    auto size = button_play->getTextureRect();
    button_play->setFillColor(COLOR_BLUE);
    button_play->setOrigin(Origin::Left);
    button_play->setPosition(16, OSD_STATUS_HEIGHT / 2);
    button_play->setScale((OSD_STATUS_HEIGHT * 0.6f) / (float) size.width,
                          (OSD_STATUS_HEIGHT * 0.6f) / (float) size.height);
    button_play->setVisibility(Visibility::Hidden);

    path = p->getMain()->getIo()->getDataPath() + "skin/pause-button.png";
    button_pause = new C2DTexture(path);
    button_pause->setFillColor(COLOR_BLUE);
    button_pause->setOrigin(Origin::Left);
    button_pause->setPosition(16, OSD_STATUS_HEIGHT / 2);
    button_pause->setScale((OSD_STATUS_HEIGHT * 0.6f) / (float) size.width,
                           (OSD_STATUS_HEIGHT * 0.6f) / (float) size.height);

    status = new C2DRoundedRectangle({getSize().x - 384, OSD_STATUS_HEIGHT});
    status->setOrigin(Origin::Bottom);
    status->setPosition(getSize().x / 2, getSize().y - 32);
    status->setFillColor(COLOR_GRAY_DARK);
    status->setOutlineColor(COLOR_BLUE);
    status->setOutlineThickness(2);

    status->add(button_play);
    status->add(button_pause);

    add(status);

    float x = button_play->getGlobalBounds().left + button_play->getGlobalBounds().width + 16;
    progress = new Progress({x, status->getSize().y / 2.0f, status->getSize().x - 196, 16});
    progress->setOrigin(Origin::Left);
    progress->setFgColor(COLOR_BLUE);
    progress->setBgColor(COLOR_GRAY);
    progress->setOutlineThickness(1);
    progress->setOutlineColor(COLOR_GRAY_LIGHT);
    status->add(progress);

    remaining = new Text("01:23:45", *player->getMain()->getFont(), 22);
    remaining->setOrigin(Origin::Right);
    remaining->setPosition(
            status->getSize().x - 20,
            status->getSize().y / 2.0f - 2);
    remaining->setOutlineThickness(1);
    status->add(remaining);

    current = new Text("01:23:45", *player->getMain()->getFont(), 18);
    current->setOrigin(Origin::Bottom);
    current->setOutlineThickness(1);
    status->add(current);

    tweenAlpha = new TweenAlpha(0, 200, 0.6f);
    status->add(tweenAlpha);

    clock.restart();
    setVisibility(Visibility::Hidden, false);
}

void PlayerOSD::setProgress(float duration, float position) {

    if (clock.getElapsedTime().asSeconds() >= OSD_HIDE_TIME) {
        setVisibility(Visibility::Hidden);
    } else {
        std::string rem = Utils::formatTime(duration - position);
        remaining->setString(rem);

        std::string cur = Utils::formatTime(position);
        current->setString(cur);
        current->setPosition(
                progress->getPosition().x + progress->getProgressWidth(),
                progress->getPosition().y - 12);

        progress->setProgress(position / duration);
    }
}

void PlayerOSD::pause() {
    clock.restart();
    button_pause->setVisibility(Visibility::Hidden);
    button_play->setVisibility(Visibility::Visible);
}

void PlayerOSD::resume() {
    clock.restart();
    button_pause->setVisibility(Visibility::Visible);
    button_play->setVisibility(Visibility::Hidden);
}

bool PlayerOSD::isVisible() {
    return status->getVisibility() == Visibility::Visible;
}

c2d::Visibility PlayerOSD::getVisibility() {
    return status->getVisibility();
}

void PlayerOSD::setVisibility(c2d::Visibility visibility, bool tweenPlay) {

    clock.restart();
    if (visibility == status->getVisibility()) {
        return;
    }

    status->setVisibility(visibility, tweenPlay);
}

PlayerOSD::~PlayerOSD() {

}
