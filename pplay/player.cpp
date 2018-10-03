//
// Created by cpasjuste on 03/10/18.
//

#include <sstream>
#include <iomanip>

#include "main.h"

using namespace c2d;

Player::Player(Main *_main) : Rectangle(_main->getRenderer()->getSize()) {

    main = _main;
    setFillColor(Color::Transparent);

}

bool Player::load(const c2d::Io::File &file) {

    // init Kit library
    int err = Kit_Init(KIT_INIT_ASS);
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
    // make sure there is video in the file to play first.
    if (Kit_GetPlayerVideoStream(player) == -1) {
        printf("file contains no video!\n");
        stop();
        return false;
    }

    printf("Video(%s, %s): %i x %i , Audio(%s): %i hz\n",
           playerInfo.video.codec.name,
           SDL_GetPixelFormatName(playerInfo.video.output.format),
           playerInfo.video.output.width, playerInfo.video.output.height,
           playerInfo.audio.codec.name,
           playerInfo.audio.output.samplerate);

    // init audio
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

    // init video texture
    texture = new C2DTexture(
            {playerInfo.video.output.width, playerInfo.video.output.height}, C2D_TEXTURE_FMT_ABGR8);
    texture->setFiltering(C2D_TEXTURE_FILTER_LINEAR);
    add(texture);

    // start playback
    Kit_PlayerPlay(player);

    return true;
}

void Player::run() {

    while (true) {

        // handle input
        unsigned int keys = main->getInput()->update()[0].state;

        if (keys > 0) {

            if (((keys & c2d::Input::Key::KEY_START)
                 && (keys & c2d::Input::Key::KEY_COIN))) {
                // TODO: menu ?
            }

            if (keys & EV_QUIT) { // SDL2 quit event
                break;
            }

            if (keys & Input::Key::KEY_FIRE2) {
                // TODO: ask confirmation to exit
                break;
            }
        }

        /// process audio/video
        if (Kit_GetPlayerState(player) == KIT_STOPPED) {
            printf("STOPPED\n");
            break;
        }
        if (Kit_GetPlayerState(player) == KIT_PAUSED) {
            printf("PAUSED\n");
        }
        if (Kit_GetPlayerState(player) == KIT_CLOSED) {
            printf("CLOSED\n");
            break;
        }

        /// audio
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

        /// video
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
        texture->setOriginCenter();
        texture->setPosition(getSize().x / 2.0f, getSize().y / 2.0f);
        texture->setScale(scale);

        /// render
        main->getRenderer()->flip();
    }

    stop();
}

void Player::pause() {

    if (player) {
        Kit_PlayerPause(player);
    }
}

void Player::resume() {
    if (player) {
        Kit_PlayerPlay(player);
    }
}

void Player::stop() {

    // Kit
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

    // Audio
    if (audioDeviceID) {
        SDL_CloseAudioDevice(audioDeviceID);
        audioDeviceID = 0;
    }

    // Video
    if (texture) {
        delete (texture);
        texture = nullptr;
    }
}

Player::~Player() {

}

// utils
#if 0
static std::string formatTime(double seconds) {

    int h((int) seconds / 3600);
    int min((int) seconds / 60 - h * 60);
    int sec((int) seconds - (h * 60 + min) * 60);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":";
    oss << std::setfill('0') << std::setw(2) << min << ":";
    oss << std::setfill('0') << std::setw(2) << sec;

    return oss.str();
}
#endif
