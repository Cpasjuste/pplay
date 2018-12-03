//
// Created by cpasjuste on 03/12/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "filer_item.h"

using namespace c2d;

FilerItem::FilerItem(Main *ma, const FloatRect &rect, const Io::File &fi, const Media &me) : RectangleShape(rect) {

    main = ma;
    file = fi;
    media = me;

    setFillColor(Color::Transparent);
    setOutlineThickness(2);
    setOutlineColor(COLOR_FONT);

    textTitle = new Text(file.name, FONT_SIZE, main->getFont());
    textTitle->setWidth(getSize().x - 4);
    add(textTitle);
}

const Media FilerItem::getMedia() const {
    return media;
}

const c2d::Io::File FilerItem::getFile() const {
    return file;
}
