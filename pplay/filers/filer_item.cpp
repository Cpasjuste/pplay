//
// Created by cpasjuste on 03/12/18.
//

#include <sstream>

#include "cross2d/c2d.h"
#include "main.h"
#include "filer_item.h"
#include "utility.h"

using namespace c2d;

FilerItem::FilerItem(Main *main, const c2d::FloatRect &rect, const MediaFile &file) : Rectangle(rect) {

    this->main = main;
    this->file = file;

    textTitle = new Text(file.name, main->getFontSize(Main::FontSize::Medium), main->getFont());
    textTitle->setPosition(16, 4);
    textTitle->setSizeMax(getSize().x - 64, 0);
    add(textTitle);

    textInfo = new Text("INFO", main->getFontSize(Main::FontSize::Small), main->getFont());
    textInfo->setPosition(16, textTitle->getPosition().y + main->getFontSize(Main::FontSize::Medium) + 4);
    textInfo->setSizeMax(getSize().x - 64, 0);
    textInfo->setFillColor(COLOR_FONT);
    add(textInfo);
}

void FilerItem::setFile(const MediaFile &f) {

    this->file = f;

    textTitle->setString(file.name);
    uint8_t alpha = textTitle->getAlpha();
    if (file.type == Io::Type::Directory) {
        textTitle->setFillColor(COLOR_BLUE);
    } else {
        textTitle->setFillColor(COLOR_RED);
    }
    textTitle->setAlpha(alpha);
    if (file.type == Io::Type::File) {
        textInfo->setString(file.name);
    } else {
        textInfo->setString("");
    }
}

void FilerItem::setTitle(const std::string &title) {
    textTitle->setString(title);
}
