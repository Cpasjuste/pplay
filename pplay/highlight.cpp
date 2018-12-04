//
// Created by cpasjuste on 04/12/18.
//

#include "cross2d/c2d.h"

#include "main.h"
#include "highlight.h"

using namespace c2d;

Highlight::Highlight(const c2d::Vector2f &size, const CursorPosition &pos) : RectangleShape(size) {

    Shape::setFillColor(Color::Transparent);

    texture = new C2DTexture(c2d_renderer->getIo()->getDataPath() + "/skin/gradient_white.png");
    texture->setSize(size);
    texture->setFillColor(COLOR_HIGHLIGHT);
    if (pos == CursorPosition::Right) {
        texture->setOrigin(Origin::BottomRight);
        texture->setRotation(180);
    }
    add(texture);

    cursor = new RectangleShape(Vector2f{6, size.y});
    cursor->setFillColor(COLOR_BLUE);
    if (pos == CursorPosition::Right) {
        cursor->move(size.x - 4, 0);
    }
    add(cursor);

    tweenPosition = new TweenPosition({0, 0}, {0, 0}, 0.5f);
    add(tweenPosition);

    tweenScale = new TweenScale({1, 1}, {1, 1}, 0.5f);
    add(tweenScale);
}

void Highlight::setFillColor(const c2d::Color &color) {
    texture->setFillColor(color);
}

void Highlight::setCursorColor(const c2d::Color &color) {
    cursor->setFillColor(color);
}
