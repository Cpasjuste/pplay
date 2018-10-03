//
// Created by cpasjuste on 03/10/18.
//

#include "main.h"
#include "utility.h"

using namespace c2d;

PlayerOSD::PlayerOSD(Player *p) : Rectangle(p->getSize()) {

    player = p;
    setFillColor(Color::Transparent);

    status = new RoundedRectangle({getSize().x - 256, 96});
    status->setPosition(128, getSize().y - 128);
    Color color = COLOR_GRAY_DARK;
    color.a = 200;
    status->setFillColor(color);
    status->setOutlineColor(COLOR_BLUE);
    status->setOutlineThickness(2);
    add(status);

    progress = new Progress({status->getSize().x / 2.0f, status->getSize().y / 2.0f,
                             status->getSize().x - 256, 16});
    progress->setOriginCenter();
    progress->setFgColor(COLOR_BLUE);
    progress->setBgColor(COLOR_GRAY);
    progress->setOutlineThickness(1);
    progress->setOutlineColor(COLOR_GRAY_LIGHT);
    status->add(progress);

    remaining = new Text("01:23:45", *player->getMain()->getFont(), 22);
    remaining->setOrigin(remaining->getLocalBounds().width, 22 / 2);
    remaining->setPosition(
            status->getSize().x - 20,
            status->getSize().y / 2.0f - 2);
    remaining->setOutlineThickness(1);
    status->add(remaining);

    // TODO: tween alpha not working properly ?
    //tweenAlpha = new TweenAlpha(0, 230, 5);
    //statusRect->add(tweenAlpha);

    setVisibility(Visibility::Hidden, false);
}

void PlayerOSD::setProgress(float duration, float position) {

    std::string remain = Utils::formatTime(duration - position);
    remaining->setString(remain);
    remaining->setOrigin(remaining->getLocalBounds().width, 22 / 2);
    progress->setProgress(position / duration);
}

PlayerOSD::~PlayerOSD() {

}
