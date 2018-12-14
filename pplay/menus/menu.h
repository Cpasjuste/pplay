//
// Created by cpasjuste on 20/10/18.
//

#ifndef PPLAY_OPTION_MENU_H
#define PPLAY_OPTION_MENU_H

#include "cross2d/skeleton/sfml/RectangleShape.hpp"
#include "highlight.h"

class Main;

class MenuItem {

public:

    enum class Position {
        Top,
        Bottom
    };

    MenuItem() = default;

    MenuItem(const std::string &name, const std::string &icon, const Position &position, int id = 0) {
        this->name = name;
        this->icon = icon;
        this->position = position;
        this->id = id;
    }

    int id = 0;
    std::string name;
    std::string icon;
    Position position = Position::Top;
};

class MenuButton : public c2d::RectangleShape {

public:

    MenuButton(Main *main, const MenuItem &item, const c2d::FloatRect &rect);

    MenuItem item;
    c2d::Texture *icon = nullptr;
    c2d::Text *name = nullptr;
};

class Menu : public c2d::RectangleShape {

public:

    explicit Menu(Main *main, const c2d::FloatRect &rect,
                  const std::string &title, const std::vector<MenuItem> &items, bool left);

    void setVisibility(c2d::Visibility visibility, bool tweenPlay) override;

    virtual void onOptionSelection(MenuItem *item) {};

    virtual MenuItem *getSelection();

protected:

    bool onInput(c2d::Input::Player *players) override;

    Main *main;
    c2d::Text *title;
    std::vector<MenuButton *> buttons;
    c2d::RectangleShape *menuButton;
    Highlight *highlight;
    c2d::TweenPosition *tween;
    int index = 0;
};

#endif //PPLAY_OPTION_MENU_H
