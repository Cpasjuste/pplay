//
// Created by cpasjuste on 03/10/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

//#include "libmpv/client.h"
//#include "libmpv/render_gl.h"
//#include "libmpv/opengl_cb.h"

#include "menus/menu_video_submenu.h"
#include "video_texture.h"
#include "subtitles_texture.h"
#include "media_config.h"

#define MAX_STREAM_LIST_SIZE 32

class Main;

class PlayerOSD;

class Player : public c2d::Rectangle {

public:

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

    enum class CpuClock {
        Min = 0,
        Max = 1
    };

    explicit Player(Main *main);

    bool load(const MediaFile &file);

    void pause();

    void resume();

    void stop();

    int seek(double position);

    bool isPlaying();

    bool isPaused();

    bool isStopped();

    bool isLoading();

    bool isSubtitlesEnabled();

    bool isFullscreen();

    void setFullscreen(bool maximize);

    void setVideoStream(int streamId);

    void setAudioStream(int streamId);

    void setSubtitleStream(int streamId);

    void setCpuClock(const CpuClock &clock);

    Main *getMain();

    PlayerOSD *getOSD();

    //Kit_Player *getKitPlayer();

    MenuVideoSubmenu *getMenuVideoStreams();

    MenuVideoSubmenu *getMenuAudioStreams();

    MenuVideoSubmenu *getMenuSubtitlesStreams();

    Stream *getVideoStreams();

    Stream *getAudioStreams();

    Stream *getSubtitlesStreams();

    const std::string &getTitle() const;

    bool onInput(c2d::Input::Player *players) override;

private:

    void onDraw(c2d::Transform &transform, bool draw = true) override;

    void play();

    // ui
    Main *main = nullptr;
    PlayerOSD *osd = nullptr;
    c2d::TweenPosition *tweenPosition = nullptr;
    c2d::TweenScale *tweenScale = nullptr;
    MenuVideoSubmenu *menuVideoStreams = nullptr;
    MenuVideoSubmenu *menuAudioStreams = nullptr;
    MenuVideoSubmenu *menuSubtitlesStreams = nullptr;
    std::string title;

    // player
    VideoTexture *texture = nullptr;
    SubtitlesTexture *textureSub = nullptr;
    Stream video_streams;
    Stream audio_streams;
    Stream subtitles_streams;

    //Kit_Source *source = nullptr;
    //Kit_Player *kit_player = nullptr;
    //Kit_PlayerInfo playerInfo;

    // audio
    //c2d::C2DAudio *audio = nullptr;

    //mpv_handle *mpv_handle;

    MediaConfig *config = nullptr;

    bool show_subtitles = false;
    bool fullscreen = false;
    bool stopped = true;
    bool loading = false;
};

#endif //PPLAY_PLAYER_H
