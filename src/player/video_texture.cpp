//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "video_texture.h"

using namespace c2d;

VideoTexture::VideoTexture(const c2d::Vector2f &size, Mpv *m) : GLTextureBuffer(size, Format::RGBA8) {
    mpv = m;

    // fade
    fade = new C2DTexture(c2d_renderer->getIo()->getRomFsPath() + "skin/fade.png");
    fade->setScale(size.x / fade->getTextureRect().width, size.y / fade->getTextureRect().height);
    fade->setFillColor(Color::Black);
    fade->setAlpha(0);
    fadeTween = new TweenAlpha(0, 255, 0.5f);
    fade->add(fadeTween);
    GLTextureBuffer::add(fade);
}

void VideoTexture::hideFade() {
    fadeTween->play(TweenDirection::Backward);
}

void VideoTexture::showFade() {
    fadeTween->play(TweenDirection::Forward);
}

mpv_render_context *VideoTexture::getContext() {
    return mpv->getContext();
}

void VideoTexture::onDraw(c2d::Transform &transform, bool draw) {
    uint64_t flags = mpv_render_context_update(mpv->getContext());
    if (!draw || !(flags & MPV_RENDER_UPDATE_FRAME)) {
        GLTextureBuffer::onDraw(transform, draw);
        return;
    }

    int flip_y{0};
    mpv_opengl_fbo mpv_fbo{
            .fbo = (int) fbo,
            .w = (int) getSize().x, .h = (int) getSize().y,
            .internal_format = GL_RGBA8};
    mpv_render_param r_params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
            {MPV_RENDER_PARAM_FLIP_Y,     &flip_y},
            {MPV_RENDER_PARAM_INVALID,    nullptr}
    };
    mpv_render_context_render(mpv->getContext(), r_params);

    GLTextureBuffer::onDraw(transform, draw);
}
