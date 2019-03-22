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

using namespace c2d;

static void *get_proc_address_mpv(void *fn_ctx, const char *name) {
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

    mpv_set_option_string(mpv.handle, "terminal", "yes");
    mpv_set_option_string(mpv.handle, "msg-level", "all=v");
    mpv_set_option_string(mpv.handle, "vd-lavc-threads", "4");
    mpv_set_option_string(mpv.handle, "vd-lavc-dr", "yes");
    mpv_set_option_string(mpv.handle, "vd-lavc-fast", "yes");

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

    texture = new VideoTexture(main->getSize());
    add(texture);

    osd = new PlayerOSD(main);
    add(osd);
}

bool Player::load(const MediaFile &file) {

    stop();
    stopped = false;
    loading = true;

#if 0
    // default buffer, "Low"
    Kit_SetHint(KIT_HINT_VIDEO_BUFFER_FRAMES, 3);
    Kit_SetHint(KIT_HINT_AUDIO_BUFFER_FRAMES, 64);
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
#endif

    title = file.name;

    // get media config (info)
    std::string hash = std::to_string(std::hash<std::string>()(file.path));
    std::string cfgPath = main->getIo()->getDataWritePath() + "cache/" + hash + ".cfg";
    config = new MediaConfig(cfgPath);

    if (!file.media.videos.empty()) {
        if (config->getStream(OPT_STREAM_VID) > -1) {
            //video_streams.setCurrent(config->getStream(OPT_STREAM_VID));
        } else {
            //config->setStream(OPT_STREAM_VID, video_streams.getCurrent());
        }
    }

    if (!file.media.audios.empty()) {
        if (config->getStream(OPT_STREAM_AUD) > -1) {
            //audio_streams.setCurrent(config->getStream(OPT_STREAM_AUD));
        } else {
            //config->setStream(OPT_STREAM_AUD, audio_streams.getCurrent());
        }
    }

    if (!file.media.subtitles.empty()) {
        if (config->getStream(OPT_STREAM_SUB) > -1) {
            //subtitles_streams.setCurrent(config->getStream(OPT_STREAM_SUB));
            show_subtitles = true;
        } else {
            //config->setStream(OPT_STREAM_AUD, subtitles_streams.getCurrent());
            show_subtitles = false;
        }
    }

    // we should be good to go, set cpu speed if needed
    setCpuClock(CpuClock::Max);

#if 0
    // get some information
    Kit_GetPlayerInfo(kit_player, &playerInfo);
    printf("Player::load: Video(%s, %s): %i x %i , Audio(%s): %i @ %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.format,
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
        audio = new C2DAudio(playerInfo.audio.output.samplerate, fps);
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
        add(textureSub);
        if (!show_subtitles) {
            textureSub->setVisibility(Visibility::Hidden);
        }
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
#endif

    /*
    if (!file.media.videos.empty()) {
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
    */

    const char *cmd[] = {"loadfile", file.path.c_str(), nullptr};
    int res = mpv_command(mpv.handle, cmd);
    if (res != 0) {
        main->getStatus()->show("Error...", "Could not play file:\n\n", mpv_error_string(res));
        printf("Player::load: unable to create player: %s\n", mpv_error_string(res));
        stop();
        return false;
    }
    pause();

    return true;
}

void Player::onLoadedEvent() {

    // load subtitles tracks
    mpv_node node;
    mpv_get_property(mpv.handle, "track-list", MPV_FORMAT_NODE, &node);
    if (node.format == MPV_FORMAT_NODE_ARRAY) {
        for (int i = 0; i < node.u.list->num; i++) {
            if (node.u.list->values[i].format == MPV_FORMAT_NODE_MAP) {
                for (int n = 0; n < node.u.list->values[i].u.list->num; n++) {
                    if (strcmp(node.u.list->values[i].u.list->keys[n], "type") == 0) {
                        if (node.u.list->values[i].u.list->values[n].format == MPV_FORMAT_STRING) {
                            //track.type = node.u.list->values[i].u.list->values[n].u.string;
                            printf("TRACK TYPE: %s\n", node.u.list->values[i].u.list->values[n].u.string);
                        }
                    }
                }
            }
        }
    }

    if (config->getPosition() > 0) {
        std::string msg = "Resume playback at " + pplay::Utility::formatTime(config->getPosition()) + " ?";
        if (main->getMessageBox()->show("RESUME", msg, "RESUME", "RESTART") == MessageBox::LEFT) {
            seek(config->getPosition());
        }
    }

    resume();
    loading = false;

#ifdef __SWITCH__
    appletSetMediaPlaybackState(true);
#endif

    setVisibility(Visibility::Visible);
}

void Player::onUpdate() {

    // handle mpv events
    if (mpv.available) {
        mpv_event *event = mpv_wait_event(mpv.handle, 0);
        if (event) {
            switch (event->event_id) {
                case MPV_EVENT_FILE_LOADED:
                    printf("MPV_EVENT_FILE_LOADED\n");
                    onLoadedEvent();
                    break;
                case MPV_EVENT_NONE:
                    break;
            }
        }
    }
}

void Player::onDraw(c2d::Transform &transform, bool draw) {

    if (loading || isStopped()) {
        Rectangle::onDraw(transform, draw);
        return;
    }

    /*
    if (!isPlaying() && !isPaused()) {
        stop();
        if (isFullscreen()) {
            setFullscreen(false);
        }
        Rectangle::onDraw(transform, draw);
        return;
    }
    */

    //////////////////
    /// step ffmpeg
    //////////////////
    if (mpv.available) {
        int flip_y{0};
        mpv_opengl_fbo fbo{
                .fbo = texture->fbo,
                .w = (int) texture->getSize().x, .h = (int) texture->getSize().y,
                .internal_format = GL_RGBA8};
        mpv_render_param r_params[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
                {MPV_RENDER_PARAM_FLIP_Y,     &flip_y},
                {MPV_RENDER_PARAM_INVALID,    nullptr}
        };
        mpv_render_context_render(mpv.ctx, r_params);
    }

    Rectangle::onDraw(transform, draw);
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
#if 0
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
#endif
}

void Player::setAudioStream(int streamId) {
#if 0
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
#endif
}

void Player::setSubtitleStream(int streamId) {
#if 0
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
#endif
}

int Player::seek(double seek_position) {

    std::string cmd = "no-osd seek " + std::to_string(seek_position);
    mpv_command_string(mpv.handle, cmd.c_str());

#if 0
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
    osd->reset();
#endif
    return 0;
}

bool Player::isPlaying() {

    int res = -1;

    if (mpv.available) {
        mpv_get_property(mpv.handle, "core-idle", MPV_FORMAT_FLAG, &res);
    }

    return res == 0;
}

bool Player::isPaused() {

    int res = -1;

    if (mpv.available) {
        mpv_get_property(mpv.handle, "pause", MPV_FORMAT_FLAG, &res);
    }

    return res == 1;
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
        main->getTitle()->setVisibility(Visibility::Visible, true);
        main->getStatusBar()->setVisibility(Visibility::Visible, true);
    } else {
        if (texture) {
            texture->hideGradients();
        }
        tweenPosition->play(TweenDirection::Forward);
        tweenScale->play(TweenDirection::Forward);
        main->getFiler()->setVisibility(Visibility::Hidden, true);
        main->getTitle()->setVisibility(Visibility::Hidden, true);
        main->getStatusBar()->setVisibility(Visibility::Hidden, true);
    }
}

void Player::pause() {

    if (mpv.available) {
        mpv_command_string(mpv.handle, "set pause yes");
    }

    setCpuClock(CpuClock::Min);
}

void Player::resume() {

    if (mpv.available) {
        mpv_command_string(mpv.handle, "set pause no");
    }

    setCpuClock(CpuClock::Max);
}

void Player::stop() {

    printf("Player::stop: stopped = %i\n", stopped);

    if (!stopped) {

        if (mpv.available) {
            mpv_command_string(mpv.handle, "stop");
        }

        // save position in stream
        if (config) {
            long position = getPlaybackPosition();
            if (position > 5) {
                config->setPosition((int) position - 5);
            } else {
                config->setPosition(0);
            }
            delete (config);
            config = nullptr;
        }

        show_subtitles = false;
        title.clear();
        osd->reset();

#if 0
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

        video_streams.reset();
        audio_streams.reset();
        subtitles_streams.reset();
#endif
        setCpuClock(CpuClock::Min);
#ifdef __SWITCH__
        appletSetMediaPlaybackState(false);
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

/*
Player::Stream *Player::getVideoStreams() {
    return &video_streams;
}

Player::Stream *Player::getAudioStreams() {
    return &audio_streams;
}

Player::Stream *Player::getSubtitlesStreams() {
    return &subtitles_streams;
}
*/

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

