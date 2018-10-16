//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"
#include "player.h"

using namespace c2d;

Player::Player(Main *_main) : C2DRectangle(_main->getRenderer()->getSize()) {

    main = _main;
    setFillColor(Color::Transparent);

    osd = new PlayerOSD(this);
    add(osd);
}

bool Player::load(const c2d::Io::File &file) {

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
    source = Kit_CreateSourceFromUrl(file.path.c_str());
    if (!source) {
        printf("unable to load file '%s': %s\n", file.path.c_str(), Kit_GetError());
        stop();
        return false;
    }

    // create the player. pick best video, audio and subtitle streams, and set subtitle
    // rendering resolution to screen resolution.
    player = Kit_CreatePlayer(
            source,
            Kit_GetBestSourceStream(source, KIT_STREAMTYPE_VIDEO),
            Kit_GetBestSourceStream(source, KIT_STREAMTYPE_AUDIO),
            -1, //Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE), // TODO: subtitles
            (int) getSize().x, (int) getSize().y);
    if (!player) {
        printf("unable to create player: %s\n", Kit_GetError());
        stop();
        return false;
    }

    // get some information;
    Kit_GetPlayerInfo(player, &playerInfo);
    has_video = Kit_GetPlayerVideoStream(player) != -1;
    has_audio = Kit_GetPlayerAudioStream(player) != -1;
    if (!has_audio && !has_video) {
        printf("no usable audio or video stream found: %s\n", Kit_GetError());
    }

    printf("Video(%s, %s): %i x %i , Audio(%s): %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.samplerate);

    // init audio
    if (has_audio) {
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

    if (has_video) {
        // init video texture
        texture = new C2DTexture(
                {playerInfo.video.output.width, playerInfo.video.output.height}, Texture::Format::RGBA8);
        texture->setFilter(Texture::Filter::Linear);
        add(texture);
    }

    setVisibility(Visibility::Visible);
    //osd->setVisibility(Visibility::Visible);
    osd->setLayer(100);

    // start playback
    Kit_PlayerPlay(player);

    return true;
}

void Player::step(unsigned int keys) {

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
            osd->setVisibility(Visibility::Visible);
        }
    } else if (keys & Input::Key::KEY_FIRE2) {
        if (osd->isVisible()) {
            osd->setVisibility(Visibility::Hidden);
        } else {
            osd->setVisibility(Visibility::Visible);
        }
    } else if (!paused) {
        if (keys & c2d::Input::Key::KEY_LEFT) {
            osd->setVisibility(Visibility::Visible);
            Kit_PlayerSeek(player, position - 60.0);
        } else if (keys & c2d::Input::Key::KEY_RIGHT) {
            osd->setVisibility(Visibility::Visible);
            if (position + 60 < duration) {
                Kit_PlayerSeek(player, position + 60.0);
            }
        } else if (keys & c2d::Input::Key::KEY_UP) {
            osd->setVisibility(Visibility::Visible);
            if (position + (60.0 * 10.0) < duration) {
                Kit_PlayerSeek(player, position + (60.0 * 10.0));
            }
        } else if (keys & c2d::Input::Key::KEY_DOWN) {
            osd->setVisibility(Visibility::Visible);
            Kit_PlayerSeek(player, position - (60.0 * 10.0));
        }
    }

    //////////////////
    /// step ffmpeg
    //////////////////
    /// audio
    if (has_audio) {
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
            // If we now have data, start playback (again)
            if (SDL_GetQueuedAudioSize(audioDeviceID) > 0) {
                SDL_PauseAudioDevice(audioDeviceID, 0);
            }
        }
    }

    /// video
    if (has_video) {
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
}

bool Player::isPlaying() {

    return player != nullptr
           && (Kit_GetPlayerState(player) == KIT_PLAYING
               || Kit_GetPlayerState(player) == KIT_PAUSED);
}

bool Player::isMaximized() {
    return fullscreen;
}

void Player::setMaximized(bool maximize) {
    fullscreen = maximize;
}

void Player::pause() {

    if (player && !paused) {
        Kit_PlayerPause(player);
    }

    paused = true;
}

void Player::resume() {

    if (player && paused) {
        Kit_PlayerPlay(player);
    }

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

    has_video = false;
    has_audio = false;
    paused = false;

    osd->setVisibility(Visibility::Hidden);
    setVisibility(Visibility::Hidden);
}

Main *Player::getMain() {
    return main;
}

Player::~Player() {

}

