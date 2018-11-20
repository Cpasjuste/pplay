//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

#include "kitchensink/kitchensink.h"

#define AUDIO_BUFFER_SIZE (1024 * 128)
#define MAX_STREAM_LIST_SIZE 32

class Main;

class PlayerOSD;

class Player : public OutlineRect {

public:

    class Stream {
    public:
        int streams[MAX_STREAM_LIST_SIZE]{};
        int size = 0;
        int current = 0;
    };

    explicit Player(Main *main);

    ~Player();

    bool load(c2d::Io::File *file);

    void step(unsigned int keys);

    void pause();

    void resume();

    void stop();

    bool isPlaying();

    bool isFullscreen();

    void setFullscreen(bool maximize);

    Main *getMain();

    c2d::TweenPosition *getTweenPosition();

    c2d::TweenScale *getTweenScale();

private:

    Main *main = nullptr;
    PlayerOSD *osd = nullptr;
    c2d::Texture *texture = nullptr;
    c2d::TweenPosition *tweenPosition;
    c2d::TweenScale *tweenScale;

    // kit player
    Kit_Source *source = nullptr;
    Kit_Player *player = nullptr;
    Kit_PlayerInfo playerInfo;
    Stream video_streams;
    Stream audio_streams;
    Stream subtitles_streams;

    // audio
    SDL_AudioDeviceID audioDeviceID;
    char audioBuffer[AUDIO_BUFFER_SIZE];

    bool fullscreen = false;
    bool paused = false;
};

#endif //PPLAY_PLAYER_H
