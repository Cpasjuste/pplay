//
// Created by cpasjuste on 08/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "status_box.h"
#include "player_osd.h"
#include "utility.h"

using namespace c2d;

StatusBox::StatusBox(Main *m, const c2d::Vector2f &position)
        : Rectangle(m->getScaled({64, 48})) {

    main = m;
    pos = m->getScaled(position);
    StatusBox::setPosition(pos);

    icon = new C2DTexture(main->getIo()->getRomFsPath() + "skin/wait.png");
    icon->setOrigin(Origin::Center);
    icon->setPosition(icon->getSize().x / 2 + (8 * m->getScaling().x), StatusBox::getSize().y / 2);
    icon->setScale(main->getScaling());
    icon->setFillColor(COLOR_FONT);
    icon->setAlpha(200);
    icon->add(new TweenRotation(360, 0, 2, TweenLoop::Loop, TweenState::Playing));
    StatusBox::add(icon);

    titleText = new Text("Please Wait...", main->getFontSize(Main::FontSize::Medium), main->getFont());
    titleText->setFillColor(COLOR_RED);
    titleText->setPosition(pplay::Utility::ceil(icon->getSize().x + (16 * m->getScaling().x), (4 * m->getScaling().y)));
    StatusBox::add(titleText);

    messageText = new Text("Doing something in background, please wait",
                           main->getFontSize(Main::FontSize::Small), main->getFont());
    messageText->setFillColor(COLOR_FONT);
    messageText->setPosition(pplay::Utility::ceil(icon->getSize().x + (16 * m->getScaling().x),
                                                  titleText->getPosition().y
                                                  + (float) main->getFontSize(Main::FontSize::Medium) +
                                                  (4 * m->getScaling().y)));
    messageText->setSizeMax(StatusBox::getSize().x - icon->getSize().x - (16 * m->getScaling().x), 0);
    StatusBox::add(messageText);

    clock = new C2DClock();
    mutex = SDL_CreateMutex();

    tween = new TweenAlpha(0, 255, 0.5f);
    StatusBox::add(tween);
    StatusBox::setVisibility(Visibility::Hidden);
}

StatusBox::~StatusBox() {
    delete (clock);
    SDL_DestroyMutex(mutex);
}

void StatusBox::show(const std::string &title, const std::string &message, bool inf, bool drawNow) {

    SDL_LockMutex(mutex);
    titleText->setString(title);
    messageText->setString(message);
    SDL_UnlockMutex(mutex);

    infinite = inf;
    clock->restart();
    setVisibility(Visibility::Visible, true);
    if (drawNow) {
        for (int i = 0; i < 30; i++) {
            main->flip();
        }
    }
}

void StatusBox::hide() {
    clock->restart();
    infinite = false;
}

void StatusBox::onDraw(c2d::Transform &transform, bool draw) {

    if (isVisible() && !infinite && clock->getElapsedTime().asSeconds() > 2) {
        setVisibility(Visibility::Hidden, true);
    }

    PlayerOSD *osd = main->getPlayer()->getOSD();
    if (osd && osd->isVisible()) {
        FloatRect bounds = main->getPlayer()->getOSD()->getGlobalBounds();
        setPosition(pos.x, bounds.top - (32 * main->getScaling().y));
    } else {
        FloatRect bounds = main->getMenuMain()->getGlobalBounds();
        setPosition(bounds.left + bounds.width + pos.x, main->getSize().y - (16 * main->getScaling().x));
    }

    SDL_LockMutex(mutex);
    C2DObject::onDraw(transform, draw);
    SDL_UnlockMutex(mutex);
}

void StatusBox::setAlpha(uint8_t alpha, bool recursive) {
    C2DObject::setAlpha(alpha, recursive);
}
