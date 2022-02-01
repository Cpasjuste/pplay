//
// Created by cpasjuste on 09/12/18.
//

#ifndef PPLAY_VIDEO_TEXTURE_H
#define PPLAY_VIDEO_TEXTURE_H

#include "player.h"
#include "gradient_rectangle.h"

class VideoTexture : public c2d::GLTextureBuffer {

public:

    explicit VideoTexture(Main *main, const c2d::Vector2f &size);

    int resize(const c2d::Vector2i &size, bool keepPixels = false) override;

    void showFade();

    void hideFade();

private:

    void onDraw(c2d::Transform &transform, bool draw = true) override;

    Main *main;
    c2d::Texture *fade;
    c2d::TweenAlpha *fadeTween;
};

#endif //PPLAY_VIDEO_TEXTURE_H
