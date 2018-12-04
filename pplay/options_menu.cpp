//
// Created by cpasjuste on 20/10/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "options_menu.h"

using namespace c2d;

#define ICON_SIZE 24
#define BUTTON_HEIGHT 64

class OptionButton : public RectangleShape {

public:

    OptionButton(Main *main, const std::string &str, const std::string &iconName, const FloatRect &rect)
            : RectangleShape(rect) {

        setFillColor(Color::Transparent);
        setOrigin(Origin::Left);

        if (!iconName.empty()) {
            icon = new C2DTexture(main->getIo()->getDataPath() + "/skin/" + iconName);
            icon->setOrigin(Origin::Left);
            icon->setPosition(16, getSize().y / 2);
            icon->setFillColor(COLOR_FONT);
            add(icon);
        }

        text = new Text(str, 20, main->getFont());
        text->setOrigin(Origin::Left);
        text->setPosition(ICON_SIZE + 32, getSize().y / 2);
        text->setFillColor(COLOR_FONT);
        add(text);
    }

private:

    C2DTexture *icon;
    Text *text;
};

OptionMenu::OptionMenu(Main *m, const c2d::FloatRect &rect) : RectangleShape(rect) {

    main = m;

    setFillColor(COLOR_BG);
    setAlpha(245);
    setOutlineColor(Color::GrayLight);
    setOutlineThickness(2);

    // menu button
    menuButton = new RectangleShape({16, 16, 40, 40});
    menuButton->setFillColor(COLOR_BG);
    menuButton->setAlpha(245);
    menuButton->setOutlineColor(Color::GrayLight);
    menuButton->setOutlineThickness(2);
    auto *tex = new C2DTexture(main->getIo()->getDataPath() + "/skin/menu.png");
    tex->setFillColor(COLOR_FONT);
    tex->setOrigin(Origin::Center);
    tex->setPosition(menuButton->getSize().x / 2, menuButton->getSize().y / 2);
    menuButton->add(tex);
    main->getMainRect()->add(menuButton);

    // highlight
    highlight = new Highlight({getSize().x, BUTTON_HEIGHT});
    highlight->setOrigin(Origin::Left);
    highlight->setPosition(0, 200);
    add(highlight);

    // title
    title = new Text("PPLAY______", 24, main->getFont());
    title->setStyle(Text::Underlined);
    title->setPosition(32, 32);
    title->setFillColor(COLOR_FONT);
    add(title);

    // options
    FloatRect r = {0, 200, getSize().x, BUTTON_HEIGHT};
    options[Home] = new OptionButton(main, "Home", "home.png", r);
    add(options[Home]);
    r.top += 64;
    options[Network] = new OptionButton(main, "Network", "network.png", r);
    add(options[Network]);
    r.top = getSize().y - 32;
    options[Exit] = new OptionButton(main, "Exit", "exit.png", r);
    add(options[Exit]);

    // tween!
    add(new TweenPosition({-rect.width, 0}, {0, 0}, 0.5f));
}

void OptionMenu::onInput(c2d::Input::Player *players) {

    if (!isVisible()) {
        return;
    }

    int keys = players[0].state;

    if (keys & Input::Touch) {
        if (options[Home]->getGlobalBounds().contains(players[0].touch)) {
            index = Home;
            setVisibility(Visibility::Hidden, true);
            main->show(Main::MenuType::Home);
        } else if (options[Network]->getGlobalBounds().contains(players[0].touch)) {
            index = Network;
            setVisibility(Visibility::Hidden, true);
            main->show(Main::MenuType::Network);
        } else if (options[Exit]->getGlobalBounds().contains(players[0].touch)) {
            main->quit();
        } else if (!getGlobalBounds().contains(players[0].touch)) {
            setVisibility(Visibility::Hidden, true);
        }
    } else {
        if (keys & Input::Up) {
            index--;
            if (index < 0) {
                index = OPT_COUNT - 1;
            }
        } else if (keys & Input::Down) {
            index++;
            if (index == OPT_COUNT) {
                index = 0;
            }
        } else if (keys & Input::Fire1) {
            if (index == Home) {
                setVisibility(Visibility::Hidden, true);
                main->show(Main::MenuType::Home);
            } else if (index == Network) {
                setVisibility(Visibility::Hidden, true);
                main->show(Main::MenuType::Network);
            } else if (index == Exit) {
                main->quit();
            }
        } else if (keys & Input::Fire2
                   || keys & Input::Right
                   || keys & c2d::Input::Start
                   || keys & c2d::Input::Select) {
            setVisibility(Visibility::Hidden, true);
        }
    }

    highlight->setPosition(0, options[index]->getPosition().y);
}

void OptionMenu::setVisibility(c2d::Visibility visibility, bool tweenPlay) {

    menuButton->setVisibility(
            visibility == Visibility::Visible ? Visibility::Hidden : Visibility::Visible, false);

    C2DObject::setVisibility(visibility, true);
}

c2d::RectangleShape *OptionMenu::getMenuButton() {
    return menuButton;
}
