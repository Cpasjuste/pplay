//
// Created by cpasjuste on 09/01/19.
//

#ifndef PPLAY_STATUSBAR_H
#define PPLAY_STATUSBAR_H

#include "gradient_rectangle.h"

class Main;

class Battery;

class StatusBar : public GradientRectangle {

public:

    explicit StatusBar(Main *main);

    ~StatusBar() override;

    void onDraw(c2d::Transform &transform) override;

    Battery *battery = nullptr;
    c2d::Text *timeText = nullptr;
};

#endif //PPLAY_STATUSBAR_H
