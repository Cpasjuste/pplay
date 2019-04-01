//
// Created by cpasjuste on 01/04/19.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "utility.h"
#include "scrap_view.h"

using namespace c2d;

ScrapView::ScrapView(Main *m, const c2d::FloatRect &rect) : Rectangle(rect) {

    main = m;

    // backdrop
    backdrop = new Sprite();
    backdrop->setOrigin(Origin::BottomRight);
    backdrop->setPosition(getSize());
    backdrop->setVisibility(Visibility::Hidden);
    add(backdrop);

    // fade
    fade = new C2DTexture(main->getIo()->getDataReadPath() + "skin/fade.png");
    fade->setOrigin(Origin::BottomRight);
    fade->setPosition(backdrop->getPosition());
    fade->setVisibility(Visibility::Hidden);
    add(fade);

    // poster
    poster = new Sprite();
    poster->setPosition(64, 32);
    poster->setVisibility(Visibility::Hidden);
    add(poster);

    // text
    Vector2f pos{poster->getPosition().x + 216, poster->getPosition().y + 16};
    Vector2f size{getSize().x - pos.x - 16, 300 - 50};
    text = new C2DText("", main->getFontSize(Main::FontSize::Small), main->getFont());
    text->setFillColor(COLOR_FONT);
    text->setPosition(pos.x, pos.y);
    text->setOverflow(Text::NewLine);
    text->setSizeMax(size.x, size.y);
    text->setVisibility(Visibility::Hidden);
    add(text);
}

void ScrapView::setMovie(const MediaFile &file) {

    // always delete previous textures
    if (backdrop_texture) {
        delete (backdrop_texture);
        backdrop_texture = nullptr;
    }
    if (poster_texture) {
        delete (poster_texture);
        poster_texture = nullptr;
    }

    // if no movie was scrapped, return
    if (file.movies.empty()) {
        return;
    }

    setVisibility(Visibility::Visible);
    fade->setVisibility(Visibility::Hidden);
    backdrop->setVisibility(Visibility::Hidden);
    poster->setVisibility(Visibility::Hidden);
    text->setVisibility(Visibility::Hidden);

    if (!main->getPlayer() || main->getPlayer()->isStopped()) {
        // load backdrop if available
        std::string tex_path = pplay::Utility::getMediaBackdropPath(file);
        if (main->getIo()->exist(tex_path)) {
            fade->setVisibility(Visibility::Visible);
            backdrop_texture = new C2DTexture(tex_path);
            backdrop->setTexture(backdrop_texture, true);
            backdrop->setVisibility(Visibility::Visible);
            if (backdrop_texture->getTextureRect().width != 780) {
                // scaling
                float scaling = std::min(
                        getSize().x / backdrop_texture->getTextureRect().width,
                        getSize().y / backdrop_texture->getTextureRect().height);
                backdrop->setScale(scaling, scaling);
            }
        }
    }

    // load poster if available
    std::string tex_path = pplay::Utility::getMediaPosterPath(file);
    if (main->getIo()->exist(tex_path)) {
        poster_texture = new C2DTexture(tex_path);
        poster->setTexture(poster_texture, true);
        poster->setVisibility(Visibility::Visible);
        if (poster_texture->getTextureRect().width != 200) {
            // scaling
            float scaling = std::min(
                    getSize().x / poster_texture->getTextureRect().width,
                    getSize().y / poster_texture->getTextureRect().height);
            poster->setScale(scaling, scaling);
        }
    }

    // load overview
    std::string date =
            file.movies[0].release_date.substr(
                    0, file.movies[0].release_date.find_first_of('-'));
    std::string str = file.movies[0].title + " (" + date + ")\n\n"
                      + file.movies[0].overview;
    text->setString(str);
    text->setVisibility(Visibility::Visible);
}

void ScrapView::setVisibility(Visibility visibility, bool tweenPlay) {
    Rectangle::setVisibility(visibility, tweenPlay);
}

ScrapView::~ScrapView() {
    if (backdrop_texture) {
        delete (backdrop_texture);
    }
    if (poster_texture) {
        delete (poster_texture);
    }
}
