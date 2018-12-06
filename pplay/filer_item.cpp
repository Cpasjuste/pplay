//
// Created by cpasjuste on 03/12/18.
//

#include <sstream>

#include "cross2d/c2d.h"
#include "main.h"
#include "filer_item.h"

using namespace c2d;

FilerItem::FilerItem(Main *main, const c2d::FloatRect &rect, const MediaFile &file) : RectangleShape(rect) {

    this->main = main;
    this->file = file;

    setFillColor(Color::Transparent);

    textTitle = new Text(file.name, FONT_SIZE, main->getFont());
    textTitle->setPosition(16, 4);
    textTitle->setWidth(getSize().x - 64);
    add(textTitle);

    textVideo = new Text("", FONT_SIZE - 8, main->getFont());
    textVideo->setPosition(16, 4 + FONT_SIZE + 2);
    textVideo->setWidth(getSize().x - 64);
    add(textVideo);

    textAudio = new Text("", FONT_SIZE - 8, main->getFont());
    textAudio->setPosition(16, textVideo->getPosition().y + FONT_SIZE - 6);
    textAudio->setWidth(getSize().x - 64);
    add(textAudio);
}

c2d::Text *FilerItem::getTitle() {
    return textTitle;
}

const MediaFile FilerItem::getFile() const {
    return file;
}

void FilerItem::setFile(const MediaFile &file) {

    this->file = file;

    textTitle->setString(file.name);
    if (file.type == Io::Type::Directory) {
        textTitle->setFillColor(file.color);
    } else {
        textTitle->setFillColor(Color::White);
    }

    if (!file.media.videos.empty()) {
        std::ostringstream oss;
        oss << "Video: " << file.media.videos[0].width << "x" << file.media.videos[0].height;
        oss << ", " << file.media.videos[0].codec << " @ " << file.media.videos[0].rate / 1000 << " kb/s";
        textVideo->setString(oss.str());
        textVideo->setFillColor(file.color);
    } else {
        textVideo->setString("");
    }

    if (!file.media.audios.empty()) {
        std::ostringstream oss;
        oss << "Audio: " << file.media.audios[0].codec << " @ " << file.media.audios[0].rate / 1000 << " khz";
        textAudio->setString(oss.str());
        textAudio->setFillColor(file.color);
    } else {
        textAudio->setString("");
    }
}
