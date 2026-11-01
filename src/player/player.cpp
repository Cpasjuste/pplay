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

// tracks without a title come with "Unknown": tell them apart by number instead
static std::string trackTitle(const MediaInfo::Track &track, const char *kind) {
    if (track.title.empty() || track.title == "Unknown") {
        return std::string(kind) + " " + std::to_string(track.id);
    }
    return track.title;
}

// subtitles that are pictures (not text): mpv would need a GPU shader that is not available
static bool isBitmapSubtitle(const std::string &codec) {
    return codec == "hdmv_pgs_subtitle" || codec == "dvd_subtitle"
           || codec == "dvb_subtitle" || codec == "xsub";
}

Player::Player(Main *_main) : Rectangle(_main->getSize()) {

    main = _main;

    Vector2f pos = main->getSize();
    Player::setPosition(pos);
    Player::setOrigin(Origin::BottomRight);

    tweenScale = new TweenScale({0.25f, 0.25f}, {1.0f, 1.0f}, 0.5f);
    Player::add(tweenScale);
    tweenPosition = new TweenPosition({pos.x - 16, pos.y - 16}, {pos}, 0.5f);
    Player::add(tweenPosition);

    Player::setVisibility(Visibility::Hidden);

    mpv = new Mpv(main->getIo()->getDataPath() + "mpv", true, resolveHwdec(),
              main->getConfig()->getOption(OPT_SW_SPEED)->getString());

#ifndef FULL_TEXTURE_TEST
    texture = new VideoTexture(main, pos);
    texture->setOutlineColor(Color::Red);
    texture->setOutlineThickness(4);
#else
    texture = new VideoTexture(main, {64, 64});
    texture->setOrigin(Origin::Center);
    texture->setPosition(Vector2f(Player::getSize().x / 2, Player::getSize().y / 2));
#endif
    Player::add(texture);

    osd = new PlayerOSD(main);
    Player::add(osd);
}

Player::~Player() {
    delete (mpv);
    releaseHwdecGuard();
}

bool Player::load(const MediaFile &f) {

    file = f;
    std::string path = file.path;
#ifdef __SMB2__
#if 0
    if (Utility::startWith(path, "smb://")) {
        std::replace(path.begin(), path.end(), '\\', '/');
    }
#endif
#endif

    // disable subtitles if slang option not set in "mpv.conf" (overridden by "watch_later")
    // (mpv_get_property_string may return NULL: strlen(NULL) would crash the app)
    char *slang = mpv_get_property_string(mpv->getHandle(), "slang");
    if (slang == nullptr || strlen(slang) == 0) {
        printf("slang not set, disabling subtitles by default\n");
        mpv_set_option_string(mpv->getHandle(), "sid", "no");
    }
    if (slang != nullptr) {
        mpv_free(slang);
    }

    // external subtitles next to the video (srt, ass/ssa, vtt, smi/sami, idx+sub, sup...)
    externalSubs = pplay::Subtitles::find(path, main->getIo()->getDataPath() + "subcache/");
    for (auto &sub: externalSubs) {
        printf("Player::load: external subtitle: %s (%s)\n", sub.path.c_str(), sub.note.c_str());
    }

    int res = mpv->load(path, Mpv::LoadType::Replace, "pause=yes,speed=1");
    if (res != 0) {
        main->getStatus()->show("Error...", "Could not play file:\n" + std::string(mpv_error_string(res)));
        printf("Player::load: could not play file: %s\n", mpv_error_string(res));
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// Hardware decoding (PS4 libSceVideodec2 through ffmpeg's "orbis" wrapper)
//
// pplay.cfg: HWDEC="auto" (default) | "no" | "force" | any mpv hwdec name
//
// The GPU decoder cannot be tested from outside a console and a driver fault
// takes the whole system down. So the first seconds of every hardware decoded
// playback are guarded: "hwdec.guard" is created when a file starts and
// deleted once playback proved stable. If the console hangs in between, the
// guard is still there at the next start and hardware decoding is turned off
// ("hwdec.disabled") instead of crashing again. Delete hwdec.disabled, or set
// HWDEC="force", to try again.
// ---------------------------------------------------------------------------
std::string Player::resolveHwdec() {
#ifdef __PS4__
    std::string mode = main->getConfig()->getOption(OPT_HWDEC)->getString();
    std::string dir = main->getIo()->getDataPath();
    hwdecGuardFile = dir + "hwdec.guard";
    std::string disabledFile = dir + "hwdec.disabled";

    if (mode == "no" || mode == "off" || mode == "none") {
        ::remove(hwdecGuardFile.c_str());
        return "no";
    }
    if (mode != "auto" && mode != "force") {
        // explicit mpv method chosen by the user: no guard, no questions asked
        hwdecGuardFile.clear();
        return mode;
    }

    if (mode == "force") {
        ::remove(disabledFile.c_str());
    } else if (main->getIo()->exist(hwdecGuardFile)) {
        // previous hardware decoded playback never proved itself: it crashed
        ::remove(hwdecGuardFile.c_str());
        FILE *f = fopen(disabledFile.c_str(), "w");
        if (f) {
            fputs("hardware decoding was turned off because the console stopped while using it.\n"
                  "delete this file (or set HWDEC=\"force\" in pplay.cfg) to try again.\n", f);
            fclose(f);
        }
        hwdecNotice = true;
    }
    if (main->getIo()->exist(disabledFile)) {
        printf("Player: hardware decoding disabled (%s)\n", disabledFile.c_str());
        hwdecGuardFile.clear();
        return "no";
    }

    hwdecMethod = "orbis-copy";
    return hwdecMethod;
#else
    return "";
#endif
}

void Player::armHwdecGuard() {
    if (hwdecGuardFile.empty() || hwdecMethod.empty() || hwdecGuardArmed) {
        return;
    }
    FILE *f = fopen(hwdecGuardFile.c_str(), "w");
    if (f) {
        fputs("hardware decoding in progress\n", f);
        fclose(f);
        hwdecGuardArmed = true;
    }
}

void Player::releaseHwdecGuard() {
    if (hwdecGuardArmed) {
        ::remove(hwdecGuardFile.c_str());
        hwdecGuardArmed = false;
    }
}

void Player::updateHwdecGuard() {
    if (!hwdecGuardArmed || !mpv->isAvailable()) {
        return;
    }

    long position = mpv->getPosition();
    char *current = mpv_get_property_string(mpv->getHandle(), "hwdec-current");
    std::string hw = current ? current : "";
    if (current) {
        mpv_free(current);
    }
    bool hardware = hw.find("orbis") != std::string::npos;

    // software decoding (stream not supported by the hardware): nothing risky
    // is going on; hardware decoding: wait for a few seconds of clean playback
    if ((!hardware && position >= 3) || (hardware && position >= 10)) {
        releaseHwdecGuard();
        printf("Player: hwdec guard released (hwdec-current=%s)\n", hw.c_str());
    }
}

// Keep the "CPU renders the subtitles" mode in line with the subtitle track mpv is showing
// (selected from the menu, by mpv.conf or by pplay itself while loading the file).
void Player::updateBitmapSubtitles() {
    if (!mpv->isAvailable() || mpv->isStopped()) {
        return;
    }

    bool bitmap = isBitmapSubtitle(mpv->getSubtitleCodec());
    if (bitmap == mpv->isBitmapSubtitleRendering()) {
        return;
    }

    if (mpv->setBitmapSubtitleRendering(bitmap) < 0 && bitmap) {
        // can not draw this subtitle safely: better no subtitle than a blue screen
        printf("Player: image subtitles can not be drawn, turning subtitles off\n");
        mpv->setSid(-1);
        main->getStatus()->show("Subtitles", "This subtitle format can not be displayed");
    }
}

void Player::onLoadEvent() {
    updateBitmapSubtitles();

    // add external subtitles first so they show up in the track list below
    for (auto &sub: externalSubs) {
        int r = mpv->addSubtitle(sub.path, sub.title, sub.lang);
        if (r < 0) {
            printf("Player::onLoadEvent: could not add subtitle %s: %s\n", sub.path.c_str(), mpv_error_string(r));
        }
    }
    externalSubs.clear();

    // load/update media information
    file.mediaInfo = mpv->getMediaInfo(file);
    file.mediaInfo.save(file);

    // TODO: is this really needed as it should be extracted from scrapper
    // main->getFiler()->setMediaInfo(file, file.mediaInfo);

    // build video track selection menu
    if (!file.mediaInfo.videos.empty()) {
        // videos menu options
        std::vector<MenuItem> items;
        for (auto &stream: file.mediaInfo.videos) {
            items.emplace_back(trackTitle(stream, "Video") + "\n" + stream.language + " " + stream.codec + " "
                               + std::to_string(stream.width) + "x" + std::to_string(stream.height),
                               "", MenuItem::Position::Top, stream.id);
        }
        menuVideoStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "VIDEO", items, MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setVisibility(Visibility::Hidden, false);
        menuVideoStreams->setLayer(3);
        add(menuVideoStreams);
    }

    // build audio track selection menu
    if (!file.mediaInfo.audios.empty()) {
        // audios menu options
        std::vector<MenuItem> items;
        for (auto &stream: file.mediaInfo.audios) {
            items.emplace_back(trackTitle(stream, "Audio") + "\n" + stream.language + " "
                               + stream.codec + " " + std::to_string(stream.channels) + "ch "
                               + std::to_string(stream.sample_rate / 1000) + " Khz",
                               "", MenuItem::Position::Top, stream.id);
        }
        menuAudioStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "AUDIO", items, MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setVisibility(Visibility::Hidden, false);
        menuAudioStreams->setLayer(3);
        add(menuAudioStreams);
    }

    // build subtitles track selection menu
    if (!file.mediaInfo.subtitles.empty()) {
        // subtitles menu options
        std::vector<MenuItem> items;
        items.emplace_back("None", "", MenuItem::Position::Top, -1);
        for (auto &stream: file.mediaInfo.subtitles) {
            // many files have untitled tracks: show the language and the format (srt, ass, pgs...)
            items.emplace_back(trackTitle(stream, "Subtitle") + "\n"
                               + (stream.language.empty() || stream.language == "N/A" ? "und" : stream.language)
                               + (stream.codec.empty() ? "" : " [" + stream.codec + "]"),
                               "", MenuItem::Position::Top, stream.id);
        }
        menuSubtitlesStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "SUBTITLES", items, MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setVisibility(Visibility::Hidden, false);
        menuSubtitlesStreams->setLayer(3);
        add(menuSubtitlesStreams);
    }

#ifdef FULL_TEXTURE_TEST
    texture->resize({file.mediaInfo.videos.at(0).width,
                     file.mediaInfo.videos.at(0).height});
    float scaling = std::min(
            getSize().x / texture->getSize().x,
            getSize().y / texture->getSize().y);
    texture->setScale(scaling, scaling);
#endif

    resume();
    setFullscreen(true);
}

void Player::onStopEvent(int reason) {
    // the decoder was closed cleanly (or never got going): nothing to guard
    releaseHwdecGuard();
    mpv->setBitmapSubtitleRendering(false);
    main->getStatus()->hide();
    main->getMenuVideo()->reset();
    osd->reset();

    if (reason == MPV_END_FILE_REASON_ERROR) {
        main->getStatus()->show("Error...", "Could not load file");
        printf("Player::load: could not load file\n");
    }

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
    if (hwdecNotice) {
        hwdecNotice = false;
        main->getStatus()->show("Hardware decoding disabled",
                                "The console stopped during hardware decoding. Software decoding will be used. "
                                "Delete hwdec.disabled to try again.");
    }
    updateHwdecGuard();
    if (++bitmapSubTicks >= 15) {
        // a few times per second is plenty
        bitmapSubTicks = 0;
        updateBitmapSubtitles();
    }
    if (mpv->isAvailable()) {
        mpv_event *event = mpv->getEvent();
        if (event != nullptr) {
            switch (event->event_id) {
                case MPV_EVENT_START_FILE:
                    printf("MPV_EVENT_START_FILE\n");
                    armHwdecGuard();
                    main->getStatus()->show("Please Wait...", "Loading... " + file.name, true);
                    break;
                case MPV_EVENT_FILE_LOADED:
                    printf("MPV_EVENT_FILE_LOADED\n");
                    onLoadEvent();
                    main->getStatus()->hide();
                    break;
                case MPV_EVENT_END_FILE:
                    printf("MPV_EVENT_END_FILE\n");
                    onStopEvent(((mpv_event_end_file *) event->data)->reason);
                    break;
                default:
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
        double new_speed = mpv->getSpeed() * 2;
        if (new_speed <= 100) {
            setSpeed(new_speed);
        }
    }
#ifdef __PS4__
    else if (keys & c2d::Input::Key::Select) {
        osd->setVisibility(c2d::Visibility::Visible);
        getMpv()->seek(-60.0);
    } else if (keys & c2d::Input::Key::Start) {
        osd->setVisibility(c2d::Visibility::Visible);
        getMpv()->seek(60.0);
    }
#endif

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
    printf("Player::setVideoStream: %i\n", streamId);
    mpv->setVid(streamId);
}

void Player::setAudioStream(int streamId) {
    printf("Player::setAudioStream: %i\n", streamId);
    mpv->setAid(streamId);
}

void Player::setSubtitleStream(int streamId) {
    printf("Player::setSubtitleStream: %i\n", streamId);

    // switch the way subtitles are drawn BEFORE mpv starts showing the new track
    bool bitmap = false;
    for (auto &stream: file.mediaInfo.subtitles) {
        if (stream.id == streamId) {
            bitmap = isBitmapSubtitle(stream.codec);
            break;
        }
    }
    if (mpv->setBitmapSubtitleRendering(bitmap) < 0 && bitmap) {
        main->getStatus()->show("Subtitles", "This subtitle format can not be displayed");
        mpv->setSid(-1);
        return;
    }

    mpv->setSid(streamId);
}

int Player::getVideoStream() {
    return mpv->getVid();
}

int Player::getAudioStream() {
    return mpv->getAid();
}

int Player::getSubtitleStream() {
    return mpv->getSid();
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
#ifdef __SWITCH__
    if (main->getConfig()->getOption(OPT_CPU_BOOST)->getString() == "Enabled") {
        pplay::Utility::setCpuClock(pplay::Utility::CpuClock::Max);
    }
    appletSetMediaPlaybackState(true);
#endif
}

void Player::stop() {
    mpv->stop();
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
