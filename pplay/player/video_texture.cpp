//
// Created by cpasjuste on 09/12/18.
//

#include "cross2d/c2d.h"
#include "video_texture.h"

using namespace c2d;

VideoTexture::VideoTexture(const c2d::Vector2f &size, Player::Mpv *_mpv) : GLTextureBuffer(size, Format::RGBA8) {

    mpv = _mpv;

    float l_width = getSize().x / 5;
    rectLeft = new GradientRectangle({(l_width / 2) - 2, size.y / 2, l_width, size.y});
    rectLeft->setOrigin(Origin::Center);
    rectLeft->setRotation(180);
    rectLeft->setColor(Color::Black, Color::Transparent);
    rectLeft->setAlpha(0);
    rectLeftTween = new TweenAlpha(0, 255, 0.5f);
    rectLeft->add(rectLeftTween);
    add(rectLeft);

    float b_height = getSize().y / 5;
    rectBottom = new GradientRectangle({size.x / 2, (size.y - (b_height / 2)) + 2, b_height, size.x});
    rectBottom->setOrigin(Origin::Center);
    rectBottom->setRotation(90);
    rectBottom->setColor(Color::Black, Color::Transparent);
    rectBottom->setAlpha(0);
    rectBottomTween = new TweenAlpha(0, 255, 0.5f);
    rectBottom->add(rectBottomTween);
    add(rectBottom);
}

void VideoTexture::hideGradients() {
    rectLeftTween->play(TweenDirection::Backward);
    rectBottomTween->play(TweenDirection::Backward);
}

void VideoTexture::showGradients() {
    rectLeftTween->play(TweenDirection::Forward);
    rectBottomTween->play(TweenDirection::Forward);
}

void VideoTexture::onDraw(c2d::Transform &transform, bool draw) {

    if (draw && mpv && mpv->available) {
        int flip_y{0};
        mpv_opengl_fbo mpv_fbo{
                .fbo = fbo,
                .w = (int) getSize().x, .h = (int) getSize().y,
                .internal_format = GL_RGBA8};
        mpv_render_param r_params[] = {
                {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
                {MPV_RENDER_PARAM_FLIP_Y,     &flip_y},
                {MPV_RENDER_PARAM_INVALID,    nullptr}
        };

        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        mpv_render_context_render(mpv->ctx, r_params);
        glViewport(vp[0], vp[1], (GLsizei) vp[2], (GLsizei) vp[3]);
    }

    GLTextureBuffer::onDraw(transform, draw);
}
