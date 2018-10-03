//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

#include "kitchensink/kitchensink.h"

#define AUDIO_BUFFER_SIZE (1024 * 64)

class Main;

class Player : public c2d::Rectangle {

public:

    Player(Main *main);

    ~Player();

    bool load(const c2d::Io::File &file);

    void run();

    void pause();

    void resume();

    void stop();

private:

    Main *main = nullptr;
    c2d::Texture *texture = nullptr;

    Kit_Source *source = nullptr;
    Kit_Player *player = nullptr;
    Kit_PlayerInfo playerInfo;
    // audio
    SDL_AudioDeviceID audioDeviceID;
    char audioBuffer[AUDIO_BUFFER_SIZE];
};

#endif //PPLAY_PLAYER_H
