//
// Created by cpasjuste on 03/12/18.
//

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
    textTitle->setFillColor(file.color);
}
