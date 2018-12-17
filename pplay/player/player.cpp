//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"
#include "player.h"
#include "player_osd.h"
#include "gradient_rectangle.h"
#include "kitchensink/internal/kitdecoder.h"

using namespace c2d;

Player::Player(Main *_main) : Rectangle(_main->getSize()) {

    main = _main;

    setOrigin(Origin::TopRight);

    tweenPosition = new TweenPosition(
            {main->getSize().x - 32, 32},
            {main->getSize().x, 0}, 0.5f);
    add(tweenPosition);
    tweenScale = new TweenScale({0.6f, 0.6f}, {1.0f, 1.0f}, 0.5f);
    add(tweenScale);

    osd = new PlayerOSD(main);
    osd->setLayer(3);
    add(osd);

    audio = new C2DAudio(48000, 24);

    setVisibility(Visibility::Hidden);
}

Player::~Player() {
    delete (audio);
    // crash on switch?
    //stop();
}

bool Player::load(const MediaFile &file) {

    stop();
    stopped = false;

    // avformat_network_init/deinit handled in media info thread
    int err = Kit_Init(KIT_INIT_ASS);
    if (err != 0) {
        printf("unable to initialize Kitchensink: %s\n", Kit_GetError());
        stop();
        return false;
    }

    Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 256);
    Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 2048);
    //Kit_SetHint(KIT_HINT_SUBTITLE_BUFFER_FRAMES, 8);

    // open source file
    printf("Player::load: %s\n", file.path.c_str());
    source = Kit_CreateSourceFromUrl(file.path.c_str());
    if (!source) {
        main->getStatus()->show("Error...", Kit_GetError());
        printf("unable to load '%s': %s\n", file.path.c_str(), Kit_GetError());
        stop();
        return false;
    }

    // find available streams
    video_streams.size = Kit_GetSourceStreamList(
            source, KIT_STREAMTYPE_VIDEO, video_streams.streams, MAX_STREAM_LIST_SIZE);
    audio_streams.size = Kit_GetSourceStreamList(
            source, KIT_STREAMTYPE_AUDIO, audio_streams.streams, MAX_STREAM_LIST_SIZE);
    subtitles_streams.size = Kit_GetSourceStreamList(
            source, KIT_STREAMTYPE_SUBTITLE, subtitles_streams.streams, MAX_STREAM_LIST_SIZE);
    printf("Player::load: \n\tVIDEO STREAMS: %i\n\tAUDIO STREAMS: %i\n\tSUBTITLES STREAMS: %i\n",
           video_streams.size, audio_streams.size, subtitles_streams.size);
    if (!video_streams.size && !audio_streams.size) {
        printf("no usable audio or video stream found: %s\n", Kit_GetError());
        main->getStatus()->show("Error...", "no video or audio stream found");
        stop();
        return false;
    }

    // set default font
    Kit_LibraryState *state = Kit_GetLibraryState();
    snprintf(state->subtitle_font_path, 511, "%sskin/font.ttf",
             getMain()->getIo()->getDataReadPath().c_str());

    // create the player
    kit_player = Kit_CreatePlayer(
            source,
            video_streams.getCurrentStream(),
            audio_streams.getCurrentStream(),
            subtitles_streams.getCurrentStream(),
            (int) getSize().x, (int) getSize().y);
    if (!kit_player) {
        main->getStatus()->show("Error...", Kit_GetError());
        printf("unable to create player: %s\n", Kit_GetError());
        stop();
        return false;
    }

    title = file.name;

    // we should be good to go, set max cpu for now
    setCpuClock(CpuClock::Max);

    // hack, needs to fix subtitles problem when not enabled on Kit_CreatePlayer
    subtitles_streams.current = -1;

    // get some information
    Kit_GetPlayerInfo(kit_player, &playerInfo);
    printf("Video(%s, %s): %i x %i , Audio(%s): %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.samplerate);

    if (audio_streams.size > 0) {
        // audios menu options
        std::vector<MenuItem> items;
        for (auto &stream : file.media.audios) {
            items.emplace_back("Lang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuAudioStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "AUDIO______", items, MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setVisibility(Visibility::Hidden, false);
        menuAudioStreams->setLayer(3);
        add(menuAudioStreams);
    }

    if (video_streams.size > 0) {
        texture = new VideoTexture(
                {playerInfo.video.output.width, playerInfo.video.output.height});
        texture->setFilter(Texture::Filter::Linear);
        add(texture);

        // videos menu options
        std::vector<MenuItem> items;
        for (auto &stream : file.media.videos) {
            items.emplace_back("Lang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuVideoStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "VIDEO______", items, MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setVisibility(Visibility::Hidden, false);
        menuVideoStreams->setLayer(3);
        add(menuVideoStreams);
    }

    if (subtitles_streams.size > 0) {
        textureSub = new SubtitlesTexture();
        textureSub->setFilter(Texture::Filter::Point);
        void *buf;
        textureSub->lock(nullptr, &buf, nullptr);
        memset(buf, 0, 1024 * 1024 * 4);
        textureSub->unlock();
        textureSub->setVisibility(Visibility::Hidden);
        add(textureSub);

        // subtitles menu options
        std::vector<MenuItem> items;
        items.emplace_back("None", "", MenuItem::Position::Top, -1);
        for (auto &stream : file.media.subtitles) {
            items.emplace_back("Lang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuSubtitlesStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "SUBTITLES______", items, MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setVisibility(Visibility::Hidden, false);
        menuSubtitlesStreams->setLayer(3);
        add(menuSubtitlesStreams);
    }

    // preload/cache some stream frames
    loading = true;
    int flip = 0, ret = 1;
    auto *dec = (Kit_Decoder *) kit_player->decoders[0];
    if (!dec) {
        // try with audio decoder
        dec = (Kit_Decoder *) kit_player->decoders[1];
    }
    while (ret > 0) {
        ret = Kit_PlayerPlay(kit_player);
        if (flip % 30 == 0) {
            if (dec) {
                float total = dec->buffer[KIT_DEC_BUF_OUT]->size;
                float current = Kit_GetBufferLength(dec->buffer[KIT_DEC_BUF_OUT]);
                int progress = (int) ((current / total) * 100.0f);
                std::string msg = "Loading..." + file.name + "... " + std::to_string(progress) + "%";
                main->getStatus()->show("Please Wait...", msg, true);
            }
            main->flip();
        }
        flip++;
    }
    loading = false;

    // start playback
    audio->pause(0);
    // must be after Kit_PlayerPlay pre-loading
    setVisibility(Visibility::Visible);

    return true;
}

bool Player::onInput(c2d::Input::Player *players) {

    if (main->getFiler()->isVisible()
        || main->getMenuVideo()->isVisible()
        || osd->isVisible()
        || (getMenuVideoStreams() && getMenuVideoStreams()->isVisible())
        || (getMenuAudioStreams() && getMenuAudioStreams()->isVisible())
        || (getMenuSubtitlesStreams() && getMenuSubtitlesStreams()->isVisible())) {
        return C2DObject::onInput(players);
    }

    //////////////////
    /// handle inputs
    //////////////////
    unsigned int keys = players[0].keys;
    if ((keys & Input::Key::Fire1) || (keys & Input::Key::Down)) {
        if (!osd->isVisible()) {
            osd->setVisibility(Visibility::Visible, true);
        }
    } else if (keys & c2d::Input::Key::Left || keys & Input::Key::Fire2) {
        setFullscreen(false);
    } else if (keys & c2d::Input::Key::Right) {
        main->getMenuVideo()->setVisibility(Visibility::Visible, true);
    }

    return true;
}


void Player::onDraw(c2d::Transform &transform) {

    if (loading) {
        return;
    }

    if (!isPlaying() && !isPaused()) {
        stop();
        if (isFullscreen()) {
            setFullscreen(false);
        }
        Rectangle::onDraw(transform);
        return;
    }

    //////////////////
    /// step ffmpeg
    //////////////////

    /// audio
    if (audio_streams.size > 0) {
        int queued = audio->getQueuedSize();
        if (queued < audio->getBufferSize()) {
            int need = audio->getBufferSize() - queued;
            while (need > 0) {
                int ret = Kit_GetPlayerAudioData(
                        kit_player, (unsigned char *) audio->getBuffer(), audio->getBufferSize());
                need -= ret;
                if (ret > 0) {
                    audio->play(audio->getBuffer(), ret);
                } else {
                    break;
                }
            }
        }
    }

    /// video
    if (video_streams.size > 0) {
        void *video_data;
        texture->lock(nullptr, &video_data, nullptr);
        if (Kit_GetPlayerVideoDataRaw(kit_player, video_data)) {
            texture->unlock();
        }
        // scaling
        Vector2f max_scale = {
                getSize().x / texture->getTextureRect().width,
                getSize().y / texture->getTextureRect().height};
        Vector2f scale = {max_scale.y, max_scale.y};
        if (scale.x > max_scale.x) {
            scale.x = scale.y = max_scale.x;
        }
        // TODO: set position to top right
        texture->setOrigin(Origin::Center);
        texture->setPosition(getSize().x / 2.0f, getSize().y / 2.0f);
        texture->setScale(scale);
    }

    /// Subtitles
    if (show_subtitles && subtitles_streams.size > 0) {
        int count = Kit_GetPlayerSubtitleDataRaw(
                kit_player, textureSub->pixels, textureSub->getRectsSrc(), textureSub->getRectsDst(), ATLAS_MAX);
        textureSub->setRectsCount(count);
        if (count > 0) {
            textureSub->unlock();
        }
    }

    Rectangle::onDraw(transform);
}

void Player::setVideoStream(int index) {

    if (index == video_streams.getCurrentStream()) {
        main->getStatus()->show("Info...", "Selected video stream already set");
        return;
    }

    if (texture && index > -1) {
        for (int i = 0; i < video_streams.size; i++) {
            if (index == video_streams.streams[i]) {
                video_streams.current = i;
            }
        }
        texture->setVisibility(Visibility::Visible);
        Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_VIDEO, index);
    } else {
        video_streams.current = -1;
        if (texture) {
            texture->setVisibility(Visibility::Hidden);
        }
    }
}

void Player::setAudioStream(int index) {

    if (index == audio_streams.getCurrentStream()) {
        main->getStatus()->show("Info...", "Selected audio stream already set");
        return;
    }

    if (index > -1) {
        for (int i = 0; i < audio_streams.size; i++) {
            if (index == audio_streams.streams[i]) {
                audio_streams.current = i;
            }
        }
        Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_AUDIO, index);
    } else {
        audio_streams.current = -1;
    }
}

void Player::setSubtitleStream(int index) {

    if (index == subtitles_streams.getCurrentStream()) {
        main->getStatus()->show("Info...", "Selected subtitles stream already set");
        return;
    }

    if (textureSub && index >= 0) {
        for (int i = 0; i < subtitles_streams.size; i++) {
            if (index == subtitles_streams.streams[i]) {
                subtitles_streams.current = i;
            }
        }
        Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_SUBTITLE, index);
        textureSub->setVisibility(Visibility::Visible);
        show_subtitles = true;
    } else {
        subtitles_streams.current = index;
        show_subtitles = false;
        if (textureSub) {
            textureSub->setVisibility(Visibility::Hidden);
        }
    }
}

bool Player::isPlaying() {

    return kit_player != nullptr
           && Kit_GetPlayerState(kit_player) == KIT_PLAYING;
}

bool Player::isPaused() {

    return kit_player != nullptr
           && Kit_GetPlayerState(kit_player) == KIT_PAUSED;
}

bool Player::isStopped() {
    return stopped;
}

bool Player::isFullscreen() {
    return fullscreen;
}

void Player::setFullscreen(bool fs) {

    if (fs == fullscreen) {
        return;
    }

    fullscreen = fs;

    if (!fullscreen) {
        tweenPosition->play(TweenDirection::Backward);
        tweenScale->play(TweenDirection::Backward);
        if (texture) {
            texture->showGradients();
        }
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
    } else {
        if (texture) {
            texture->hideGradients();
        }
        tweenPosition->play(TweenDirection::Forward);
        tweenScale->play(TweenDirection::Forward);
        main->getFiler()->setVisibility(Visibility::Hidden, true);
    }
}

void Player::pause() {

    if (isPlaying()) {
        Kit_PlayerPause(kit_player);
    }

    setCpuClock(CpuClock::Min);
}

void Player::resume() {

    if (isPaused()) {
        Kit_PlayerPlay(kit_player);
    }

    setCpuClock(CpuClock::Max);
}

void Player::stop() {

    if (!stopped) {

        setCpuClock(CpuClock::Min);

        /// Kit
        if (kit_player) {
            Kit_PlayerStop(kit_player);
            Kit_ClosePlayer(kit_player);
            kit_player = nullptr;
        }
        if (source) {
            Kit_CloseSource(source);
            source = nullptr;
        }
        Kit_Quit();

        /// Audio
        audio->pause(1);
        if (menuAudioStreams) {
            delete (menuAudioStreams);
            menuAudioStreams = nullptr;
        }


        /// Video
        if (texture) {
            delete (texture);
            texture = nullptr;
        }
        if (menuVideoStreams) {
            delete (menuVideoStreams);
            menuVideoStreams = nullptr;
        }

        /// Subtitles
        if (textureSub) {
            delete (textureSub);
            textureSub = nullptr;
        }
        if (menuSubtitlesStreams) {
            delete (menuSubtitlesStreams);
            menuSubtitlesStreams = nullptr;
        }

        video_streams.reset();
        audio_streams.reset();
        subtitles_streams.reset();
        title.clear();
        show_subtitles = false;
    }

    stopped = true;
}

void Player::setCpuClock(const CpuClock &clock) {
#ifdef __SWITCH__
    if (clock == CpuClock::Min) {
        if (SwitchSys::getClock(SwitchSys::Module::Cpu) != SwitchSys::getClockStock(SwitchSys::Module::Cpu)) {
            int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
            SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Stock);
            printf("restoring cpu speed (old: %i, new: %i)\n",
                   clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
        }
    } else {
        //if (playerInfo.video.output.width > 1280 || playerInfo.video.output.height > 720) {
        int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
        SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Max);
        printf("setting max cpu speed (old: %i, new: %i)\n",
               clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
        //}
    }
#endif
}

Main *Player::getMain() {
    return main;
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

Player::Stream *Player::getVideoStreams() {
    return &video_streams;
}

Player::Stream *Player::getAudioStreams() {
    return &audio_streams;
}

Player::Stream *Player::getSubtitlesStreams() {
    return &subtitles_streams;
}

Kit_Player *Player::getKitPlayer() {
    return kit_player;
}

const std::string &Player::getTitle() const {
    return title;
}

PlayerOSD *Player::getOSD() {
    return osd;
}

bool Player::isLoading() {
    return loading;
}
