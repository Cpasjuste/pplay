//
// Created by cpasjuste on 09/12/18.
//

#include "main.h"
#include "video_texture.h"

using namespace c2d;

VideoTexture::VideoTexture(Main *m, const c2d::Vector2f &size) : GLTextureBuffer(size, Format::RGBA8) {
    main = m;

    // fade
    fade = new C2DTexture(main->getIo()->getRomFsPath() + "skin/fade.png");
    fade->setScale(size.x / fade->getSize().x, size.y / fade->getSize().y);
    fade->setFillColor(Color::Black);
    fade->setAlpha(0);
    fadeTween = new TweenAlpha(0, 255, 0.5f);
    fade->add(fadeTween);
    GLTextureBuffer::add(fade);
}

int VideoTexture::resize(const Vector2i &size, bool keepPixels) {
    fade->setScale((float) size.x / fade->getSize().x, (float) size.y / fade->getSize().y);
    return GLTextureBuffer::resize(size, keepPixels);
}

void VideoTexture::hideFade() {
    fadeTween->play(TweenDirection::Backward);
}

void VideoTexture::showFade() {
    fadeTween->play(TweenDirection::Forward);
}

void VideoTexture::onDraw(c2d::Transform &transform, bool draw) {

    // sync
    while (!(mpv_render_context_update(main->getPlayer()->getMpv()->getContext()) & MPV_RENDER_UPDATE_FRAME)) {
        if (main->getPlayer()->getMpv()->isPaused()
            || main->getPlayer()->getMpv()->isStopped()) {
            return GLTextureBuffer::onDraw(transform, draw);
        }
        main->delay(1);
    }

    if (draw) {
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

        mpv_render_context_render(main->getPlayer()->getMpv()->getContext(), r_params);
#ifdef FULL_TEXTURE_TEST
        // mpv change viewport, restore
        glViewport(0, 0, (GLsizei) main->getSize().x, (GLsizei) main->getSize().y);
#endif
    }

    GLTextureBuffer::onDraw(transform, draw);
}
