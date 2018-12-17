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
