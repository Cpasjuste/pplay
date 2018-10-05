//
// Created by cpasjuste on 03/10/18.
//

#include "main.h"
#include "utility.h"
#include "player_osd.h"


using namespace c2d;

PlayerOSD::PlayerOSD(Player *p) : C2DRectangle(p->getSize()) {

    player = p;
    setFillColor(Color::Transparent);

    status = new C2DRoundedRectangle({getSize().x - 256, 96});
    status->setPosition(128, getSize().y - 128);
    Color color = COLOR_GRAY_DARK;
    color.a = 200;
    status->setFillColor(color);
    status->setOutlineColor(COLOR_BLUE);
    status->setOutlineThickness(2);
    add(status);

    progress = new Progress({status->getSize().x / 2.0f, status->getSize().y / 2.0f,
                             status->getSize().x - 256, 16});
    progress->setOrigin(Origin::Center);
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

    tweenAlpha = new TweenAlpha(0, 200, 0.6f);
    status->add(tweenAlpha);

    setVisibility(Visibility::Hidden, false);
}

void PlayerOSD::setProgress(float duration, float position) {

    std::string remain = Utils::formatTime(duration - position);
    remaining->setString(remain);
    progress->setProgress(position / duration);
}

bool PlayerOSD::isVisible() {
    return status->getVisibility() == Visibility::Visible;
}

c2d::Visibility PlayerOSD::getVisibility() {
    return status->getVisibility();
}

void PlayerOSD::setVisibility(c2d::Visibility visibility, bool tweenPlay) {
    status->setVisibility(visibility, tweenPlay);
}

PlayerOSD::~PlayerOSD() {

}
