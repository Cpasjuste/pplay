//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

#include "../mpv/libmpv/client.h"
#include "../mpv/libmpv/render_gl.h"

#include "menus/menu_video_submenu.h"
#include "media_config.h"
#include "media_file.h"

class Main;

class PlayerOSD;

class VideoTexture;

class Player : public c2d::Rectangle {

public:

    struct Mpv {
        mpv_handle *handle = nullptr;
        mpv_render_context *ctx = nullptr;
        bool available = false;
    };

    /*
    class Stream {
    public:
        Stream() {
            reset();
        }

        void reset() {
            for (int &stream : streams) {
                stream = -1;
            }
            size = 0;
            current = 0;
        }

        void setCurrent(int streamId) {
            for (int i = 0; i < size; i++) {
                if (streamId == streams[i]) {
                    current = i;
                }
            }
        }

        int getCurrent() {
            if (current > -1 && current < size) {
                return streams[current];
            }
            return -1;
        }

        int streams[MAX_STREAM_LIST_SIZE]{};
        int size = 0;
        int current = 0;
    };
    */

    enum class CpuClock {
        Min = 0,
        Max = 1
    };

    explicit Player(Main *main);

    ~Player();

    bool load(const MediaFile &file);

    void pause();

    void resume();

    void stop();

    int seek(double position);

    bool isPaused();

    bool isStopped();

    bool isFullscreen();

    void setFullscreen(bool maximize, bool hide = false);

    void setVideoStream(int streamId);

    void setAudioStream(int streamId);

    void setSubtitleStream(int streamId);

    int getVideoStream();

    int getAudioStream();

    int getSubtitleStream();

    void setCpuClock(const CpuClock &clock);

    long getPlaybackDuration();

    long getPlaybackPosition();

    PlayerOSD *getOSD();

    MenuVideoSubmenu *getMenuVideoStreams();

    MenuVideoSubmenu *getMenuAudioStreams();

    MenuVideoSubmenu *getMenuSubtitlesStreams();

    const std::string &getTitle() const;

    bool onInput(c2d::Input::Player *players) override;

private:

    void onUpdate() override;

    void onLoadEvent();

    void onStopEvent();

    // ui
    Main *main = nullptr;
    PlayerOSD *osd = nullptr;
    c2d::TweenPosition *tweenPosition = nullptr;
    c2d::TweenScale *tweenScale = nullptr;
    MenuVideoSubmenu *menuVideoStreams = nullptr;
    MenuVideoSubmenu *menuAudioStreams = nullptr;
    MenuVideoSubmenu *menuSubtitlesStreams = nullptr;
    MediaFile file;

    // player
    VideoTexture *texture = nullptr;
    MediaConfig *config = nullptr;
    Mpv mpv;

    bool fullscreen = false;
};

#endif //PPLAY_PLAYER_H
