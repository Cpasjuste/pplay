//
// Created by cpasjuste on 03/12/18.
//

#ifndef PPLAY_FILERITEM_H
#define PPLAY_FILERITEM_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"
#include "media_file.h"

class Main;

class FilerItem : public c2d::Rectangle {

public:

    FilerItem(Main *main, const c2d::FloatRect &rect, const MediaFile &file = {});

    void setFile(const MediaFile &file);

    void setTitle(const std::string &title);

private:

    Main *main;
    MediaFile file;

    c2d::Text *textTitle;
    c2d::Text *textInfo;
};

#endif //PPLAY_FILERITEM_H
