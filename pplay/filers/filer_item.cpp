//
// Created by cpasjuste on 03/12/18.
//

#include <sstream>

#include "cross2d/c2d.h"
#include "main.h"
#include "filer_item.h"

using namespace c2d;

FilerItem::FilerItem(Main *main, const c2d::FloatRect &rect, const MediaFile &file) : Rectangle(rect) {

    this->main = main;
    this->file = file;

    textTitle = new Text(file.name, FONT_SIZE, main->getFont());
    textTitle->setPosition(16, 4);
    textTitle->setWidth(getSize().x - 64);
    add(textTitle);

    textInfo = new Text("Info: N/A", FONT_SIZE - 8, main->getFont());
    textInfo->setPosition(16, 4 + FONT_SIZE + 4);
    textInfo->setWidth(getSize().x - 64);
    textInfo->setFillColor(COLOR_FONT);
    add(textInfo);

    textVideo = new Text("Video: N/A", FONT_SIZE - 8, main->getFont());
    textVideo->setPosition(16, textInfo->getPosition().y + FONT_SIZE - 6);
    textVideo->setWidth(getSize().x - 64);
    textVideo->setFillColor(COLOR_FONT);
    add(textVideo);

    textAudio = new Text("Audio: N/A", FONT_SIZE - 8, main->getFont());
    textAudio->setPosition(16, textVideo->getPosition().y + FONT_SIZE - 6);
    textAudio->setWidth(getSize().x - 64);
    textAudio->setFillColor(COLOR_FONT);
    add(textAudio);
}

void FilerItem::setFile(const MediaFile &file) {

    this->file = file;

    textTitle->setString(file.name);
    uint8_t alpha = textTitle->getAlpha();
    if (file.type == Io::Type::Directory) {
        textTitle->setFillColor(COLOR_BLUE);
    } else {
        textTitle->setFillColor(COLOR_RED);
    }
    textTitle->setAlpha(alpha);

    if (file.type == Io::Type::File) {
        std::ostringstream ossInfo;
        ossInfo << "Info: " << file.size;
        textInfo->setString(ossInfo.str());
        if (!file.getMedia().videos.empty()) {
            std::ostringstream oss;
            oss << "Video: "
                << file.getMedia().videos[0].codec << " @ "
                << file.getMedia().bit_rate / 1000 << " kb/s, "
                << file.getMedia().videos[0].width << "x"
                << file.getMedia().videos[0].height;
            textVideo->setString(oss.str());
        } else {
            textVideo->setString("Video: n/a");
        }
        if (!file.getMedia().audios.empty()) {
            std::ostringstream oss;
            oss << "Audio: "
                << file.getMedia().audios[0].codec << " @ "
                << file.getMedia().audios[0].bit_rate / 1000 << " kb/s, "
                << file.getMedia().audios[0].sample_rate << " hz";
            textAudio->setString(oss.str());
        } else {
            textAudio->setString("Audio: n/a");
        }
    } else {
        textInfo->setString("");
        textVideo->setString("");
        textAudio->setString("");
    }
}
