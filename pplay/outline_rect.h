//
// Created by cpasjuste on 17/10/18.
//

#ifndef PPLAY_OUTLINE_RECT_H
#define PPLAY_OUTLINE_RECT_H

#include "cross2d/skeleton/sfml/Rect.hpp"

class OutlineRect : public c2d::C2DRectangle {

public:
    explicit OutlineRect(const c2d::Vector2f &size, float outlineSize = 1);

    explicit OutlineRect(const c2d::FloatRect &rect, float outlineSize = 1);

    void setOutlineThickness(float thickness) override;

    void setSize(const c2d::Vector2f &size) override;

    void setSize(float width, float height) override;

private:
    void updateOutline();

    c2d::C2DRectangle *outline;
};

#endif //PPLAY_OUTLINE_RECT_H
