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
    backdrop->setAlpha(0);
    backdrop->add(new TweenAlpha(0, 255, 0.3f));
    add(backdrop);

    // backdrop fade
    fade = new C2DTexture(main->getIo()->getRomFsPath() + "skin/fade.png");
    fade->setOrigin(Origin::BottomRight);
    fade->setPosition(backdrop->getPosition());
    fade->setFillColor(Color::Black);
    add(fade);

    // poster
    poster = new Sprite();
    poster->setPosition(64, 32);
    poster->setVisibility(Visibility::Hidden);
    poster->setAlpha(0);
    poster->add(new TweenAlpha(0, 255, 0.3f));
    add(poster);

    Vector2f pos{poster->getPosition().x + 216, poster->getPosition().y + 16};
    Vector2f size{getSize().x - pos.x - 16, 180};

    // title
    title = new C2DText("TITLE", main->getFontSize(Main::FontSize::Medium), main->getFont());
    title->setPosition(pos.x, pos.y);
    add(title);

    // text
    overview = new C2DText("", main->getFontSize(Main::FontSize::Small), main->getFont());
    overview->setFillColor(COLOR_FONT);
    overview->setPosition(pos.x, pos.y + 70);
    overview->setOverflow(Text::NewLine);
    overview->setSizeMax(size.x, size.y);
    add(overview);

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

    clock = new C2DClock();
}

void ScrapView::setMovie(const MediaFile &f) {

    file = f;

    video_icon->setVisibility(Visibility::Hidden);
    audio_icon->setVisibility(Visibility::Hidden);
    subs_icon->setVisibility(Visibility::Hidden);
    resolution_icon->setVisibility(Visibility::Hidden);

    // if no movie was scrapped, return
    if (file.movies.empty()) {
        title->setString(file.name);
        overview->setString("No information available.\n\n"
                            "Please use the scrapper option to get "
                            "some information about this media.");
    } else {
        // load..
        pscrap::Movie movie = file.movies[0];
        std::string date =
                movie.release_date.substr(0, movie.release_date.find_first_of('-'));
        title->setString(movie.title + " (" + date + ")");
        overview->setString(movie.overview);

        // load mediaInfo
        if (!file.mediaInfo.videos.empty()) {
            MediaInfo::Track track = file.mediaInfo.videos[0];
            int width = track.width;
            int height = track.height;
            if (width == 3840) {
                resolution_icon->setString("2160p");
            } else if (width == 1920) {
                resolution_icon->setString("1080p");
            } else if (width == 1280) {
                resolution_icon->setString("720p");
            } else {
                resolution_icon->setString(std::to_string(height) + "p");
            }
            std::string vid = Utility::toUpper(track.codec);
            video_icon->setString(vid);
            video_icon->setPosition(
                    resolution_icon->getPosition().x + resolution_icon->getSize().x + 8,
                    video_icon->getPosition().y);
            video_icon->setVisibility(Visibility::Visible);
            resolution_icon->setVisibility(Visibility::Visible);
        }

        if (!file.mediaInfo.audios.empty()) {
            std::string aud = Utility::toUpper(file.mediaInfo.audios[0].codec);
            audio_icon->setString(aud);
            audio_icon->setPosition(
                    video_icon->getPosition().x + video_icon->getSize().x + 8,
                    audio_icon->getPosition().y);
            audio_icon->setVisibility(Visibility::Visible);
        }

        if (!file.mediaInfo.subtitles.empty()) {
            if (!file.mediaInfo.audios.empty()) {
                subs_icon->setPosition(
                        audio_icon->getPosition().x + audio_icon->getSize().x + 8,
                        subs_icon->getPosition().y);
            } else {
                subs_icon->setPosition(
                        video_icon->getPosition().x + video_icon->getSize().x + 8,
                        subs_icon->getPosition().y);
            }
            subs_icon->setVisibility(Visibility::Visible);
        }
    }
}

void ScrapView::onUpdate() {

    if (!isVisible() || !main->getFiler()->isVisible()) {
        return;
    }

    unsigned int keys = main->getInput()->getKeys();

    if (keys > 0 && keys != Input::Delay) {
        clock->restart();
    } else if (keys == 0 && clock->getElapsedTime().asMilliseconds() > main->getInput()->getRepeatDelay()) {
        // load images
        if (!loaded && !file.movies.empty()) {
            loaded = true;
            // load backdrop if available
            std::string tex_path = pplay::Utility::getMediaBackdropPath(file);
            if (main->getIo()->exist(tex_path)) {
                fade->setVisibility(Visibility::Visible);
                backdrop_texture = new C2DTexture(tex_path);
                backdrop->setTexture(backdrop_texture, true);
                backdrop->setVisibility(Visibility::Visible, true);
                if (backdrop_texture->getTextureRect().width != 780) {
                    // scaling
                    float scaling = std::min(
                            getSize().x / backdrop_texture->getTextureRect().width,
                            getSize().y / backdrop_texture->getTextureRect().height);
                    backdrop->setScale(scaling, scaling);
                }
            }

            // load poster if available
            tex_path = pplay::Utility::getMediaPosterPath(file);
            if (main->getIo()->exist(tex_path)) {
                poster_texture = new C2DTexture(tex_path);
                poster->setTexture(poster_texture, true);
                poster->setVisibility(Visibility::Visible, true);
                if (poster_texture->getTextureRect().width != 200) {
                    // scaling
                    float scaling = std::min(
                            getSize().x / poster_texture->getTextureRect().width,
                            getSize().y / poster_texture->getTextureRect().height);
                    poster->setScale(scaling, scaling);
                }
            }
        }
    }

    Rectangle::onUpdate();
}

void ScrapView::unload() {

    poster->setVisibility(Visibility::Hidden);
    if (poster_texture != nullptr) {
        delete (poster_texture);
        poster_texture = nullptr;
        poster->setTexture(nullptr);
    }

    fade->setVisibility(Visibility::Hidden);
    backdrop->setVisibility(Visibility::Hidden);
    if (backdrop_texture != nullptr) {
        delete (backdrop_texture);
        backdrop_texture = nullptr;
        backdrop->setTexture(nullptr);
    }

    loaded = false;
}

ScrapView::~ScrapView() {

    if (backdrop_texture != nullptr) {
        delete (backdrop_texture);
        backdrop_texture = nullptr;
    }

    if (poster_texture != nullptr) {
        delete (poster_texture);
        poster_texture = nullptr;
    }

    delete (clock);
}
