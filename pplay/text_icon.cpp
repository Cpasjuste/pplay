//
// Created by cpasjuste on 01/04/19.
//

#include "cross2d/c2d.h"
#include "text_icon.h"

using namespace c2d;

TextIcon::TextIcon(const std::string &str, unsigned int fontSize, Font *font, const Color &color)
        : RoundedRectangleShape({32, 32}) {

    text = new Text(str, fontSize, font);
    text->setOrigin(Origin::Center);
    setString(str);
    add(text);

    setFillColor(color);
    setOutlineColor(color);
    setOutlineThickness(1);
}

void TextIcon::setFillColor(const Color &color) {
    text->setFillColor(color);
    m_fillColor = Color::Transparent;
    m_shape_dirty = true;
}

void TextIcon::setString(const std::string &string) {
    text->setString(string);
    setSize(text->getLocalBounds().width + 8, text->getCharacterSize() + 6);
    text->setPosition(getSize().x / 2, (getSize().y / 2) + 1);
}
