//
// Created by cpasjuste on 04/12/18.
//

#ifndef PPLAY_HIGHLIGHT_H
#define PPLAY_HIGHLIGHT_H

class Highlight : public c2d::RectangleShape {

public:

    enum class CursorPosition {
        Left,
        Right
    };

    explicit Highlight(const c2d::Vector2f &size, const CursorPosition &cursor = CursorPosition::Right);

    void setFillColor(const c2d::Color &color) override;

    void setCursorColor(const c2d::Color &color);

private:

    c2d::Texture *texture;
    c2d::RectangleShape *cursor;
};

#endif //PPLAY_HIGHLIGHT_H
