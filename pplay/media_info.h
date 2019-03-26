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

    class Stream {
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

    std::string title;
    std::string path;
    long duration = 0;
    int bit_rate = 0;
    std::vector<Stream> videos;
    std::vector<Stream> audios;
    std::vector<Stream> subtitles;
    int loaded = 0;

    bool isLoaded() const;

    bool serialize(const std::string &dst);

    bool deserialize(const std::string &src);

    void debut_print();
};

#endif //PPLAY_MEDIA_INFO_H
