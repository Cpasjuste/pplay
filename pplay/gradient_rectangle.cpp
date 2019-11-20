//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "gradient_rectangle.h"

using namespace c2d;

GradientRectangle::GradientRectangle(const FloatRect &rect) : Sprite() {
    setSize(rect.width, rect.height);
    setPosition(rect.left, rect.top);
}

void GradientRectangle::setColor(const Color &color1, const Color &color2) {
    auto *vertices = getVertexArray()->getVertices();
    vertices->at(0).color = color2;
    vertices->at(1).color = color2;
    vertices->at(2).color = color1;
    vertices->at(3).color = color1;
    getVertexArray()->update();
}

void GradientRectangle::setAlpha(uint8_t alpha, bool  /*recursive*/) {
    auto *vertices = getVertexArray()->getVertices();
    vertices->at(2).color.a = alpha;
    vertices->at(3).color.a = alpha;
    getVertexArray()->update();
}
