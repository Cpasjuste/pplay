//
// Created by cpasjuste on 01/04/19.
//

#include "text_icon.h"

using namespace c2d;

TextIcon::TextIcon(const std::string &str, unsigned int fontSize, Font *font, const Color &color)
        : RectangleShape({32, 32}) {

    text = new Text(str, fontSize, font);
    text->setOrigin(Origin::Center);
    setString(str);
    RectangleShape::add(text);

    RectangleShape::setFillColor(Color::Transparent);
    RectangleShape::setOutlineColor(color);
    RectangleShape::setOutlineThickness(1);
    RectangleShape::setCornersRadius(5);
    RectangleShape::setCornerPointCount(8);
}

void TextIcon::setFillColor(const Color &color) {
    text->setFillColor(color);
    RectangleShape::setFillColor(Color::Transparent);
}

void TextIcon::setString(const std::string &string) {
    text->setString(string);
    setSize(text->getLocalBounds().width + 8, (float) text->getCharacterSize() + 6);
    text->setPosition(getSize().x / 2, (getSize().y / 2) + 1);
}
