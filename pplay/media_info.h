//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_INFO_H
#define PPLAY_MEDIA_INFO_H

#include <string>
#include <vector>

class MediaInfo {

public:

    class StreamInfo {
    public:
        std::string name;
        std::string codec;
        int rate;
        int width;
        int height;
    };

    std::string name;
    std::string path;
    long duration;
    std::vector<StreamInfo> videos;
    std::vector<StreamInfo> audios;
    std::vector<StreamInfo> subtitles;

    bool serialize(const std::string &dst);

    bool deserialize(const std::string &src);

};

#endif //PPLAY_MEDIA_INFO_H
