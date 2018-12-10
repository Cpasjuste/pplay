//
// Created by cpasjuste on 09/12/18.
//

#ifndef PPLAY_GRADIENTRECTANGLE_H
#define PPLAY_GRADIENTRECTANGLE_H

class GradientRectangle : public c2d::Sprite {

public:

    explicit GradientRectangle(const c2d::FloatRect &rect);

    void setColor(const c2d::Color &color1, const c2d::Color &color2);

    void setAlpha(uint8_t alpha, bool recursive = false) override;
};

#endif //PPLAY_GRADIENTRECTANGLE_H
