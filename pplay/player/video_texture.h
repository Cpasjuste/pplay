//
// Created by cpasjuste on 09/12/18.
//

#ifndef PPLAY_VIDEO_TEXTURE_H
#define PPLAY_VIDEO_TEXTURE_H

#include "player.h"
#include "gradient_rectangle.h"

class VideoTexture : public c2d::GLTextureBuffer {

public:

    explicit VideoTexture(const c2d::Vector2f &size, Player::Mpv *mpv);

    void showGradients();

    void hideGradients();

private:

    void onDraw(c2d::Transform &transform, bool draw = true) override;

    GradientRectangle *rectLeft;
    GradientRectangle *rectBottom;

    c2d::TweenAlpha *rectLeftTween;
    c2d::TweenAlpha *rectBottomTween;

    Player::Mpv *mpv;
};

#endif //PPLAY_VIDEO_TEXTURE_H
