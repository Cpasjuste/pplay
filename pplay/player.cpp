//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"
#include "player.h"

using namespace c2d;

Player::Player(Main *_main) : RectangleShape(_main->getSize()) {

    main = _main;
    setFillColor(Color::Black);
    setOutlineColor(COLOR_ORANGE);
    setOutlineThickness(4);

    tweenPosition = new TweenPosition(
            {main->getSize().x * 0.55f, main->getSize().y * 0.55f},
            {0, 0}, 0.5f);
    add(tweenPosition);
    tweenScale = new TweenScale({0.4f, 0.4f}, {1, 1}, 0.5f);
    add(tweenScale);

    osd = new PlayerOSD(this);
    add(osd);

    setVisibility(Visibility::Hidden);
}

bool Player::load(c2d::Io::File *file) {

    if (isPlaying()) {
        stop();
    }

    // init Kit library
    int err = Kit_Init(KIT_INIT_NETWORK | KIT_INIT_ASS);
    if (err != 0) {
        printf("unable to initialize Kitchensink: %s\n", Kit_GetError());
        stop();
        return false;
    }

    // open source file
    printf("Player::load: %s\n", file->path.c_str());
    source = Kit_CreateSourceFromUrl(file->path.c_str());
    if (!source) {
        printf("unable to load '%s': %s\n", file->path.c_str(), Kit_GetError());
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
        stop();
        return false;
    }

    // set default font
    Kit_LibraryState *state = Kit_GetLibraryState();
    snprintf(state->subtitle_font_path, 511, "%sLiberationSans-Regular.ttf",
             getMain()->getIo()->getDataPath().c_str());

    // create the player
    player = Kit_CreatePlayer(
            source,
            video_streams.getCurrentStream(),
            audio_streams.getCurrentStream(),
            -1,
            (int) getSize().x, (int) getSize().y);
    if (!player) {
        printf("unable to create player: %s\n", Kit_GetError());
        stop();
        return false;
    }

    // get some information;
    Kit_GetPlayerInfo(player, &playerInfo);
    printf("Video(%s, %s): %i x %i , Audio(%s): %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.samplerate);

    if (audio_streams.size > 0) {
        if (!SDL_WasInit(SDL_INIT_AUDIO)) {
            SDL_InitSubSystem(SDL_INIT_AUDIO);
        }
        SDL_AudioSpec wanted_spec, audio_spec;
        SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
        wanted_spec.freq = playerInfo.audio.output.samplerate;
        wanted_spec.format = (SDL_AudioFormat) playerInfo.audio.output.format;
        wanted_spec.channels = (Uint8) playerInfo.audio.output.channels;
        audioDeviceID = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &audio_spec, 0);
        SDL_PauseAudioDevice(audioDeviceID, 0);
    }

    if (video_streams.size > 0) {
        texture = new C2DTexture(
                {playerInfo.video.output.width, playerInfo.video.output.height}, Texture::Format::RGBA8);
        texture->setFilter(Texture::Filter::Linear);
        add(texture);
    }

    if (subtitles_streams.size > 0) {
        textureSub = new SubtitlesTexture();
        void *buf;
        textureSub->lock(nullptr, &buf, nullptr);
        memset(buf, 0, 1024 * 1024 * 4);
        textureSub->unlock();
        textureSub->setFilter(Texture::Filter::Point);
        add(textureSub);
    }

    setVisibility(Visibility::Visible);
    osd->setLayer(100);

    // start playback
    Kit_PlayerPlay(player);

    setCpuClock(CpuClock::Max);

    return true;
}

void Player::onInput(c2d::Input::Player *players) {

    double position, duration;

    if (!isVisible() || !isPlaying()) {
        stop();
        return;
    }

    position = Kit_GetPlayerPosition(player);
    duration = Kit_GetPlayerDuration(player);
    osd->setProgress((float) duration, (float) position);

    //////////////////
    /// handle inputs
    //////////////////
    unsigned int keys = isFullscreen() ? players[0].state : 0;
    if (keys & Input::Key::KEY_FIRE1) {
        if (osd->isVisible()) {
            if (paused) {
                resume();
                osd->resume();
            } else {
                pause();
                osd->pause();
            }
        } else {
            osd->setVisibility(Visibility::Visible, true);
        }
    } else if (keys & Input::Key::KEY_FIRE2) {
        if (osd->isVisible()) {
            osd->setVisibility(Visibility::Hidden, true);
        } else {
            osd->setVisibility(Visibility::Visible, true);
        }
    } else if (!paused) {
        if (keys & c2d::Input::Key::KEY_LEFT) {
            osd->setVisibility(Visibility::Visible, true);
            Kit_PlayerSeek(player, position - 60.0);
        } else if (keys & c2d::Input::Key::KEY_RIGHT) {
            osd->setVisibility(Visibility::Visible, true);
            if (position + 60 < duration) {
                Kit_PlayerSeek(player, position + 60.0);
            }
        } else if (keys & c2d::Input::Key::KEY_UP) {
            osd->setVisibility(Visibility::Visible, true);
            if (position + (60.0 * 10.0) < duration) {
                Kit_PlayerSeek(player, position + (60.0 * 10.0));
            }
        } else if (keys & c2d::Input::Key::KEY_DOWN) {
            osd->setVisibility(Visibility::Visible, true);
            Kit_PlayerSeek(player, position - (60.0 * 10.0));
        }
    }

    C2DObject::onInput(players);
}

void Player::onDraw(c2d::Transform &transform) {

    if (!isVisible() || !isPlaying()) {
        stop();
        return;
    }

    //////////////////
    /// step ffmpeg
    //////////////////
    /// audio
    if (audio_streams.size > 0) {
        int queued = SDL_GetQueuedAudioSize(audioDeviceID);
        if (queued < AUDIO_BUFFER_SIZE) {
            int need = AUDIO_BUFFER_SIZE - queued;
            while (need > 0) {
                int ret = Kit_GetPlayerAudioData(
                        player, (unsigned char *) audioBuffer, AUDIO_BUFFER_SIZE);
                need -= ret;
                if (ret > 0) {
                    SDL_QueueAudio(audioDeviceID, audioBuffer, (Uint32) ret);
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
        if (Kit_GetPlayerVideoDataRaw(player, video_data)) {
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
        texture->setOrigin(Origin::Center);
        texture->setPosition(getSize().x / 2.0f, getSize().y / 2.0f);
        texture->setScale(scale);
    }

    /// Subtitles
    if (subtitles_streams.size > 0) {
        int count = Kit_GetPlayerSubtitleDataRaw(
                player, textureSub->pixels, textureSub->getRectsSrc(), textureSub->getRectsDst(), ATLAS_MAX);
        textureSub->setRectsCount(count);
        if (count > 0) {
            textureSub->unlock();
        }
    }

    Shape::onDraw(transform);
}

bool Player::isPlaying() {

    return player != nullptr
           && (Kit_GetPlayerState(player) == KIT_PLAYING
               || Kit_GetPlayerState(player) == KIT_PAUSED);
}

bool Player::isFullscreen() {
    return fullscreen;
}

void Player::setFullscreen(bool fs) {
    fullscreen = fs;
}

void Player::pause() {

    if (player && !paused) {
        Kit_PlayerPause(player);
    }

    setCpuClock(CpuClock::Min);

    paused = true;
}

void Player::resume() {

    if (player && paused) {
        Kit_PlayerPlay(player);
    }

    setCpuClock(CpuClock::Max);

    paused = false;
}

void Player::stop() {

    /// Kit
    if (player) {
        Kit_PlayerStop(player);
        Kit_ClosePlayer(player);
        player = nullptr;
    }
    if (source) {
        Kit_CloseSource(source);
        source = nullptr;
    }
    Kit_Quit();

    /// Audio
    if (audioDeviceID) {
        SDL_CloseAudioDevice(audioDeviceID);
        audioDeviceID = 0;
    }

    /// Video
    if (texture) {
        delete (texture);
        texture = nullptr;
    }

    /// Subtitles
    if (textureSub) {
        delete (textureSub);
        textureSub = nullptr;
    }

    video_streams.reset();
    audio_streams.reset();
    subtitles_streams.reset();
    paused = false;

    osd->setVisibility(Visibility::Hidden);
    setVisibility(Visibility::Hidden);
    setFullscreen(false);

    setCpuClock(CpuClock::Min);
}

void Player::setCpuClock(const CpuClock &clock) {
#ifdef __SWITCH__
    if (clock == CpuClock::Min) {
        if (SwitchSys::getClock(SwitchSys::Module::Cpu) != SwitchSys::getClockStock(SwitchSys::Module::Cpu)) {
            int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
            SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Stock);
            printf("restored cpu speed (old: %i, new: %i)\n",
                   clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
        }
    } else {
        if (playerInfo.video.output.width > 1280
            || playerInfo.video.output.height > 720) {
            int clock_old = SwitchSys::getClock(SwitchSys::Module::Cpu);
            SwitchSys::setClock(SwitchSys::Module::Cpu, (int) SwitchSys::CPUClock::Max);
            printf("fhd video spotted, setting max cpu speed (old: %i, new: %i)\n",
                   clock_old, SwitchSys::getClock(SwitchSys::Module::Cpu));
        }
    }
#endif
}

Main *Player::getMain() {
    return main;
}

c2d::TweenPosition *Player::getTweenPosition() {
    return tweenPosition;
}

c2d::TweenScale *Player::getTweenScale() {
    return tweenScale;
}

Player::~Player() {

}

