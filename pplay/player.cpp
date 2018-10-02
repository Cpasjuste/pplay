//
// Created by cpasjuste on 29/09/18.
//

#include <sstream>
#include <iomanip>

#include "c2dui.h"
#include "player.h"
#include "kitchensink/kitchensink.h"

#define __SDL_KITCH__ 1

#ifndef __SDL_KITCH__
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
static AVFormatContext *ctx_format;
static AVCodecContext *ctx_codec;
static AVCodec *codec;
static AVFrame *frame;
static SwsContext *ctx_sws;
static AVStream *vid_stream;
static AVPacket *pkt;
int stream_idx;
#endif

#define AUDIOBUFFER_SIZE (1024 * 64)
#define ATLAS_WIDTH 4096
#define ATLAS_HEIGHT 4096
#define ATLAS_MAX 1024

using namespace c2d;
using namespace c2dui;

static UIMain *uiMain = nullptr;

static Kit_Source *src = nullptr;
static Kit_Player *player = nullptr;
static Kit_PlayerInfo player_info;
static SDL_AudioSpec wanted_spec, audio_spec;
static SDL_AudioDeviceID audio_dev;
static char audiobuf[AUDIOBUFFER_SIZE];

static std::string formatTime(double seconds);

Player::Player(UIMain *ui) : UIEmu(ui) {

    printf("Player()\n");
    uiMain = getUi();
}

int Player::run(RomList::Rom *rom) {

    std::string file = std::string(*getUi()->getConfig()->getRomPath(0) + rom->path);

#ifndef __SDL_KITCH__
    av_log_set_level(AV_LOG_VERBOSE);
    int ret = avformat_open_input(&ctx_format, file.c_str(), NULL, NULL);
    if (ret != 0) {
        printf("Error opening file. Is it a valid video?\n");
        return -1;
    }

    if (avformat_find_stream_info(ctx_format, NULL) < 0) {
        printf("Error finding stream info.\n");
        return -1;
    }

    av_dump_format(ctx_format, 0, file.c_str(), false);

    for (unsigned int i = 0; i < ctx_format->nb_streams; i++) {
        if (ctx_format->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_idx = i;
            vid_stream = ctx_format->streams[i];
            break;
        }
    }
    if (!vid_stream) {
        printf("Error getting video stream.\n");
        return -1;
    }

    codec = avcodec_find_decoder(vid_stream->codecpar->codec_id);
    if (!codec) {
        printf("Error finding a decoder (strange)");
        return -1;
    }

    ctx_codec = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(ctx_codec, vid_stream->codecpar) < 0) {
        printf("Error sending parameters to codec context.");
        return -1;
    }

    if (avcodec_open2(ctx_codec, codec, NULL) < 0) {
        printf("Error opening codec with context.");
        return -1;
    }

    frame = av_frame_alloc();
    pkt = av_packet_alloc();

    C2DUIVideo *video = new C2DUIVideo(
            getUi(), nullptr, nullptr,
            {vid_stream->codecpar->width, vid_stream->codecpar->height},
            C2D_TEXTURE_FMT_RGB565);
    video->setFiltering(C2D_TEXTURE_FILTER_LINEAR);
    addVideo(video);
#else
    int err = Kit_Init(KIT_INIT_ASS);
    if (err != 0) {
        fprintf(stderr, "Unable to initialize Kitchensink: %s\n", Kit_GetError());
        stop();
        return 1;
    }

    // Open up the sourcefile.
    src = Kit_CreateSourceFromUrl(file.c_str());
    if (src == nullptr) {
        fprintf(stderr, "Unable to load file '%s': %s\n", file.c_str(), Kit_GetError());
        stop();
        return 1;
    }

    // Create the player. Pick best video, audio and subtitle streams, and set subtitle
    // rendering resolution to screen resolution.
    player = Kit_CreatePlayer(
            src,
            Kit_GetBestSourceStream(src, KIT_STREAMTYPE_VIDEO),
            Kit_GetBestSourceStream(src, KIT_STREAMTYPE_AUDIO),
            -1,
            //Kit_GetBestSourceStream(src, KIT_STREAMTYPE_SUBTITLE),
            (int) uiMain->getRenderer()->getSize().x, (int) uiMain->getRenderer()->getSize().y);
    if (player == nullptr) {
        fprintf(stderr, "Unable to create player: %s\n", Kit_GetError());
        stop();
        return 1;
    }

    // Print some information;
    Kit_GetPlayerInfo(player, &player_info);
    // Make sure there is video in the file to play first.
    if (Kit_GetPlayerVideoStream(player) == -1) {
        fprintf(stderr, "File contains no video!\n");
        stop();
        return 1;
    }

    printf("video(%s, %s): %i x %i , audio(%s): %i\n",
           player_info.video.codec.name,
           SDL_GetPixelFormatName(player_info.video.output.format),
           player_info.video.output.width, player_info.video.output.height,
           player_info.audio.codec.name,
           player_info.audio.output.samplerate);

    // TODO
    //addAudio(player_info.audio.output.samplerate, 30);
    //addVideo(uiMain, nullptr, nullptr, Vector2f(pinfo.video.output.width, pinfo.video.output.height));

    // Init audio
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
    }
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));
    wanted_spec.freq = player_info.audio.output.samplerate;
    wanted_spec.format = player_info.audio.output.format;
    wanted_spec.channels = player_info.audio.output.channels;
    audio_dev = SDL_OpenAudioDevice(nullptr, 0, &wanted_spec, &audio_spec, 0);
    SDL_PauseAudioDevice(audio_dev, 0);

    C2DUIVideo *video = new C2DUIVideo(
            getUi(), nullptr, nullptr,
            {player_info.video.output.width, player_info.video.output.height},
            C2D_TEXTURE_FMT_ABGR8);
    video->setFiltering(C2D_TEXTURE_FILTER_LINEAR);
    addVideo(video);

    // Start playback
    Kit_PlayerPlay(player);
#endif

    return UIEmu::run(rom);
}

void Player::stop() {

#ifdef __SDL_KITCH__
    if (player) {
        Kit_PlayerStop(player);
        Kit_ClosePlayer(player);
        player = nullptr;
    }

    if (src) {
        Kit_CloseSource(src);
        src = nullptr;
    }

    Kit_Quit();

    if (audio_dev) {
        SDL_CloseAudioDevice(audio_dev);
        audio_dev = 0;
    }
#else
    avformat_close_input(&ctx_format);
    av_packet_unref(pkt);
    av_frame_unref(frame);
    avcodec_free_context(&ctx_codec);
    avformat_free_context(ctx_format);
#endif
    UIEmu::stop();
}

void Player::pause() {

    Kit_PlayerPause(player);
    UIEmu::pause();
}

void Player::resume() {

    UIEmu::resume();
    Kit_PlayerPlay(player);
}

int Player::update() {

    double position = Kit_GetPlayerPosition(player);
    double duration = Kit_GetPlayerDuration(player);
    std::string s_pos = formatTime(position);
    std::string s_dur = formatTime(duration);
    getFpsText()->setString(s_pos + " : " + s_dur);
    getFpsText()->setVisibility(Visible);

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

    if (players[0].state & EV_QUIT) {
        stop();
        return EV_QUIT;
    }

#ifndef __SDL_KITCH__
    if (av_read_frame(ctx_format, pkt) >= 0) {
        if (pkt->stream_index == stream_idx) {

            int ret = avcodec_send_packet(ctx_codec, pkt);
            if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                getUi()->getRenderer()->flip();
                return 0;
            }

            while (ret >= 0) {

                ret = avcodec_receive_frame(ctx_codec, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }

                ctx_sws = sws_getContext(
                        frame->width, frame->height,
                        ctx_codec->pix_fmt, frame->width, frame->height,
                        AV_PIX_FMT_RGB565, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

                int pitch;
                uint8_t *data[4];
                getVideo()->lock(nullptr, (void **) &data[0], &pitch);
                int linesize[4] = {pitch, 0, 0, 0};
                sws_scale(ctx_sws, frame->data, frame->linesize, 0,
                          frame->height, data, linesize);
                getVideo()->unlock();
            }
        }
    }
#else
    // player controls
    if (players[0].state & c2d::Input::Key::KEY_LEFT) {
        printf("Kit_PlayerSeek(pos=%f, dur=%f\n", position, duration);
        Kit_PlayerSeek(player, position - 10.0);
        getUi()->getInput()->clear(0);
    } else if (players[0].state & c2d::Input::Key::KEY_RIGHT) {
        printf("Kit_PlayerSeek(pos=%f, dur=%f\n", position, duration);
        if (position + 15.0 < duration) {
            Kit_PlayerSeek(player, position + 10.0);
            getUi()->getInput()->clear(0);
        }
    }

    if (!isPaused()) {

        if (Kit_GetPlayerState(player) == KIT_STOPPED) {
            printf("STOPPED\n");
            stop();
            return UI_KEY_SHOW_ROMLIST;
        }
        if (Kit_GetPlayerState(player) == KIT_PAUSED) {
            printf("PAUSED\n");
        }
        if (Kit_GetPlayerState(player) == KIT_CLOSED) {
            printf("CLOSED\n");
        }

        // audio
        int queued = SDL_GetQueuedAudioSize(audio_dev);
        if (queued < AUDIOBUFFER_SIZE) {
            int need = AUDIOBUFFER_SIZE - queued;

            while (need > 0) {
                int ret = Kit_GetPlayerAudioData(
                        player,
                        (unsigned char *) audiobuf,
                        AUDIOBUFFER_SIZE);
                need -= ret;
                if (ret > 0) {
                    SDL_QueueAudio(audio_dev, audiobuf, ret);
                } else {
                    break;
                }
            }
            // If we now have data, start playback (again)
            if (SDL_GetQueuedAudioSize(audio_dev) > 0) {
                SDL_PauseAudioDevice(audio_dev, 0);
            }
        }

        void *video_data;
        getVideo()->lock(nullptr, &video_data, nullptr);
        Kit_GetPlayerVideoDataRaw(player, video_data);
        getVideo()->unlock();
    }
#endif

    getUi()->getRenderer()->flip();

    return 0;
}

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
