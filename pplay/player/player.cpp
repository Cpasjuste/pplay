//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"
#include "player.h"
#include "player_osd.h"
#include "utility.h"
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

    setVisibility(Visibility::Hidden);
}

bool Player::load(const MediaFile &file) {

    stop();
    stopped = false;

    // avformat_network_init/deinit handled in media info thread
    int err = Kit_Init(KIT_INIT_ASS);
    if (err != 0) {
        printf("Player::load: unable to initialize Kitchensink: %s\n", Kit_GetError());
        stop();
        return false;
    }

    // default buffer, "Medium"
    Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 64);
    Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 512);
    Kit_SetHint(KIT_HINT_SUBTITLE_BUFFER_FRAMES, 64);
    std::string buffering = main->getConfig()->getOption(OPT_BUFFER)->getString();
    if (Utility::toLower(buffering) == "low") {
        Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 3);
        Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 64);
    } else if (Utility::toLower(buffering) == "medium") {
        Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 64);
        Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 512);
    } else if (Utility::toLower(buffering) == "high") {
        Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 128);
        Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 1024);
    } else if (Utility::toLower(buffering) == "veryhigh") {
        Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 256);
        Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 2048);
    }
    printf("Player::load: buffering = %s (video=%i, audio=%i)\n", buffering.c_str(),
           Kit_GetHint(KIT_HINT_VIDEO_BUFFER_FRAMES), Kit_GetHint(KIT_HINT_AUDIO_BUFFER_FRAMES));

    // open source file
    printf("Player::load: %s\n", file.path.c_str());
    source = Kit_CreateSourceFromUrl(file.path.c_str());
    if (!source) {
        main->getStatus()->show("Error...", Kit_GetError());
        printf("Player::load: unable to load '%s': %s\n", file.path.c_str(), Kit_GetError());
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
        printf("Player::load: no usable audio or video stream found: %s\n", Kit_GetError());
        main->getStatus()->show("Error...", "no video or audio stream found");
        stop();
        return false;
    }

    // set default font
    Kit_LibraryState *state = Kit_GetLibraryState();
    snprintf(state->subtitle_font_path, 511, "%sskin/font.ttf",
             getMain()->getIo()->getDataReadPath().c_str());

    title = file.name;
    // get media config (info)
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    std::string cfgPath = main->getIo()->getDataWritePath() + "cache/" + hash + ".cfg";
    config = new MediaConfig(cfgPath);
    if (video_streams.size > 0) {
        if (config->getStream(OPT_STREAM_VID) > -1) {
            video_streams.setCurrent(config->getStream(OPT_STREAM_VID));
        } else {
            config->setStream(OPT_STREAM_VID, video_streams.getCurrent());
        }
    }
    if (audio_streams.size > 0) {
        if (config->getStream(OPT_STREAM_AUD) > -1) {
            audio_streams.setCurrent(config->getStream(OPT_STREAM_AUD));
        } else {
            config->setStream(OPT_STREAM_AUD, audio_streams.getCurrent());
        }
    }
    if (subtitles_streams.size > 0) {
        if (config->getStream(OPT_STREAM_SUB) > -1) {
            subtitles_streams.setCurrent(config->getStream(OPT_STREAM_SUB));
            show_subtitles = true;
        } else {
            // we need to init Kit_Player with a subtitle track, else it won't work
            subtitles_streams.current = 0;
        }
    }

    // create the player
    kit_player = Kit_CreatePlayer(
            source,
            video_streams.getCurrent(),
            audio_streams.getCurrent(),
            subtitles_streams.getCurrent(),
            (int) getSize().x, (int) getSize().y);
    if (!kit_player) {
        main->getStatus()->show("Error...", Kit_GetError());
        printf("Player::load: unable to create player: %s\n", Kit_GetError());
        stop();
        return false;
    }

    // we should be good to go, set cpu speed if needed
    setCpuClock(CpuClock::Max);

    // get some information
    Kit_GetPlayerInfo(kit_player, &playerInfo);
    printf("Player::load: Video(%s, %s): %i x %i , Audio(%s): %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.samplerate);

    // get a decoder handle for audio fps and buffering status
    auto *decoder = (Kit_Decoder *) kit_player->decoders[0];
    if (!decoder) {
        // try with audio decoder
        decoder = (Kit_Decoder *) kit_player->decoders[1];
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
                main, main->getMenuVideo()->getGlobalBounds(), "VIDEO", items, MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setSelection(MENU_VIDEO_TYPE_VID);
        menuVideoStreams->setVisibility(Visibility::Hidden, false);
        menuVideoStreams->setLayer(3);
        add(menuVideoStreams);
    }

    if (audio_streams.size > 0) {
        // set audio framerate based on video fps
        float fps = 24;
        if (decoder) {
            if (video_streams.getCurrent() > -1) {
                fps = (float) av_q2d(decoder->format_ctx->streams[video_streams.getCurrent()]->r_frame_rate);
            }
        }
        // TODO: < 48000 not working fine on switch, check sdl2/libsamplerate
        //audio = new C2DAudio(playerInfo.audio.output.samplerate, fps);
        audio = new C2DAudio(48000, fps);
        // audios menu options
        std::vector<MenuItem> items;
        for (auto &stream : file.media.audios) {
            items.emplace_back("Lang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuAudioStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "AUDIO", items, MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setSelection(MENU_VIDEO_TYPE_AUD);
        menuAudioStreams->setVisibility(Visibility::Hidden, false);
        menuAudioStreams->setLayer(3);
        add(menuAudioStreams);
    }

    if (subtitles_streams.size > 0) {
        textureSub = new SubtitlesTexture();
        textureSub->setFilter(Texture::Filter::Point);
        void *buf;
        textureSub->lock(nullptr, &buf, nullptr);
        memset(buf, 0, 1024 * 1024 * 4);
        textureSub->unlock();
        if (!show_subtitles) {
            textureSub->setVisibility(Visibility::Hidden);
        }
        add(textureSub);
        // subtitles menu options
        std::vector<MenuItem> items;
        items.emplace_back("None", "", MenuItem::Position::Top, -1);
        for (auto &stream : file.media.subtitles) {
            items.emplace_back("Lang: " + stream.language, "", MenuItem::Position::Top, stream.id);
        }
        menuSubtitlesStreams = new MenuVideoSubmenu(
                main, main->getMenuVideo()->getGlobalBounds(), "SUBTITLES", items, MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setSelection(MENU_VIDEO_TYPE_SUB);
        menuSubtitlesStreams->setVisibility(Visibility::Hidden, false);
        menuSubtitlesStreams->setLayer(3);
        add(menuSubtitlesStreams);
    }

    // preload/cache some frames, resume playback if needed
    loading = true;
    if (config->getPosition() > 10) {
        std::string msg = "Resume playback at " + pplay::Utility::formatTime(config->getPosition()) + " ?";
        if (main->getMessageBox()->show("RESUME", msg, "RESUME", "RESTART") == MessageBox::LEFT) {
            Kit_SetClockSync(kit_player);
            if (seek(config->getPosition() - 10) != 0) {
                play();
            }
        } else {
            play();
        }
    } else {
        play();
    }
    loading = false;

#ifdef __SWITCH__
    appletSetMediaPlaybackStateForApplication(true);
#endif

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

#ifndef NDEBUG
    if (kit_player) {
        std::string v_buf_in = "video: in = ", v_buf_out = "out = ", a_buf_in = "audio: in = ", a_buf_out = "out: ";
        auto *v_dec = (Kit_Decoder *) kit_player->decoders[0];
        if (v_dec) {
            v_buf_in += std::to_string(Kit_GetBufferBufferedSize(v_dec->buffer[KIT_DEC_BUF_IN]));
            v_buf_out += std::to_string(Kit_GetBufferBufferedSize(v_dec->buffer[KIT_DEC_BUF_OUT]));
        }
        auto *a_dec = (Kit_Decoder *) kit_player->decoders[1];
        if (a_dec) {
            a_buf_in += std::to_string(Kit_GetBufferBufferedSize(a_dec->buffer[KIT_DEC_BUF_IN]));
            a_buf_out += std::to_string(Kit_GetBufferBufferedSize(a_dec->buffer[KIT_DEC_BUF_OUT]));
        }
        main->debugText->setString(v_buf_in + "\t" + v_buf_out + "\n" + a_buf_in + "\t" + a_buf_out);
    }
#endif

    if (loading || isStopped()) {
        Rectangle::onDraw(transform);
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
    if (audio && audio_streams.size > 0) {
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
    if (texture && video_streams.size > 0) {
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

void Player::setVideoStream(int streamId) {

    if (streamId == video_streams.getCurrent()) {
        main->getStatus()->show("Info...", "Selected video stream already set");
        return;
    }

    if (texture && streamId > -1) {
        video_streams.setCurrent(streamId);
        texture->setVisibility(Visibility::Visible);
        if (Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_VIDEO, streamId) == 0) {
            config->setStream(OPT_STREAM_VID, streamId);
        }
    } else {
        video_streams.current = -1;
        if (texture) {
            texture->setVisibility(Visibility::Hidden);
        }
    }
}

void Player::setAudioStream(int streamId) {

    if (streamId == audio_streams.getCurrent()) {
        main->getStatus()->show("Info...", "Selected audio stream already set");
        return;
    }

    if (audio && streamId > -1) {
        audio_streams.setCurrent(streamId);
        if (Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_AUDIO, streamId) == 0) {
            config->setStream(OPT_STREAM_AUD, streamId);
        }
    } else {
        audio_streams.current = -1;
    }
}

void Player::setSubtitleStream(int streamId) {

    if (streamId == subtitles_streams.getCurrent()) {
        if (streamId > -1) {
            if (kit_player->decoders[2] && textureSub) {
                config->setStream(OPT_STREAM_SUB, streamId);
                textureSub->setVisibility(Visibility::Visible);
                show_subtitles = true;
            } else {
                config->setStream(OPT_STREAM_SUB, -1);
                main->getStatus()->show("Info...", "Subtitle format not supported");
            }
        } else {
            main->getStatus()->show("Info...", "Selected subtitles stream already set");
        }
        return;
    }

    if (textureSub && streamId > -1) {
        subtitles_streams.setCurrent(streamId);
        if (Kit_SetPlayerStream(kit_player, KIT_STREAMTYPE_SUBTITLE, streamId) == 0) {
            config->setStream(OPT_STREAM_SUB, streamId);
            textureSub->setVisibility(Visibility::Visible);
            show_subtitles = true;
        } else {
            config->setStream(OPT_STREAM_SUB, -1);
            subtitles_streams.current = -1;
            show_subtitles = false;
            textureSub->setVisibility(Visibility::Hidden);
            main->getStatus()->show("Info...", "Subtitle format not supported");
        }
    } else {
        config->setStream(OPT_STREAM_SUB, -1);
        subtitles_streams.current = -1;
        show_subtitles = false;
        if (textureSub) {
            textureSub->setVisibility(Visibility::Hidden);
        }
    }
}

void Player::play() {

    int flip = 0;
    auto *decoder = (Kit_Decoder *) kit_player->decoders[0];
    if (!decoder) {
        decoder = (Kit_Decoder *) kit_player->decoders[1];
    }

    loading = true;

    while (Kit_PlayerPlay(kit_player) > 0) {
        if (flip % 30 == 0) {
            if (decoder) {
                int progress = Kit_GetBufferBufferedSize(decoder->buffer[KIT_DEC_BUF_OUT]);
                std::string msg = "Loading... " + title + "... " + std::to_string(progress) + "%";
                main->getStatus()->show("Please Wait...", msg);
            }
            main->flip();
        }
        flip++;
    }

    loading = false;
}

int Player::seek(double seek_position) {

    int flip = 0;
    double position = Kit_GetPlayerPosition(kit_player);
    auto *decoder = (Kit_Decoder *) kit_player->decoders[0];
    if (!decoder) {
        decoder = (Kit_Decoder *) kit_player->decoders[1];
    }

    loading = true;

    if (Kit_PlayerSeekStart(kit_player, position, seek_position) != 0) {
        main->getStatus()->show("Error...", Kit_GetError());
        loading = false;
        return -1;
    }

    std::string msg = "Seeking... " + title + "... 0%";
    main->getStatus()->show("Please Wait...", msg, false, true);

    while (Kit_PlayerSeekEnd(kit_player, position, seek_position) > 0) {
        if (flip % 30 == 0) {
            if (decoder) {
                int progress = Kit_GetBufferBufferedSize(decoder->buffer[KIT_DEC_BUF_OUT]);
                msg = "Seeking... " + title + "... " + std::to_string(progress) + "%";
                main->getStatus()->show("Please Wait...", msg);
            }
            main->flip();
        }
        flip++;
    }

    kit_player->state = KIT_PLAYING;
    loading = false;

    return 0;
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

    printf("Player::stop: stopped = %i\n", stopped);

    if (!stopped) {

        /// Kit
        if (kit_player) {
            // save position in stream
            if (config) {
                if (Kit_GetPlayerPosition(kit_player) < Kit_GetPlayerDuration(kit_player) - 5) {
                    config->setPosition((float) Kit_GetPlayerPosition(kit_player));
                } else {
                    config->setPosition(0);
                }
            }
            Kit_ClosePlayer(kit_player);
            kit_player = nullptr;
        }
        if (source) {
            Kit_CloseSource(source);
            source = nullptr;
        }
        Kit_Quit();

        /// Audio
        if (audio) {
            audio->pause(1);
            delete (audio);
            audio = nullptr;
        }
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

        /// media configuration
        if (config) {
            delete (config);
            config = nullptr;
        }

        video_streams.reset();
        audio_streams.reset();
        subtitles_streams.reset();
        show_subtitles = false;
        title.clear();

        setCpuClock(CpuClock::Min);

#ifdef __SWITCH__
        appletSetMediaPlaybackStateForApplication(false);
#endif
    }

    stopped = true;
}

void Player::setCpuClock(const CpuClock &clock) {
#ifdef __SWITCH__
    if (main->getConfig()->getOption(OPT_CPU_BOOST)->getString() == "Enabled") {
        if (clock == CpuClock::Min) {
            if (SwitchSys::getClock(SwitchSys::Module::Cpu) != SwitchSys::getClockStock(SwitchSys::Module::Cpu)) {
                int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
                SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Stock);
                printf("restoring cpu speed (old: %i, new: %i)\n",
                       clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
            }
        } else {
            int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
            SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Max);
            printf("setting max cpu speed (old: %i, new: %i)\n",
                   clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
        }
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

bool Player::isSubtitlesEnabled() {
    return show_subtitles;
}
