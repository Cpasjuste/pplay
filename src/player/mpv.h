//
// Created by cpasjuste on 02/04/19.
//

#ifndef PPLAY_MPV_H
#define PPLAY_MPV_H

#include <cstdio>
#include <string>

#include <mpv/client.h>
#include <mpv/render_gl.h>
#include "media_info.h"

class Mpv {

public:

    enum class LoadType {
        Replace,
        Append,
        AppendPlay
    };

    explicit Mpv(const std::string &configPath, bool initRender, const std::string &hwdec = "",
                 const std::string &swSpeed = "quality");

    ~Mpv();

    int load(const std::string &file, LoadType loadType, const std::string &options);

    int save();

    // add an external subtitle file (text subtitles must already be UTF-8)
    int addSubtitle(const std::string &path, const std::string &title, const std::string &lang);

    int pause();

    int resume();

    int stop();

    int seek(double position);

    int setSpeed(double speed);

    double getSpeed();

    int setVid(int id);

    int getVid();

    int setAid(int id);

    int getAid();

    int setSid(int id);

    int getSid();

    // codec of the subtitle track in use ("subrip", "ass", "hdmv_pgs_subtitle", ...), "" if none
    std::string getSubtitleCodec();

    // Image subtitles (PGS, VobSub, DVB) are drawn by the CPU on the video frames instead of
    // by the GPU: the PS4 libmpv only has a few precompiled shaders and the one needed for
    // image subtitles is not among them (mpv then paints the whole screen blue).
    // Returns a negative mpv error when the filter could not be added/removed.
    int setBitmapSubtitleRendering(bool enable);

    bool isBitmapSubtitleRendering() const;

    int getVideoBitrate();

    int getAudioBitrate();

    long getDuration();

    long getPosition();

    bool isStopped();

    bool isPaused();

    bool isAvailable();

    mpv_event *getEvent();

    mpv_handle *getHandle();

    mpv_render_context *getContext();

    MediaInfo getMediaInfo(const c2d::Io::File &file);

private:

    mpv_handle *handle = nullptr;
    mpv_render_context *context = nullptr;
    bool bitmapSubFilter = false;
};

#endif //PPLAY_MPV_H
