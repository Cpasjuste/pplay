//
// Created by cpasjuste on 04/12/18.
//

#ifndef PPLAY_HIGHLIGHT_H
#define PPLAY_HIGHLIGHT_H

#include "gradient_rectangle.h"

class Highlight : public c2d::Rectangle {

public:

    enum class CursorPosition {
        Left,
        Right
    };

    explicit Highlight(const c2d::Vector2f &size, const CursorPosition &cursor = CursorPosition::Right);

    void setAlpha(uint8_t alpha, bool recursive = false) override;

    void setFillColor(const c2d::Color &color) override;

    void setCursorColor(const c2d::Color &color);

    void tweenTo(const c2d::Vector2f &position);

private:

    GradientRectangle *gradientRectangle;
    c2d::RectangleShape *cursor;
    c2d::TweenPosition *tween;

};

#endif //PPLAY_HIGHLIGHT_H
