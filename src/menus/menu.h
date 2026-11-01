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

    MenuItem(const std::string &name, const std::string &icon, const Position &position, int id = 0,
             const std::string &value = "") {
        this->name = name;
        this->icon = icon;
        this->position = position;
        this->id = id;
        this->value = value;
    }

    // text written to the configuration file by the options submenus (the name when empty)
    const std::string &getValue() const {
        return value.empty() ? name : value;
    }

    int id = 0;
    std::string name;
    std::string value;
    std::string icon;
    Position position = Position::Top;
};

class MenuButton : public c2d::Rectangle {

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

    void reset();

    void setVisibility(c2d::Visibility visibility, bool tweenPlay = true) override;

    virtual void onOptionSelection(MenuItem *item) {};

    virtual MenuItem *getSelection();

    virtual std::vector<MenuButton *> getButtons() {
        return buttons;
    }

protected:

    bool onInput(c2d::Input::Player *players) override;

    // scroll "top" buttons so the selected one stays in the middle (lists taller than the menu)
    void updateScroll();

    // move the cursor to a button without animation (and scroll to it)
    void setIndex(int i);

    // called after buttons have been moved/hidden by updateScroll
    virtual void onScroll() {}

    int scrollIndex = 0;
    Main *main;
    c2d::Text *title;
    c2d::Text *counter = nullptr;
    std::vector<MenuButton *> buttons;
    c2d::RectangleShape *menuButton;
    Highlight *highlight;
    c2d::TweenPosition *tween;
    int index = 0;
};

#endif //PPLAY_OPTION_MENU_H
