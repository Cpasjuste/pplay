//
// Created by cpasjuste on 04/12/18.
//

#include "cross2d/c2d.h"

#include "main.h"
#include "highlight.h"

using namespace c2d;

Highlight::Highlight(const c2d::Vector2f &size, const CursorPosition &pos) : Rectangle(size) {

    gradientRectangle = new GradientRectangle({0, 0, size.x, size.y});
    gradientRectangle->setColor(COLOR_HIGHLIGHT, Color::Transparent);
    if (pos == CursorPosition::Left) {
        gradientRectangle->setOrigin(Origin::BottomRight);
        gradientRectangle->setRotation(180);
    }
    add(gradientRectangle);

    cursor = new RectangleShape(Vector2f{6, size.y});
    cursor->setFillColor(COLOR_BLUE);
    if (pos == CursorPosition::Right) {
        cursor->move(size.x - 4, 0);
    }
    add(cursor);

    tween = new TweenPosition(getPosition(), getPosition(), (float) INPUT_DELAY / 2);
    tween->setState(TweenState::Stopped);
    add(tween);
}

void Highlight::setAlpha(uint8_t alpha, bool recursive) {
    if (alpha <= COLOR_HIGHLIGHT.a) {
        gradientRectangle->setAlpha(alpha);
    }
    cursor->setAlpha(alpha);
}

void Highlight::setFillColor(const c2d::Color &color) {
    gradientRectangle->setColor(color, Color::Transparent);
}

void Highlight::setCursorColor(const c2d::Color &color) {
    cursor->setFillColor(color);
}

void Highlight::tweenTo(const c2d::Vector2f &position) {
    if (tween) {
        float seconds = (float) c2d_renderer->getInput()->getRepeatDelay() * 0.001f / 5;
        tween->setFromTo(getPosition(), position, seconds);
        tween->play(TweenDirection::Forward, true);
    } else {
        Transformable::setPosition(position);
    }
}
