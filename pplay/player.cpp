//
// Created by cpasjuste on 29/09/18.
//

#include "c2dui.h"
#include "player.h"
#include "kitchensink/kitchensink.h"

#define AUDIOBUFFER_SIZE (1024 * 64)
#define ATLAS_WIDTH 4096
#define ATLAS_HEIGHT 4096
#define ATLAS_MAX 1024

using namespace c2d;
using namespace c2dui;

static UIMain *uiMain = nullptr;

SDL_Window *window = NULL;
//SDL_Renderer *renderer = NULL;
Kit_Source *src = NULL;
Kit_Player *player = NULL;
//SDL_AudioSpec wanted_spec, audio_spec;
//SDL_AudioDeviceID audio_dev;
SDL_Texture *video_tex;

Player::Player(UIMain *ui) : UIEmu(ui) {

    printf("Player()\n");
    uiMain = getUi();
}

int Player::run(RomList::Rom *rom) {

    int err = Kit_Init(KIT_INIT_ASS);
    if (err != 0) {
        fprintf(stderr, "Unable to initialize Kitchensink: %s", Kit_GetError());
        return 1;
    }

    // Open up the sourcefile.
    // This can be a local file, network url, ...
    std::string file = uiMain->getConfig()->getHomePath()->c_str();
    file += rom->path;
    src = Kit_CreateSourceFromUrl(file.c_str());
    if (src == NULL) {
        fprintf(stderr, "Unable to load file '%s': %s\n", file.c_str(), Kit_GetError());
        return 1;
    }

    // Create the player. Pick best video, audio and subtitle streams, and set subtitle
    // rendering resolution to screen resolution.
    player = Kit_CreatePlayer(
            src,
            Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
            Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
            Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
            (int) uiMain->getRenderer()->getSize().x, (int) uiMain->getRenderer()->getSize().y);
    if (player == NULL) {
        fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
        return 1;
    }

    // Print some information
    Kit_PlayerInfo pinfo;
    Kit_GetPlayerInfo(player, &pinfo);
    // Make sure there is video in the file to play first.
    if (Kit_GetPlayerVideoStream(player) == -1) {
        fprintf(stderr, "File contains no video!\n");
        return 1;
    }

    addAudio(pinfo.audio.output.samplerate, pinfo.video.output.samplerate);
    //addVideo(uiMain, nullptr, nullptr, Vector2f(pinfo.video.output.width, pinfo.video.output.height));

    video_tex = SDL_CreateTexture(
            renderer,
            pinfo.video.output.format,
            SDL_TEXTUREACCESS_STATIC,
            pinfo.video.output.width,
            pinfo.video.output.height);
    if (video_tex == NULL) {
        fprintf(stderr, "Error while attempting to create a video texture\n");
        return 1;
    }

    // Start playback
    Kit_PlayerPlay(player);

    return 0;
}

void Player::stop() {

}

int Player::update() {

    // fps
    int showFps = getUi()->getConfig()->getValue(Option::Index::ROM_SHOW_FPS, true);
    getFpsText()->setVisibility(showFps ? Visible : Hidden);
    if (showFps) {
        sprintf(getFpsString(), "FPS: %.2g/%2d", getUi()->getRenderer()->getFps(), 60);
        getFpsText()->setString(getFpsString());
    }

    c2d::Input::Player *players = getUi()->getInput()->update();

    // look for player 1 menu combo
    if (((players[0].state & c2d::Input::Key::KEY_START) && (players[0].state & c2d::Input::Key::KEY_COIN))) {
        pause();
        return UI_KEY_SHOW_MEMU_ROM;
    } else if (((players[0].state & c2d::Input::Key::KEY_START) && (players[0].state & c2d::Input::Key::KEY_FIRE5))
               || ((players[0].state & c2d::Input::Key::KEY_COIN) && (players[0].state & c2d::Input::Key::KEY_FIRE5))
               || ((players[0].state & c2d::Input::Key::KEY_START) && (players[0].state & c2d::Input::Key::KEY_FIRE6))
               || ((players[0].state & c2d::Input::Key::KEY_COIN) && (players[0].state & c2d::Input::Key::KEY_FIRE6))) {
        pause();
        return UI_KEY_SHOW_MEMU_ROM;
    }

    // look for window resize event
    if (players[0].state & EV_RESIZE) {
        // useful for sdl resize event
        getVideo()->updateScaling();
    }


    if (!isPaused()) {

        // audio
        Kit_GetPlayerAudioData(
                player, (unsigned char *) getAudio()->getBuffer(), AUDIOBUFFER_SIZE);
        getAudio()->play();

        // video
        Kit_GetPlayerVideoData(player, video_tex);
        SDL_RenderCopy(renderer, video_tex, NULL, NULL);
    }

    getUi()->getRenderer()->flip(false);

    return 0;

}
