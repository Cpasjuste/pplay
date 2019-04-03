//
// Created by cpasjuste on 13/11/18.
//

#include <fstream>
#include <iostream>

#include "cross2d/c2d.h"
#include "media_info.h"
#include "utility.h"

MediaInfo::MediaInfo(const c2d::Io::File &file) {

    serialize_path = pplay::Utility::getMediaInfoPath(file);
    if (!pplay::Utility::isMedia(file)) {
        return;
    }
    deserialize();
}

void MediaInfo::save(const c2d::Io::File &file) {

    serialize_path = pplay::Utility::getMediaInfoPath(file);
    serialize();
}

bool MediaInfo::serialize() {

    int count;
    size_t size;
    std::fstream fs;

    fs.open(serialize_path.c_str(), std::ios::binary | std::ios::out);
    if (!fs.is_open()) {
        return false;
    }

    // title
    size = title.size();
    fs.write((char *) &size, sizeof(size_t));
    fs.write((char *) title.c_str(), size);
    //printf("Media::serialize: title = %s\n", title.c_str());

    // path
    size = path.size();
    fs.write((char *) &size, sizeof(size_t));
    fs.write((char *) path.c_str(), size);
    //printf("Media::serialize: path = %s\n", path.c_str());

    // duration
    fs.write((char *) &duration, sizeof(duration));
    //printf("Media::serialize: duration = %li\n", duration);

    // bit rate
    fs.write((char *) &bit_rate, sizeof(bit_rate));

    // media playback status
    fs.write((char *) &playbackInfo.vid_id, sizeof(playbackInfo.vid_id));
    fs.write((char *) &playbackInfo.aud_id, sizeof(playbackInfo.aud_id));
    fs.write((char *) &playbackInfo.sub_id, sizeof(playbackInfo.sub_id));
    fs.write((char *) &playbackInfo.position, sizeof(playbackInfo.position));

    // video streams
    count = (int) videos.size();
    fs.write((char *) &count, sizeof(count));
    //printf("Media::serialize: video streams = %i\n", count);
    for (auto &stream : videos) {
        // id
        fs.write((char *) &stream.id, sizeof(stream.id));
        // type
        size = stream.type.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.type.c_str(), size);
        // title
        size = stream.title.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.title.c_str(), size);
        // language
        size = stream.language.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.language.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
        // rate
        fs.write((char *) &stream.bit_rate, sizeof(stream.bit_rate));
        // width / height
        fs.write((char *) &stream.width, sizeof(stream.width));
        fs.write((char *) &stream.height, sizeof(stream.height));
    }

    // audio streams
    count = (int) audios.size();
    fs.write((char *) &count, sizeof(count));
    //printf("Media::serialize: audio streams = %i\n", count);
    for (auto &stream : audios) {
        // id
        fs.write((char *) &stream.id, sizeof(stream.id));
        // type
        size = stream.type.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.type.c_str(), size);
        // title
        size = stream.title.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.title.c_str(), size);
        // language
        size = stream.language.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.language.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
        // rate
        fs.write((char *) &stream.bit_rate, sizeof(stream.bit_rate));
        fs.write((char *) &stream.sample_rate, sizeof(stream.sample_rate));
    }

    // subtitles streams
    count = (int) subtitles.size();
    fs.write((char *) &count, sizeof(count));
    //printf("Media::serialize: subtitle streams = %i\n", count);
    for (auto &stream : subtitles) {
        // id
        fs.write((char *) &stream.id, sizeof(stream.id));
        // type
        size = stream.type.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.type.c_str(), size);
        // title
        size = stream.title.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.title.c_str(), size);
        // language
        size = stream.language.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.language.c_str(), size);
        // codec
        size = stream.codec.size();
        fs.write((char *) &size, sizeof(size_t));
        fs.write((char *) stream.codec.c_str(), size);
    }

    fs.close();

    return true;
}

bool MediaInfo::deserialize() {

    int count;
    char *data;
    size_t size;
    std::fstream fs;

    fs.open(serialize_path.c_str(), std::ios::binary | std::ios::in);
    if (!fs.is_open()) {
        return false;
    }

    // title
    fs.read((char *) &size, sizeof(size));
    data = new char[size + 1];
    fs.read(data, size);
    data[size] = '\0';
    title = data;
    delete[] data;
    //printf("Media::deserialize: title = %s\n", title.c_str());

    // path
    fs.read((char *) &size, sizeof(size));
    data = new char[size + 1];
    fs.read(data, size);
    data[size] = '\0';
    path = data;
    delete[] data;
    //printf("Media::deserialize: path = %s\n", path.c_str());

    // duration
    fs.read((char *) &duration, sizeof(duration));
    //printf("Media::deserialize: duration = %li\n", duration);

    // bit rate
    fs.read((char *) &bit_rate, sizeof(bit_rate));

    // media playback status
    fs.read((char *) &playbackInfo.vid_id, sizeof(playbackInfo.vid_id));
    fs.read((char *) &playbackInfo.aud_id, sizeof(playbackInfo.aud_id));
    fs.read((char *) &playbackInfo.sub_id, sizeof(playbackInfo.sub_id));
    fs.read((char *) &playbackInfo.position, sizeof(playbackInfo.position));

    // video streams
    videos.clear();
    fs.read((char *) &count, sizeof(count));
    //printf("Media::deserialize: video streams = %i\n", count);
    for (int i = 0; i < count; i++) {
        Track stream{};
        // id
        fs.read((char *) &stream.id, sizeof(stream.id));
        // type
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.type = data;
        // title
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.title = data;
        delete[] data;
        // language
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.language = data;
        delete[] data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete[] data;
        // rate
        fs.read((char *) &stream.bit_rate, sizeof(stream.bit_rate));
        // width / height
        fs.read((char *) &stream.width, sizeof(stream.width));
        fs.read((char *) &stream.height, sizeof(stream.height));

        videos.push_back(stream);
    }

    // audio streams
    audios.clear();
    fs.read((char *) &count, sizeof(count));
    //printf("Media::deserialize: audio streams = %i\n", count);
    for (int i = 0; i < count; i++) {
        Track stream{};
        // id
        fs.read((char *) &stream.id, sizeof(stream.id));
        // type
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.type = data;
        // title
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.title = data;
        delete[] data;
        // language
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.language = data;
        delete[] data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete[] data;
        // rate
        fs.read((char *) &stream.bit_rate, sizeof(stream.bit_rate));
        fs.read((char *) &stream.sample_rate, sizeof(stream.sample_rate));

        audios.push_back(stream);
    }

    // subtitles streams
    subtitles.clear();
    fs.read((char *) &count, sizeof(count));
    //printf("Media::deserialize: subtitle streams = %i\n", count);
    for (int i = 0; i < count; i++) {
        Track stream{};
        // id
        fs.read((char *) &stream.id, sizeof(stream.id));
        // type
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.type = data;
        // title
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.title = data;
        delete[] data;
        // language
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.language = data;
        delete[] data;
        // codec
        fs.read((char *) &size, sizeof(size));
        data = new char[size + 1];
        fs.read(data, size);
        data[size] = '\0';
        stream.codec = data;
        delete[] data;

        subtitles.push_back(stream);
    }

    fs.close();

    return true;
}

void MediaInfo::debut_print() {
    printf("============= MEDIA ===============\n");
    printf("title: %s, duration: %s\n", title.c_str(), pplay::Utility::formatTimeShort(duration).c_str());
    printf("\tvideo streams: %i\n", (int) videos.size());
    for (auto &stream : videos) {
        printf("\t\tlanguage: %s, title: %s, resolution: %ix%i, codec: %s @ %i kb/s\n",
               stream.language.c_str(), stream.title.c_str(),
               stream.width, stream.height, stream.codec.c_str(), stream.bit_rate / 1000);
    }
    printf("\taudio streams: %i\n", (int) audios.size());
    for (auto &stream : audios) {
        printf("\t\tlanguage: %s, title: %s, codec: %s @ %i hz\n",
               stream.language.c_str(), stream.title.c_str(),
               stream.codec.c_str(), stream.bit_rate);
    }
    printf("\tsubtitle streams: %i\n", (int) subtitles.size());
    for (auto &stream : subtitles) {
        printf("\t\tlanguage: %s, title: %s, codec: %s\n",
               stream.language.c_str(), stream.title.c_str(),
               stream.codec.c_str());
    }
    printf("===================================\n");
}
