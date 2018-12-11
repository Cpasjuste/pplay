//
// Created by cpasjuste on 29/11/18.
//

#include "cross2d/c2d.h"
#include "subtitles_texture.h"

using namespace c2d;

SubtitlesTexture::SubtitlesTexture(const Vector2f &size, Texture::Format format)
        : C2DTexture(size, format) {

    sprite = new Sprite();
    sprite->setTexture(this);
}

void SubtitlesTexture::onDraw(c2d::Transform &transform) {

    for (int i = 0; i < rects_count; i++) {
        sprite->setPosition(rects_dst[i].x, rects_dst[i].y);
        sprite->setTextureRect({rects_src[i].x, rects_src[i].y, rects_src[i].w, rects_src[i].h});
        c2d_renderer->draw(sprite->getVertexArray(), transform * sprite->getTransform(), this);
    }
}

void SubtitlesTexture::setRectsCount(int count) {
    rects_count = count;
}

SDL_Rect *SubtitlesTexture::getRectsSrc() {
    return rects_src;
}

SDL_Rect *SubtitlesTexture::getRectsDst() {
    return rects_dst;
}

SubtitlesTexture::~SubtitlesTexture() {
    delete (sprite);
}
