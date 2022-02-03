//
// Created by cpasjuste on 09/01/19.
//

#ifndef PPLAY_STATUSBAR_H
#define PPLAY_STATUSBAR_H

class Main;

class Battery;

class StatusBar : public c2d::GradientRectangle {

public:

    explicit StatusBar(Main *main);

    ~StatusBar() override;

    void onUpdate() override;

    Battery *battery = nullptr;
    c2d::Text *timeText = nullptr;
};

#endif //PPLAY_STATUSBAR_H
