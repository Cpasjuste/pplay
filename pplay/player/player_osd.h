//
// Created by cpasjuste on 12/12/18.
//

#ifndef PPLAY_PLAYER_OSD_H
#define PPLAY_PLAYER_OSD_H

#include "cross2d/skeleton/sfml/Rectangle.hpp"

class Main;

class PlayerOSD : public c2d::Rectangle {

public:

    enum class ButtonID {
        Pause,
        SeekBackward10,
        SeekBackward1,
        SeekForward1,
        SeekForward10,
        Stop
    };

    explicit PlayerOSD(Main *main);

    void reset();

    void setVisibility(c2d::Visibility visibility, bool tweenPlay = true) override;

private:

    void onDraw(c2d::Transform &transform, bool draw = true) override;

    bool onInput(c2d::Input::Player *players) override;

    Main *main;
    Highlight *highlight = nullptr;
    c2d::Progress *progress = nullptr;
    c2d::Text *title = nullptr;
    c2d::Text *progress_text = nullptr;
    c2d::Text *duration_text = nullptr;
    c2d::Texture *btn_play = nullptr;
    std::vector<c2d::Texture *> buttons;
    float position = 0;
    float duration = 0;
    int index = 0;

    c2d::C2DClock clock;
};

#endif //PPLAY_PLAYER_OSD_H
