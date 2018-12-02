//
// Created by cpasjuste on 20/10/18.
//

#include "cross2d/c2d.h"
#include "main.h"
#include "options_menu.h"

using namespace c2d;

#define FONT_COLOR {200, 200, 200, 255}

class Highlight : public RectangleShape {

public:

    Highlight(const Vector2f &size) : RectangleShape(size) {

        setFillColor({255, 255, 255, 80});
        add(new TweenAlpha(40, 60, 1, TweenLoop::PingPong));

        RectangleShape *rect = new RectangleShape(Vector2f{6, size.y});
        rect->setFillColor(COLOR_BLUE);
        rect->setAlpha(200);
        rect->move(size.x - 4, 0);
        add(rect);
    }
};

OptionMenu::OptionMenu(Main *m, const c2d::FloatRect &rect) : RectangleShape(rect) {

    main = m;

    setFillColor(Color::GrayDark);
    setAlpha(245);
    setOutlineColor(Color::GrayLight);
    setOutlineThickness(2);

    title = new Text("PPLAY", 24, main->getFont());
    title->setPosition(32, 32);
    title->setFillColor(FONT_COLOR);
    title->setAlpha(245);
    add(title);

    options[Home] = new Text("Home", 20, main->getFont());
    options[Home]->setPosition(64, 200);
    options[Home]->setFillColor(FONT_COLOR);
    C2DTexture *icon = new C2DTexture(main->getIo()->getDataPath() + "/skin/home.png");
    icon->setOrigin(Origin::Right);
    icon->setPosition(options[Home]->getPosition().x - 16, options[Home]->getPosition().y);
    icon->setFillColor(FONT_COLOR);
    add(icon);

    options[Network] = new Text("Network", 20, main->getFont());
    options[Network]->setPosition(64, 200 + 64 * 1);
    options[Network]->setFillColor(FONT_COLOR);
    icon = new C2DTexture(main->getIo()->getDataPath() + "/skin/network.png");
    icon->setOrigin(Origin::Right);
    icon->setPosition(options[Network]->getPosition().x - 16, options[Network]->getPosition().y);
    icon->setFillColor(FONT_COLOR);
    add(icon);

    options[Exit] = new Text("Exit", 20, main->getFont());
    options[Exit]->setPosition(32, getSize().y - 32);
    options[Exit]->setFillColor(FONT_COLOR);

    for (auto &option : options) {
        option->setOrigin(Origin::Left);
        option->setAlpha(245);
        add(option);
    }

    highlight = new Highlight({getSize().x, 64});
    highlight->setOrigin(Origin::Left);
    highlight->setPosition(0, options[index]->getPosition().y);
    add(highlight);

    tween = new TweenPosition({-rect.width, 0}, {0, 0}, 0.5f);
    add(tween);
}

void OptionMenu::onInput(c2d::Input::Player *players) {

    if (!isVisible()) {
        return;
    }

    int keys = players[0].state;

    if (keys & Input::KEY_TOUCH) {
        if (options[Home]->getGlobalBounds().contains(players[0].touch)) {
            index = Home;
            setVisibility(Visibility::Hidden, true);
            main->showHome();
        } else if (options[(int) Index::Exit]->getGlobalBounds().contains(players[0].touch)) {
            main->quit();
        }
    } else {
        if (keys & Input::KEY_UP) {
            index--;
            if (index < 0) {
                index = OPT_COUNT - 1;
            }
        } else if (keys & Input::KEY_DOWN) {
            index++;
            if (index == OPT_COUNT) {
                index = 0;
            }
        } else if (keys & Input::KEY_FIRE1) {
            if (index == (int) Index::Home) {
                setVisibility(Visibility::Hidden, true);
                main->showHome();
            } else if (index == (int) Index::Exit) {
                main->quit();
            }
        } else if (keys & Input::KEY_FIRE2
                   || keys & c2d::Input::KEY_START
                   || keys & c2d::Input::KEY_COIN) {
            setVisibility(Visibility::Hidden, true);
        }
    }

    highlight->setPosition(0, options[index]->getPosition().y);
}
