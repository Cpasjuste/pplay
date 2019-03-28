//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"
#include "player.h"
#include "player_osd.h"
#include "video_texture.h"
#include "utility.h"

using namespace c2d;

static void *get_proc_address_mpv(void *unused, const char *name) {
    return SDL_GL_GetProcAddress(name);
}

Player::Player(Main *_main) : Rectangle(_main->getSize()) {

    main = _main;

    setOrigin(Origin::TopRight);

    tweenPosition = new TweenPosition(
            {main->getSize().x - 32, 32},
            {main->getSize().x, 0}, 0.5f);
    add(tweenPosition);
    tweenScale = new TweenScale({0.6f, 0.6f}, {1.0f, 1.0f}, 0.5f);
    add(tweenScale);

    setVisibility(Visibility::Hidden);

    // MPV INIT
    mpv.handle = mpv_create();
    if (!mpv.handle) {
        printf("error: mpv_create\n");
        return;
    }

    std::string mpv_home = main->getIo()->getDataWritePath() + "mpv";
    printf("config: %s\n", mpv_home.c_str());
    mpv_set_option_string(mpv.handle, "config-dir", mpv_home.c_str());
    mpv_set_option_string(mpv.handle, "terminal", "yes");
    mpv_set_option_string(mpv.handle, "msg-level", "all=v");
    mpv_set_option_string(mpv.handle, "vd-lavc-threads", "4");
    mpv_set_option_string(mpv.handle, "vd-lavc-dr", "yes");
    mpv_set_option_string(mpv.handle, "vd-lavc-fast", "yes");
    mpv_set_option_string(mpv.handle, "audio-channels", "stereo");
    //mpv_set_option_string(mpv.handle, "vo", "gpu");

    int res = mpv_initialize(mpv.handle);
    if (res) {
        printf("error: mpv_initialize: %s\n", mpv_error_string(res));
        return;
    }

    mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr, nullptr};
    mpv_render_param params[]{
            {MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
            {MPV_RENDER_PARAM_INVALID,            nullptr}
    };

    if (mpv_render_context_create(&mpv.ctx, mpv.handle, params) < 0) {
        printf("error: mpv_render_context_create: %s\n", mpv_error_string(res));
        return;
    }

    mpv.available = true;
    // MPV INIT

    // TODO: create texture of video size?
    texture = new VideoTexture(main->getSize(), &mpv);
    add(texture);

    osd = new PlayerOSD(main);
    add(osd);
}

Player::~Player() {
    if (mpv.available) {
        mpv.available = false;
        mpv_render_context_free(mpv.ctx);
        mpv_terminate_destroy(mpv.handle);
    }
}

bool Player::load(const MediaFile &f) {

    file = f;
    std::string path = file.path;
#ifdef __SMB_SUPPORT__
    if (Utility::startWith(path, "smb://")) {
        std::replace(path.begin(), path.end(), '\\', '/');
    }
#endif
    printf("loadfile: %s\n", path.c_str());
    const char *cmd[] = {"loadfile", path.c_str(), "replace", "pause=yes,speed=1", nullptr};
    int res = mpv_command(mpv.handle, cmd);
    if (res != 0) {
        main->getStatus()->show("Error...", "Could not play file:\n" + std::string(mpv_error_string(res)));
        printf("Player::load: could not play file: %s\n", mpv_error_string(res));
        return false;
    }

    return true;
}

void Player::onLoadEvent() {

    std::vector<MediaInfo::Stream> streams;

    // load tracks
    mpv_node node;
    mpv_get_property(mpv.handle, "track-list", MPV_FORMAT_NODE, &node);
    if (node.format == MPV_FORMAT_NODE_ARRAY) {
        for (int i = 0; i < node.u.list->num; i++) {
            if (node.u.list->values[i].format == MPV_FORMAT_NODE_MAP) {
                MediaInfo::Stream stream{};
                for (int n = 0; n < node.u.list->values[i].u.list->num; n++) {
                    std::string key = node.u.list->values[i].u.list->keys[n];
                    if (key == "type") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.type = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "id") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.id = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "title") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.title = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "lang") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.language = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "decoder-desc") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            stream.codec = node.u.list->values[i].u.list->values[n].u.string;
                        }
                    } else if (key == "demux-w") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.width = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-h") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.height = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-samplerate") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.sample_rate = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    } else if (key == "demux-channel-count") {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_INT64) {
                            stream.channels = (int) node.u.list->values[i].u.list->values[n].u.int64;
                        }
                    }
                }
                streams.push_back(stream);
            }
        }
    }

    file.mediaInfo.videos.clear();
    file.mediaInfo.audios.clear();
    file.mediaInfo.subtitles.clear();
    for (auto &stream : streams) {
        if (stream.type == "video") {
            file.mediaInfo.videos.push_back(stream);
        } else if (stream.type == "audio") {
            file.mediaInfo.audios.push_back(stream);
        } else if (stream.type == "sub") {
            file.mediaInfo.subtitles.push_back(stream);
        }
    }

    // update filer item
    file.mediaInfo.duration = getPlaybackDuration();
    file.mediaInfo.save();
    main->getFiler()->setMediaInfo(file, file.mediaInfo);

    // create menus
    if (!file.mediaInfo.videos.empty()) {
        // videos menu options
        std::vector<MenuItem> items;
        for (auto &stream : file.mediaInfo.videos) {
            items.emplace_back(stream.title + "\n" + stream.language + " " + stream.codec + " "
                               + std::to_string(stream.width) + "x" + std::to_string(stream.height),
                               "", MenuItem::Position::Top, stream.id);
        }
        menuVideoStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "VIDEO", items, MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setSelection(MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setVisibility(Visibility::Hidden, false);
        menuVideoStreams->setLayer(3);
        add(menuVideoStreams);
    }
    if (!file.mediaInfo.audios.empty()) {
        // audios menu options
        std::vector<MenuItem> items;
        for (auto &stream : file.mediaInfo.audios) {
            items.emplace_back(stream.title + "\n" + stream.language + " "
                               + stream.codec + " " + std::to_string(stream.channels) + "ch "
                               + std::to_string(stream.sample_rate / 1000) + " Khz",
                               "", MenuItem::Position::Top, stream.id);
        }
        menuAudioStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "AUDIO", items, MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setSelection(MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setVisibility(Visibility::Hidden, false);
        menuAudioStreams->setLayer(3);
        add(menuAudioStreams);
    }
    if (!file.mediaInfo.subtitles.empty()) {
        // subtitles menu options
        std::vector<MenuItem> items;
        items.emplace_back("None", "", MenuItem::Position::Top, -1);
        for (auto &stream : file.mediaInfo.subtitles) {
            items.emplace_back(stream.title + "\nLang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuSubtitlesStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "SUBTITLES", items, MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setSelection(MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setVisibility(Visibility::Hidden, false);
        menuSubtitlesStreams->setLayer(3);
        add(menuSubtitlesStreams);
    }

    if (file.mediaInfo.playbackInfo.position > 0) {
        std::string msg = "Resume playback at "
                          + pplay::Utility::formatTime(file.mediaInfo.playbackInfo.position) + " ?";
        if (main->getMessageBox()->show("RESUME", msg, "RESUME", "RESTART") == MessageBox::LEFT) {
            seek(file.mediaInfo.playbackInfo.position);
        }
    }

    setVideoStream(file.mediaInfo.playbackInfo.vid_id);
    setAudioStream(file.mediaInfo.playbackInfo.aud_id);
    setSubtitleStream(file.mediaInfo.playbackInfo.sub_id);

    resume();
    setFullscreen(true);
}

void Player::onStopEvent() {

    file.mediaInfo.save();
    main->getMenuVideo()->reset();
    osd->reset();

    // Audio
    if (menuAudioStreams) {
        delete (menuAudioStreams);
        menuAudioStreams = nullptr;
    }
    // Video
    if (menuVideoStreams) {
        delete (menuVideoStreams);
        menuVideoStreams = nullptr;
    }
    // Subtitles
    if (menuSubtitlesStreams) {
        delete (menuSubtitlesStreams);
        menuSubtitlesStreams = nullptr;
    }

    pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif

    if (main->isExiting()) {
        main->setRunningStop();
    } else if (isStopped()) {
        setFullscreen(false, true);
    }
}

void Player::onUpdate() {

    //TODO: cache-buffering-state
    // handle mpv events
    if (mpv.available) {
        mpv_event *event = mpv_wait_event(mpv.handle, 0);
        if (event) {
            switch (event->event_id) {
                case MPV_EVENT_FILE_LOADED:
                    printf("MPV_EVENT_FILE_LOADED\n");
                    onLoadEvent();
                    main->getStatus()->hide();
                    break;
                case MPV_EVENT_START_FILE:
                    printf("MPV_EVENT_START_FILE\n");
                    main->getStatus()->show("Please Wait...", "Loading... " + file.name, true);
                    break;
                case MPV_EVENT_END_FILE:
                    printf("MPV_EVENT_END_FILE\n");
                    onStopEvent();
                    main->getStatus()->hide();
                    if (!isFullscreen()) {
                        main->getStatus()->show("Error...", "Could not load file");
                        printf("Player::load: could not load file\n");
                    }
                    break;
                case MPV_EVENT_SHUTDOWN:
                    printf("MPV_EVENT_SHUTDOWN\n");
                    break;
                case MPV_EVENT_LOG_MESSAGE:
                    break;
                case MPV_EVENT_GET_PROPERTY_REPLY:
                    break;
                case MPV_EVENT_SET_PROPERTY_REPLY:
                    break;
                case MPV_EVENT_COMMAND_REPLY:
                    break;
                case MPV_EVENT_TRACKS_CHANGED:
                    break;
                case MPV_EVENT_TRACK_SWITCHED:
                    break;
                case MPV_EVENT_IDLE:
                    break;
                case MPV_EVENT_PAUSE:
                    break;
                case MPV_EVENT_UNPAUSE:
                    break;
                case MPV_EVENT_TICK:
                    break;
                case MPV_EVENT_SCRIPT_INPUT_DISPATCH:
                    break;
                case MPV_EVENT_CLIENT_MESSAGE:
                    break;
                case MPV_EVENT_VIDEO_RECONFIG:
                    break;
                case MPV_EVENT_AUDIO_RECONFIG:
                    break;
                case MPV_EVENT_METADATA_UPDATE:
                    break;
                case MPV_EVENT_SEEK:
                    break;
                case MPV_EVENT_PLAYBACK_RESTART:
                    printf("MPV_EVENT_PLAYBACK_RESTART\n");
                    break;
                case MPV_EVENT_PROPERTY_CHANGE:
                    break;
                case MPV_EVENT_CHAPTER_CHANGE:
                    break;
                case MPV_EVENT_QUEUE_OVERFLOW:
                    break;
                case MPV_EVENT_HOOK:
                    break;
                case MPV_EVENT_NONE:
                    break;
            }
        }
    }
}

bool Player::onInput(c2d::Input::Player *players) {

    unsigned int keys = players[0].keys;

    if (isStopped()
        || main->getFiler()->isVisible()
        || main->getMenuVideo()->isVisible()
        || (getMenuVideoStreams() && getMenuVideoStreams()->isVisible())
        || (getMenuAudioStreams() && getMenuAudioStreams()->isVisible())
        || (getMenuSubtitlesStreams() && getMenuSubtitlesStreams()->isVisible())) {
        return C2DObject::onInput(players);
    }

    if (keys & c2d::Input::Key::Fire5) {
        setSpeed(1);
    } else if (keys & c2d::Input::Key::Fire6) {
        double new_speed = getSpeed() * 2;
        if (new_speed <= 100) {
            setSpeed(new_speed);
        }
    }

    if (osd->isVisible()) {
        return C2DObject::onInput(players);
    }

    //////////////////
    /// handle inputs
    //////////////////
    if ((keys & Input::Key::Fire1) || (keys & Input::Key::Down)) {
        if (!osd->isVisible()) {
            osd->setVisibility(Visibility::Visible, true);
            main->getStatusBar()->setVisibility(Visibility::Visible, true);
        }
    } else if (keys & c2d::Input::Key::Left || keys & Input::Key::Fire2) {
        setFullscreen(false);
    } else if (keys & c2d::Input::Key::Right) {
        main->getMenuVideo()->setVisibility(Visibility::Visible, true);
    }

    return true;
}

void Player::setVideoStream(int streamId) {
    if (streamId > -1) {
        std::string cmd = "no-osd set vid " + std::to_string(streamId);
        mpv_command_string(mpv.handle, cmd.c_str());
        file.mediaInfo.playbackInfo.vid_id = streamId;
    }
}

void Player::setAudioStream(int streamId) {
    if (streamId > -1) {
        std::string cmd = "no-osd set aid " + std::to_string(streamId);
        mpv_command_string(mpv.handle, cmd.c_str());
        file.mediaInfo.playbackInfo.aud_id = streamId;
    }
}

void Player::setSubtitleStream(int streamId) {
    std::string cmd = "no-osd set sid ";
    cmd += streamId == -1 ? "no" : std::to_string(streamId);
    mpv_command_string(mpv.handle, cmd.c_str());
    file.mediaInfo.playbackInfo.sub_id = streamId;
}

int Player::getVideoStream() {
    return file.mediaInfo.playbackInfo.vid_id;
}

int Player::getAudioStream() {
    return file.mediaInfo.playbackInfo.aud_id;
}

int Player::getSubtitleStream() {
    return file.mediaInfo.playbackInfo.sub_id;
}

int Player::seek(double position) {

    std::string cmd = "no-osd seek " + std::to_string(position) + " absolute";
    mpv_command_string(mpv.handle, cmd.c_str());

    return 0;
}

void Player::setSpeed(double speed) {

    std::string cmd = "set speed " + std::to_string(speed);
    mpv_command_string(mpv.handle, cmd.c_str());
    osd->setVisibility(Visibility::Visible, true);
}

double Player::getSpeed() {

    double res = -1;

    if (mpv.available) {
        mpv_get_property(mpv.handle, "speed", MPV_FORMAT_DOUBLE, &res);
    }

    return res;
}

void Player::pause() {

    if (mpv.available) {
        mpv_command_string(mpv.handle, "set pause yes");
    }

    pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
}

void Player::resume() {

    if (mpv.available) {
        mpv_command_string(mpv.handle, "set pause no");
    }

    if (main->getConfig()->getOption(OPT_CPU_BOOST)->getString() == "Enabled") {
        pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Max);
    }
#ifdef __SWITCH__
    appletSetMediaPlaybackState(true);
#endif
}

void Player::stop() {

    printf("Player::stop\n");
    if (mpv.available && !isStopped()) {
        // save media info
        long position = getPlaybackPosition();
        if (position > 5) {
            file.mediaInfo.playbackInfo.position = (int) position - 5;
        } else {
            file.mediaInfo.playbackInfo.position = 0;
        }

        // stop mpv playback
        mpv_command_string(mpv.handle, "stop");
    }
}

bool Player::isStopped() {

    int res = -1;

    if (mpv.available) {
        mpv_get_property(mpv.handle, "playback-abort", MPV_FORMAT_FLAG, &res);
    }

    return res == 1;
}

bool Player::isPaused() {

    int res = -1;

    if (mpv.available) {
        mpv_get_property(mpv.handle, "pause", MPV_FORMAT_FLAG, &res);
    }

    return res == 1;
}

bool Player::isFullscreen() {
    return fullscreen;
}

void Player::setFullscreen(bool fs, bool hide) {

    if (fs == fullscreen) {
        return;
    }

    fullscreen = fs;

    if (!fullscreen) {
        if (hide) {
            setVisibility(Visibility::Hidden, true);
        } else {
            tweenPosition->play(TweenDirection::Backward);
            tweenScale->play(TweenDirection::Backward);
        }
        texture->showGradients();
        main->getMenuVideo()->setVisibility(Visibility::Hidden, true);
        if (menuVideoStreams) {
            menuVideoStreams->setVisibility(Visibility::Hidden, true);
        }
        if (menuAudioStreams) {
            menuAudioStreams->setVisibility(Visibility::Hidden, true);
        }
        if (menuSubtitlesStreams) {
            menuSubtitlesStreams->setVisibility(Visibility::Hidden, true);
        }
        main->getFiler()->setVisibility(Visibility::Visible, true);
        main->getTitle()->setVisibility(Visibility::Visible, true);
        main->getStatusBar()->setVisibility(Visibility::Visible, true);
    } else {
        texture->hideGradients();
        main->getFiler()->setVisibility(Visibility::Hidden, true);
        main->getTitle()->setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
        setVisibility(Visibility::Visible, true);
    }
}

int Player::getVideoBitrate() {
    double bitrate = 0;
    if (mpv.available) {
        mpv_get_property(mpv.handle, "video-bitrate", MPV_FORMAT_DOUBLE, &bitrate);
    }
    return (int) bitrate;
}

int Player::getAudioBitrate() {
    double bitrate = 0;
    if (mpv.available) {
        mpv_get_property(mpv.handle, "audio-bitrate", MPV_FORMAT_INT64, &bitrate);
    }
    return (int) bitrate;
}

long Player::getPlaybackDuration() {
    long duration = 0;
    if (mpv.available) {
        mpv_get_property(mpv.handle, "duration", MPV_FORMAT_INT64, &duration);
    }
    return duration;
}

long Player::getPlaybackPosition() {
    long position = 0;
    if (mpv.available) {
        mpv_get_property(mpv.handle, "playback-time", MPV_FORMAT_INT64, &position);
    }
    return position;
}

MenuVideoSubmenu *Player::getMenuVideoStreams() {
    return menuVideoStreams;
}

MenuVideoSubmenu *Player::getMenuAudioStreams() {
    return menuAudioStreams;
}

MenuVideoSubmenu *Player::getMenuSubtitlesStreams() {
    return menuSubtitlesStreams;
}

const std::string &Player::getTitle() const {
    return file.name;
}

PlayerOSD *Player::getOSD() {
    return osd;
}


