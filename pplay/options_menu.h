//
// Created by cpasjuste on 20/10/18.
//

#ifndef PPLAY_OPTION_MENU_H
#define PPLAY_OPTION_MENU_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"

#define OPT_COUNT 3

class Highlight;

class OptionMenu : public c2d::RectangleShape {

public:

    enum Index {
        Home = 0,
        Network = 1,
        Exit = 2
    };

    explicit OptionMenu(Main *main, const c2d::FloatRect &rect);

private:

    Main *main;
    c2d::Text *title;
    c2d::Text *options[OPT_COUNT];
    Highlight *highlight;
    c2d::TweenPosition *tween;
    int index = 0;

    void onInput(c2d::Input::Player *players) override;
};

#endif //PPLAY_OPTION_MENU_H
