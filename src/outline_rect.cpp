//
// Created by cpasjuste on 17/10/18.
//

#include "cross2d/c2d.h"
#include "outline_rect.h"

using namespace c2d;

OutlineRect::OutlineRect(const c2d::Vector2f &size, float outlineSize) : RectangleShape(size) {

    outline = new C2DRectangle(size);
    outline->setFillColor(Color::Transparent);
    outline->setOutlineColor(Color::Black);
    outline->setOutlineThickness(outlineSize);
    updateOutline();
    add(outline);
}

OutlineRect::OutlineRect(const c2d::FloatRect &rect, float outlineSize) : C2DRectangle(rect) {

    outline = new C2DRectangle(rect);
    outline->setFillColor(Color::Transparent);
    outline->setOutlineColor(Color::Black);
    outline->setOutlineThickness(outlineSize);
    updateOutline();
    add(outline);
}

void OutlineRect::setOutlineThickness(float thickness) {
    Shape::setOutlineThickness(thickness);
    updateOutline();
}

void OutlineRect::setSize(const c2d::Vector2f &size) {
    RectangleShape::setSize(size);
    updateOutline();
}

void OutlineRect::setSize(float width, float height) {
    RectangleShape::setSize(width, height);
    updateOutline();
}

void OutlineRect::updateOutline() {
    outline->setPosition(
            getPosition().x - getOutlineThickness(),
            getPosition().y - getOutlineThickness());
    outline->setSize(
            getSize().x + getOutlineThickness() * 2,
            getSize().y + getOutlineThickness() * 2);
}
