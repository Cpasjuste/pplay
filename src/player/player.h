//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

#include "menus/menu_video_submenu.h"
#include "media_file.h"
#include "mpv.h"
#include "subtitles.h"

class Main;


class PlayerOSD;

class VideoTexture;

class Player : public c2d::Rectangle {

public:

    explicit Player(Main *main);

    ~Player() override;

    bool load(const MediaFile &file);

    void pause();

    void resume();

    void stop();

    void setSpeed(double speed);

    bool isFullscreen();

    void setFullscreen(bool maximize, bool hide = false);

    void setVideoStream(int streamId);

    void setAudioStream(int streamId);

    void setSubtitleStream(int streamId);

    int getVideoStream();

    int getAudioStream();

    int getSubtitleStream();

    PlayerOSD *getOSD();

    Mpv *getMpv();

    MenuVideoSubmenu *getMenuVideoStreams();

    MenuVideoSubmenu *getMenuAudioStreams();

    MenuVideoSubmenu *getMenuSubtitlesStreams();

    const std::string &getTitle() const;

    bool onInput(c2d::Input::Player *players) override;

private:

    void onUpdate() override;

    void onLoadEvent();

    // image subtitles (PGS, VobSub...) are drawn by the CPU, see Mpv::setBitmapSubtitleRendering
    void updateBitmapSubtitles();

    void onStopEvent(int reason);

    int bitmapSubTicks = 0;

    // hardware decoding (see ffmpeg-orbis/): config + crash guard
    std::string resolveHwdec();

    void armHwdecGuard();

    void releaseHwdecGuard();

    void updateHwdecGuard();

    // ui
    Main *main = nullptr;
    PlayerOSD *osd = nullptr;
    c2d::TweenScale *tweenScale = nullptr;
    c2d::TweenPosition *tweenPosition = nullptr;
    MenuVideoSubmenu *menuVideoStreams = nullptr;
    MenuVideoSubmenu *menuAudioStreams = nullptr;
    MenuVideoSubmenu *menuSubtitlesStreams = nullptr;
    MediaFile file;
    std::vector<pplay::Subtitles::Item> externalSubs;

    // player
    VideoTexture *texture = nullptr;
    Mpv *mpv;

    bool fullscreen = false;

    // hardware decoding state
    std::string hwdecMethod;          // method given to mpv ("" = mpv default)
    std::string hwdecGuardFile;       // exists while hw decoding has not proven itself
    bool hwdecGuardArmed = false;
    bool hwdecNotice = false;         // show "disabled after crash" once
};

#endif //PPLAY_PLAYER_H
