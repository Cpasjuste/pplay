//
// Created by cpasjuste on 13/11/18.
//

#include <fstream>
#include <iostream>

#include "media_info.h"

bool MediaInfo::serialize(const std::string &dst) {

    int count;
    size_t size;
    std::fstream fs;

    fs.open(dst, std::ios::binary | std::ios::out);
    if (!fs.is_open()) {
        return false;
    }

    // name
    size = name.size();
    fs.write((char *) &size, sizeof(size_t));
    fs.write((char *) name.c_str(), size);

    // path
    size = path.size();
    fs.write((char *) &size, sizeof(size_t));
    fs.write((char *) path.c_str(), size);

    // duration
    fs.write((char *) &duration, sizeof(duration));

    // video streams
    count = (int) videos.size();
    fs.write((char *) &count, sizeof(count));
    for (auto &stream : videos) {
        // name
        size = stream.name.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.name.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
        // rate
        fs.write((char *) &stream.rate, sizeof(stream.rate));
        // width / height
        fs.write((char *) &stream.width, sizeof(stream.width));
        fs.write((char *) &stream.height, sizeof(stream.height));
    }

    // audio streams
    count = (int) audios.size();
    fs.write((char *) &count, sizeof(count));
    for (auto &stream : audios) {
        // name
        size = stream.name.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.name.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
        // rate
        fs.write((char *) &stream.rate, sizeof(stream.rate));
    }

    // subtitles streams
    count = (int) subtitles.size();
    fs.write((char *) &count, sizeof(count));
    for (auto &stream : subtitles) {
        // name
        size = stream.name.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.name.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
    }

    return true;
}

bool MediaInfo::deserialize(const std::string &src) {

    int count;
    char *data;
    size_t size;
    std::fstream fs;

    fs.open(src, std::ios::binary | std::ios::in);
    if (!fs.is_open()) {
        return false;
    }

    // name
    fs.read((char *) &size, sizeof(size));
    data = new char[size + 1];
    fs.read(data, size);
    data[size] = '\0';
    name = data;
    delete data;

    // path
    fs.read((char *) &size, sizeof(size));
    data = new char[size + 1];
    fs.read(data, size);
    data[size] = '\0';
    path = data;
    delete data;

    // duration
    fs.read((char *) &duration, sizeof(duration));

    // video streams
    videos.clear();
    fs.read((char *) &count, sizeof(count));
    for (int i = 0; i < count; i++) {
        StreamInfo stream;
        // name
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.name = data;
        delete data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete data;
        // rate
        fs.read((char *) &stream.rate, sizeof(stream.rate));
        // width / height
        fs.read((char *) &stream.width, sizeof(stream.width));
        fs.read((char *) &stream.height, sizeof(stream.height));

        videos.push_back(stream);
    }

    // audio streams
    audios.clear();
    fs.read((char *) &count, sizeof(count));
    for (int i = 0; i < count; i++) {
        StreamInfo stream;
        // name
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.name = data;
        delete data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete data;
        // rate
        fs.read((char *) &stream.rate, sizeof(stream.rate));

        audios.push_back(stream);
    }

    // subtitles streams
    subtitles.clear();
    fs.read((char *) &count, sizeof(count));
    for (int i = 0; i < count; i++) {
        StreamInfo stream;
        // name
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.name = data;
        delete data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete data;

        subtitles.push_back(stream);
    }

    return true;
}
