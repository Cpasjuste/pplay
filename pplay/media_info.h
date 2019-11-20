//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_INFO_H
#define PPLAY_MEDIA_INFO_H

#include <string>
#include <vector>

#include "cross2d/skeleton/io.h"

class MediaInfo {

public:

    class Track {
    public:
        int id;
        std::string type;
        std::string title = "Unknown";
        std::string language = "N/A";
        std::string codec;
        int channels;
        int bit_rate;
        int sample_rate;
        int width;
        int height;
    };

    class Playback {
    public:
        int vid_id = -1;
        int aud_id = -1;
        int sub_id = -1;
        int position = 0;
    };

    MediaInfo() = default;

    explicit MediaInfo(const c2d::Io::File &file);

    void save(const c2d::Io::File &file);

    // media information
    std::string title = "Unknown";
    std::string path;
    long duration = 0;
    int bit_rate = 0;
    std::vector<Track> videos;
    std::vector<Track> audios;
    std::vector<Track> subtitles;
    // media playback status
    Playback playbackInfo;

private:

    std::string serialize_path;

    bool serialize();

    bool deserialize();

    void debut_print();
};

#endif //PPLAY_MEDIA_INFO_H
