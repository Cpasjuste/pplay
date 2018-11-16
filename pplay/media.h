//
// Created by cpasjuste on 13/11/18.
//

#ifndef PPLAY_MEDIA_INFO_H
#define PPLAY_MEDIA_INFO_H

#include <string>
#include <vector>

class Media {

public:

    class Stream {
    public:
        std::string title;
        std::string language;
        std::string codec;
        int rate;
        int width;
        int height;
    };

    std::string title;
    std::string path;
    long duration;
    std::vector<Stream> videos;
    std::vector<Stream> audios;
    std::vector<Stream> subtitles;

    bool serialize(const std::string &dst);

    bool deserialize(const std::string &src);

    void debut_print();
};

#endif //PPLAY_MEDIA_INFO_H
