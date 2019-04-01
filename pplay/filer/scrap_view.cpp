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

    Vector2f pos{poster->getPosition().x + 216, poster->getPosition().y + 16};
    Vector2f size{getSize().x - pos.x - 16, 170};

    // title
    title = new C2DText("TITLE", main->getFontSize(Main::FontSize::Medium), main->getFont());
    title->setPosition(pos.x, pos.y);
    add(title);

    // text
    text = new C2DText("", main->getFontSize(Main::FontSize::Small), main->getFont());
    text->setFillColor(COLOR_FONT);
    text->setPosition(pos.x, pos.y + 70);
    text->setOverflow(Text::NewLine);
    text->setSizeMax(size.x, size.y);
    text->setVisibility(Visibility::Hidden);
    add(text);

    resolution_icon = new TextIcon("1080p", main->getFontSize(Main::FontSize::Small), main->getFont());
    resolution_icon->setPosition(pos.x, 80);
    add(resolution_icon);
    video_icon = new TextIcon("h264", main->getFontSize(Main::FontSize::Small), main->getFont());
    video_icon->setPosition(pos.x + 64, 80);
    add(video_icon);
    audio_icon = new TextIcon("DTS", main->getFontSize(Main::FontSize::Small), main->getFont());
    audio_icon->setPosition(pos.x + 64 * 2, 80);
    add(audio_icon);
    subs_icon = new TextIcon("SUBS", main->getFontSize(Main::FontSize::Small), main->getFont());
    subs_icon->setPosition(pos.x + 64 * 3, 80);
    add(subs_icon);
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

    // load..
    std::string date =
            file.movies[0].release_date.substr(
                    0, file.movies[0].release_date.find_first_of('-'));
    title->setString(file.movies[0].title + " (" + date + ")");

    text->setString(file.movies[0].overview);
    text->setVisibility(Visibility::Visible);

    // load mediaInfo
    if (!file.mediaInfo.videos.empty()) {
        int width = file.mediaInfo.videos[0].width;
        int height = file.mediaInfo.videos[0].height;
        if (width == 3840) {
            resolution_icon->setString("2160p");
        } else if (width == 1920) {
            resolution_icon->setString("1080p");
        } else if (width == 1280) {
            resolution_icon->setString("720p");
        } else {
            resolution_icon->setString(std::to_string(height) + "p");
        }
        std::string vid = Utility::toUpper(file.mediaInfo.videos[0].codec);
        video_icon->setString(vid);
        video_icon->setPosition(
                resolution_icon->getPosition().x + resolution_icon->getSize().x + 8,
                video_icon->getPosition().y);
    }

    if (!file.mediaInfo.audios.empty()) {
        std::string aud = Utility::toUpper(file.mediaInfo.audios[0].codec);
        audio_icon->setString(aud);
        audio_icon->setPosition(
                video_icon->getPosition().x + video_icon->getSize().x + 8,
                audio_icon->getPosition().y);
        audio_icon->setVisibility(Visibility::Visible);
    } else {
        audio_icon->setVisibility(Visibility::Hidden);
    }

    if (!file.mediaInfo.subtitles.empty()) {
        if(!file.mediaInfo.audios.empty()) {
            subs_icon->setPosition(
                    audio_icon->getPosition().x + audio_icon->getSize().x + 8,
                    subs_icon->getPosition().y);
        } else {
            subs_icon->setPosition(
                    video_icon->getPosition().x + video_icon->getSize().x + 8,
                    subs_icon->getPosition().y);
        }
        subs_icon->setVisibility(Visibility::Visible);
    } else {
        subs_icon->setVisibility(Visibility::Hidden);
    }
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
