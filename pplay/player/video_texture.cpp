//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "video_texture.h"

using namespace c2d;

VideoTexture::VideoTexture(const c2d::Vector2f &size) : C2DTexture(size, Format::RGBA8) {

    rectLeft = new GradientRectangle({250, size.y / 2, 500, size.y});
    rectLeft->setOrigin(Origin::Center);
    rectLeft->setRotation(180);
    rectLeft->setColor(Color::Black, Color::Transparent);
    add(rectLeft);

    rectBottom = new GradientRectangle({size.x / 2, size.y - 125, 250, size.x});
    rectBottom->setOrigin(Origin::Center);
    rectBottom->setRotation(90);
    rectBottom->setColor(Color::Black, Color::Transparent);
    add(rectBottom);
}
