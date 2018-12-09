//
// Created by cpasjuste on 09/12/18.
//

#ifndef PPLAY_VIDEO_TEXTURE_H
#define PPLAY_VIDEO_TEXTURE_H

#include "gradient_rectangle.h"

class VideoTexture : public c2d::C2DTexture {

public:

    explicit VideoTexture(const c2d::Vector2f &size);

private:

    GradientRectangle *rectLeft;
    GradientRectangle *rectBottom;
};


#endif //PPLAY_VIDEO_TEXTURE_H
