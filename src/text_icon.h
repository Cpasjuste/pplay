//
// Created by cpasjuste on 01/04/19.
//

#ifndef PPLAY_TEXT_ICON_H
#define PPLAY_TEXT_ICON_H

#include "cross2d/skeleton/sfml/RoundedRectangleShape.h"
#include "cross2d/skeleton/sfml/Text.hpp"

class TextIcon : public c2d::RoundedRectangleShape {

public:

    TextIcon(const std::string &str, unsigned int fontSize, c2d::Font *font,
             const c2d::Color &color = c2d::Color::White);

    void setFillColor(const c2d::Color &color) override;

    void setString(const std::string &string);

private:

    c2d::Text *text;
};

#endif //PPLAY_TEXT_ICON_H
