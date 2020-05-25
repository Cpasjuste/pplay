//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include "main.h"
#include "player.h"
#include "player_osd.h"
#include "video_texture.h"
#include "utility.h"

using namespace c2d;

Player::Player(Main *_main) : Rectangle(_main->getSize()) {

    main = _main;

    Vector2f pos = main->getSize();
    setPosition(pos);
    setOrigin(Origin::BottomRight);

    tweenScale = new TweenScale({0.25f, 0.25f}, {1.0f, 1.0f}, 0.5f);
    add(tweenScale);
    tweenPosition = new TweenPosition({pos.x - 16, pos.y - 16}, {pos}, 0.5f);
    add(tweenPosition);

    setVisibility(Visibility::Hidden);

    mpv = new Mpv(main->getIo()->getDataPath() + "mpv", true);

    // TODO: create texture of video size?
    texture = new VideoTexture(pos, mpv);
    texture->setOutlineColor(Color::Red);
    texture->setOutlineThickness(4);
    add(texture);

    osd = new PlayerOSD(main);
    add(osd);
}

Player::~Player() {
    delete (mpv);
}

bool Player::load(const MediaFile &f) {

    file = f;
    std::string path = file.path;
#ifdef __SMB_SUPPORT__
    if (Utility::startWith(path, "smb://")) {
        std::replace(path.begin(), path.end(), '\\', '/');
    }
#endif

    int res = mpv->load(path, Mpv::LoadType::Replace, "pause=yes,speed=1");
    if (res != 0) {
        main->getStatus()->show("Error...", "Could not play file:\n" + std::string(mpv_error_string(res)));
        printf("Player::load: could not play file: %s\n", mpv_error_string(res));
        return false;
    }

    return true;
}

void Player::onLoadEvent() {

    // load/update media information (include playback info)
    file.mediaInfo = mpv->getMediaInfo(file);
    file.mediaInfo.save(file);

    // TODO: is this really needed as it should be extracted from scrapper
    // main->getFiler()->setMediaInfo(file, file.mediaInfo);

    // build video track selection menu
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
    // build audio track selection menu
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
    // build subtitles track selection menu
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

    // restore saved tracks id
    setVideoStream(file.mediaInfo.playbackInfo.vid_id);
    setAudioStream(file.mediaInfo.playbackInfo.aud_id);
    setSubtitleStream(file.mediaInfo.playbackInfo.sub_id);

    // resume playback if wanted
    if (file.mediaInfo.playbackInfo.position > 0) {
        std::string msg = "Resume playback at "
                          + pplay::Utility::formatTime(file.mediaInfo.playbackInfo.position) + " ?";
        if (main->getMessageBox()->show("RESUME", msg, "RESUME", "RESTART") == MessageBox::LEFT) {
            mpv->seek(file.mediaInfo.playbackInfo.position);
        }
    }

    resume();
    setFullscreen(true);
}

void Player::onStopEvent(int reason) {

    main->getStatus()->hide();
    main->getMenuVideo()->reset();
    osd->reset();

    if (reason == MPV_END_FILE_REASON_ERROR) {
        main->getStatus()->show("Error...", "Could not load file");
        printf("Player::load: could not load file\n");
    } else if (reason == MPV_END_FILE_REASON_EOF) {
        file.mediaInfo.playbackInfo.position = 0;
    }

    // save mediaInfo (again, for playback position, tracks id..)
    file.mediaInfo.save(file);

    // audio
    if (menuAudioStreams != nullptr) {
        delete (menuAudioStreams);
        menuAudioStreams = nullptr;
    }
    // video
    if (menuVideoStreams != nullptr) {
        delete (menuVideoStreams);
        menuVideoStreams = nullptr;
    }
    // subtitles
    if (menuSubtitlesStreams != nullptr) {
        delete (menuSubtitlesStreams);
        menuSubtitlesStreams = nullptr;
    }

    pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif

    if (main->isExiting()) {
        main->setRunningStop();
    } else if (mpv->isStopped()) {
        setFullscreen(false, true);
    }
}

void Player::onUpdate() {

    //TODO: cache-buffering-state
    // handle mpv events
    if (mpv->isAvailable()) {
        mpv_event *event = mpv->getEvent();
        if (event != nullptr) {
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
                    //auto ef = (mpv_event_end_file *) event->data;
                    //printf("MPV_EVENT_END_FILE: %i\n", ef->reason);
                    onStopEvent(((mpv_event_end_file *) event->data)->reason);
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

    if (isVisible()) {
        Rectangle::onUpdate();
    }
}

bool Player::onInput(c2d::Input::Player *players) {

    unsigned int keys = players[0].keys;

    if (mpv->isStopped()
        || main->getFiler()->isVisible()
        || main->getMenuVideo()->isVisible()
        || (getMenuVideoStreams() != nullptr && getMenuVideoStreams()->isVisible())
        || (getMenuAudioStreams() != nullptr && getMenuAudioStreams()->isVisible())
        || (getMenuSubtitlesStreams() != nullptr && getMenuSubtitlesStreams()->isVisible())) {
        return C2DObject::onInput(players);
    }

    if (keys & c2d::Input::Key::Fire5) {
        setSpeed(1);
    } else if (keys & c2d::Input::Key::Fire6) {
        double old_speed = mpv->getSpeed();
        double new_speed = old_speed;
        if (old_speed < 1.5) {
            new_speed = 1.5;
        } else if (old_speed < 2) {
            new_speed = 2;
        } else {
            new_speed = old_speed * 2;
        }
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
        mpv->setVid(streamId);
        file.mediaInfo.playbackInfo.vid_id = streamId;
    }
}

void Player::setAudioStream(int streamId) {
    if (streamId > -1) {
        mpv->setAid(streamId);
        file.mediaInfo.playbackInfo.aud_id = streamId;
    }
}

void Player::setSubtitleStream(int streamId) {
    mpv->setSid(streamId);
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

void Player::setSpeed(double speed) {
    mpv->setSpeed(speed);
    osd->setVisibility(Visibility::Visible, true);
}

void Player::pause() {
    mpv->pause();
    pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Min);
#ifdef __SWITCH__
    appletSetMediaPlaybackState(false);
#endif
}

void Player::resume() {
    mpv->resume();
    if (main->getConfig()->getOption(OPT_CPU_BOOST)->getString() == "Enabled") {
        pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Max);
    }
#ifdef __SWITCH__
    appletSetMediaPlaybackState(true);
#endif
}

void Player::stop() {
    if (!mpv->isStopped()) {
        file.mediaInfo.playbackInfo.position = (int) mpv->getPosition();
        mpv->stop();
    }
}

bool Player::isFullscreen() {
    return fullscreen;
}

void Player::setFullscreen(bool fs, bool hide) {

    if (fs == fullscreen) {
        if (hide) {
            setVisibility(Visibility::Hidden, true);
        }
        return;
    }

    fullscreen = fs;

    if (!fullscreen) {
        if (hide) {
            setVisibility(Visibility::Hidden, true);
        } else {
            tweenScale->play(TweenDirection::Backward);
            tweenPosition->play(TweenDirection::Backward);
        }
        texture->showFade();
        main->getMenuVideo()->setVisibility(Visibility::Hidden, true);
        if (menuVideoStreams != nullptr) {
            menuVideoStreams->setVisibility(Visibility::Hidden, true);
        }
        if (menuAudioStreams != nullptr) {
            menuAudioStreams->setVisibility(Visibility::Hidden, true);
        }
        if (menuSubtitlesStreams != nullptr) {
            menuSubtitlesStreams->setVisibility(Visibility::Hidden, true);
        }
        main->getFiler()->setVisibility(Visibility::Visible, true);
        main->getStatusBar()->setVisibility(Visibility::Visible, true);
    } else {
        texture->hideFade();
        main->getFiler()->setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
        setVisibility(Visibility::Visible, true);
    }
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

Mpv *Player::getMpv() {
    return mpv;
}
