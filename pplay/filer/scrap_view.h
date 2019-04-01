//
// Created by cpasjuste on 01/04/19.
//

#ifndef PPLAY_SCRAP_VIEW_H
#define PPLAY_SCRAP_VIEW_H

#include "cross2d/skeleton/sfml/Text.hpp"
#include "cross2d/skeleton/sfml/Sprite.hpp"
#include "cross2d/skeleton/sfml/Rectangle.hpp"
#include "p_movie.h"

class Main;

class ScrapView : public c2d::Rectangle {

public:

    explicit ScrapView(Main *main, const c2d::FloatRect &rect);

    ~ScrapView() override;

    void setMovie(const MediaFile &file);

    void setVisibility(c2d::Visibility visibility, bool tweenPlay = false) override;

private:

    Main *main;
    c2d::Sprite *backdrop;
    c2d::Texture *backdrop_texture = nullptr;
    c2d::Sprite *poster;
    c2d::Texture *poster_texture = nullptr;
    c2d::Texture *fade = nullptr;
    c2d::Text *text = nullptr;
};

#endif //PPLAY_SCRAP_VIEW_H
