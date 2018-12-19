//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "video_texture.h"

using namespace c2d;

VideoTexture::VideoTexture(const c2d::Vector2f &size) : C2DTexture(size, Format::RGBA8) {

    float l_width = getSize().x / 5;
    rectLeft = new GradientRectangle({(l_width / 2) - 2, size.y / 2, l_width, size.y});
    rectLeft->setOrigin(Origin::Center);
    rectLeft->setRotation(180);
    rectLeft->setColor(Color::Black, Color::Transparent);
    rectLeft->setAlpha(0);
    rectLeftTween = new TweenAlpha(0, 255, 0.5f);
    rectLeft->add(rectLeftTween);
    add(rectLeft);

    float b_height = getSize().y / 5;
    rectBottom = new GradientRectangle({size.x / 2, (size.y - (b_height / 2)) + 2, b_height, size.x});
    rectBottom->setOrigin(Origin::Center);
    rectBottom->setRotation(90);
    rectBottom->setColor(Color::Black, Color::Transparent);
    rectBottom->setAlpha(0);
    rectBottomTween = new TweenAlpha(0, 255, 0.5f);
    rectBottom->add(rectBottomTween);
    add(rectBottom);

    void *buf;
    lock(nullptr, &buf, nullptr);
    memset(buf, 0, (size_t) (int) size.x * (int) size.y * 4);
    unlock();
}

void VideoTexture::hideGradients() {
    rectLeftTween->play(TweenDirection::Backward);
    rectBottomTween->play(TweenDirection::Backward);
}

void VideoTexture::showGradients() {
    rectLeftTween->play(TweenDirection::Forward);
    rectBottomTween->play(TweenDirection::Forward);
}
