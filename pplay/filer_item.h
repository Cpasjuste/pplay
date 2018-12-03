//
// Created by cpasjuste on 03/12/18.
//

#ifndef PPLAY_FILERITEM_H
#define PPLAY_FILERITEM_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"
#include "media.h"

class Main;

class FilerItem : public c2d::RectangleShape {

public:

    FilerItem(Main *main, const c2d::FloatRect &rect,
              const c2d::Io::File &file = {}, const Media &media = {});

    const Media getMedia() const;

    const c2d::Io::File getFile() const;

private:

    Main *main;

    c2d::Text *textTitle;
    c2d::Text *textInfo;
    c2d::Text *textVideo;
    c2d::Text *textAudio;

    c2d::Io::File file;
    Media media;
};

#endif //PPLAY_FILERITEM_H
