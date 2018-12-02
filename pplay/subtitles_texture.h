//
// Created by cpasjuste on 29/11/18.
//

#ifndef PPLAY_SUBTITLES_TEXTURE_H
#define PPLAY_SUBTITLES_TEXTURE_H

#define ATLAS_MAX 256

class SubtitlesTexture : public c2d::C2DTexture {

public:

    SubtitlesTexture(const c2d::Vector2f &size = c2d::Vector2f(1024, 1024),
                     c2d::Texture::Format format = c2d::Texture::Format::RGBA8);

    ~SubtitlesTexture();

    void setRectsCount(int count);

    SDL_Rect *getRectsSrc();

    SDL_Rect *getRectsDst();

    void onDraw(c2d::Transform &transform) override;

private:

    c2d::Sprite *sprite;
    int rects_count = 0;
    SDL_Rect rects_src[ATLAS_MAX];
    SDL_Rect rects_dst[ATLAS_MAX];
};

#endif //PPLAY_SUBTITLES_TEXTURE_H
