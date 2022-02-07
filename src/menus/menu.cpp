//
// Created by cpasjuste on 20/10/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "menu.h"
#include "utility.h"

using namespace c2d;

MenuButton::MenuButton(Main *main, const MenuItem &item, const FloatRect &rect) : Rectangle(rect) {

    this->item = item;
    MenuButton::setOrigin(Origin::Left);

    if (!item.icon.empty()) {
        icon = new C2DTexture(main->getIo()->getRomFsPath() + "skin/" + item.icon);
        icon->setOrigin(Origin::Left);
        icon->setPosition(16 * main->getScaling().x, MenuButton::getSize().y / 2);
        icon->setScale(main->getScaled(main->getSize().x / 1920, main->getSize().y / 1080));
        icon->setFillColor(COLOR_FONT);
        MenuButton::add(icon);
    }

    name = new Text(item.name, main->getFontSize(Main::FontSize::Medium), main->getFont());
    name->setOrigin(Origin::Left);
    name->setFillColor(COLOR_FONT);
    if (!item.icon.empty()) {
        name->setPosition((ICON_SIZE + 32) * main->getScaling().x, MenuButton::getSize().y / 2);
        name->setSizeMax((MenuButton::getSize().x - ICON_SIZE + 32) * main->getScaling().x, 0);
    } else {
        name->setPosition(16 * main->getScaling().x, MenuButton::getSize().y / 2);
        name->setSizeMax((MenuButton::getSize().x - (16 * main->getScaling().x)) * main->getScaling().x, 0);
    }
    MenuButton::add(name);
}

Menu::Menu(Main *m, const c2d::FloatRect &rect, const std::string &_title,
           const std::vector<MenuItem> &items, bool left) : RectangleShape(rect) {
    main = m;

    Menu::setFillColor(COLOR_BG);
    Menu::setAlpha(245);
    Menu::setOutlineColor(Color::GrayLight);
    Menu::setOutlineThickness(main->getScaling().y * 2);

    // highlight
    highlight = new Highlight({Menu::getSize().x, BUTTON_HEIGHT * main->getScaling().y});
    highlight->setOrigin(Origin::Left);
    highlight->setPosition(0, 200 * main->getScaling().y);
    Menu::add(highlight);

    // title
    title = new Text(_title, main->getFontSize(Main::FontSize::Big), main->getFont());
    title->setStyle(Text::Underlined);
    title->setPosition(main->getScaled(32, 32));
    title->setFillColor(COLOR_FONT);
    Menu::add(title);

    // options
    FloatRect top = {0, 200 * main->getScaling().y,
                     Menu::getSize().x, BUTTON_HEIGHT * main->getScaling().y};
    FloatRect bottom = {0, Menu::getSize().y - (32 * main->getScaling().y),
                        Menu::getSize().x, BUTTON_HEIGHT * main->getScaling().y};

    for (auto &item: items) {
        if (item.position == MenuItem::Position::Top) {
            auto *option = new MenuButton(main, item, top);
            add(option);
            buttons.push_back(option);
            top.top += 64 * main->getScaling().y;
        } else {
            auto *option = new MenuButton(main, item, bottom);
            Menu::add(option);
            buttons.push_back(option);
            bottom.top -= 64 * main->getScaling().y;
        }
    }

    // tween!
    if (left) {
        Menu::add(new TweenPosition({-rect.width, 0}, {0, 0}, 0.2f));
    } else {
        Menu::add(new TweenPosition({main->getSize().x, 0},
                                    {main->getSize().x - Menu::getSize().x, 0}, 0.2f));
    }
}

bool Menu::onInput(c2d::Input::Player *players) {
    unsigned int keys = players[0].keys;

    if (keys & Input::Touch) {
        Vector2f touch = players[0].touch;
        if (!getGlobalBounds().contains(touch)) {
            setVisibility(Visibility::Hidden, true);
        } else {
            for (unsigned int i = 0; i < buttons.size(); i++) {
                if (buttons.at(i)->getGlobalBounds().contains(touch)) {
                    onOptionSelection(&buttons.at(i)->item);
                    index = i;
                    break;
                }
            }
        }
    } else {
        if (keys & Input::Up) {
            index--;
            if (index < 0) {
                index = (int) buttons.size() - 1;
            }
        } else if (keys & Input::Down) {
            index++;
            if (index == (int) buttons.size()) {
                index = 0;
            }
        } else if (keys & Input::Fire1) {
            onOptionSelection(&buttons[index]->item);
        }
    }

    highlight->tweenTo({0, buttons[index]->getPosition().y});

    return true;
}

void Menu::setVisibility(c2d::Visibility visibility, bool tweenPlay) {
    C2DObject::setVisibility(visibility, true);
}

MenuItem *Menu::getSelection() {
    return &buttons[index]->item;
}

void Menu::reset() {
    index = 0;
    highlight->tweenTo({0, buttons[index]->getPosition().y});
}
