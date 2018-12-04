//
// Created by cpasjuste on 03/12/18.
//

#ifndef PPLAY_FILERITEM_H
#define PPLAY_FILERITEM_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"
#include "media_file.h"

class Main;

class FilerItem : public c2d::RectangleShape {

public:

    FilerItem(Main *main, const c2d::FloatRect &rect, const MediaFile &file = {});

    c2d::Text *getTitle();

    const MediaFile getFile() const;

    void setFile(const MediaFile &file);

private:

    Main *main;
    MediaFile file;

    c2d::Text *textTitle;
    c2d::Text *textInfo;
    c2d::Text *textVideo;
    c2d::Text *textAudio;
};

#endif //PPLAY_FILERITEM_H
