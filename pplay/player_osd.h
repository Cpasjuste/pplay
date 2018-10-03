//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_OSD_H
#define PPLAY_PLAYER_OSD_H

class PlayerOSD : public c2d::Rectangle {

public:
    PlayerOSD(Player *player);

    ~PlayerOSD();

    void setProgress(float duration, float position);

private:
    Player *player = nullptr;
    c2d::TweenAlpha *tweenAlpha = nullptr;
    c2d::Rectangle *status = nullptr;
    c2d::Progress *progress = nullptr;
    c2d::Text *remaining = nullptr;
};

#endif //PPLAY_PLAYER_OSD_H
