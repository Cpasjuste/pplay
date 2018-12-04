//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"
#include "utility.h"

#define ITEM_HEIGHT 64

using namespace c2d;

Filer::Filer(Main *m, const std::string &path, const c2d::FloatRect &rect) : RectangleShape(rect) {

    main = m;

    // set default bg colors
    setFillColor(Color::Transparent);

    // highlight
    /*
    highlight = new Highlight({getSize().x, ITEM_HEIGHT}, Highlight::CursorPosition::Left);
    highlight->setFillColor({255, 255, 255, 40});
    add(highlight);
    */
    highlight = main->getHighlight();

    // items
    item_height = ITEM_HEIGHT;
    item_max = (int) (getSize().y / item_height);
    if ((item_max * item_height) < getSize().y) {
        item_height = getSize().y / (float) item_max;
    }

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        FloatRect r = {0, (item_height * i) + 1, getSize().x - 2, item_height - 2};
        items.emplace_back(new FilerItem(main, r));
        add(items[i]);
    }

    setSelection(0);
};

const MediaFile Filer::getSelection() const {

    if (!files.empty() && files.size() > (unsigned int) item_index) {
        return files[item_index];
    }

    return MediaFile();
}

void Filer::setSelection(int index) {

    item_index = index;
    int page = item_index / item_max;
    unsigned int index_start = (unsigned int) page * item_max;

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        if (index_start + i >= files.size()) {
            items[i]->setVisibility(Visibility::Hidden);
        } else {
            // set file
            items[i]->setFile(files[index_start + i]);
            items[i]->setVisibility(Visibility::Visible);
            if (index_start + i == (unsigned int) item_index) {
                highlight->setPosition(
                        items[i]->getGlobalBounds().left,
                        items[i]->getGlobalBounds().top);
            }
        }
    }

    if (files.empty()) {
        highlight->setVisibility(Visibility::Hidden);
    } else {
        highlight->setVisibility(Visibility::Visible);
    }
}

void Filer::tweenHighlight() {

    highlight->tweenPosition->setFromTo(
            highlight->getPosition(),
            {items[0]->getGlobalBounds().left, items[0]->getGlobalBounds().top});

    highlight->tweenPosition->play(TweenDirection::Forward, true);
}

void Filer::onInput(c2d::Input::Player *players) {

    if (!isVisible()
        || main->getMenu()->isVisible()
        || main->getPlayer()->isFullscreen()) {
        return;
    }

    unsigned int keys = players[0].state;

    if (keys & c2d::Input::Start || keys & c2d::Input::Select) {
        main->getMenu()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Up) {
        item_index--;
        if (item_index < 0)
            item_index = (int) (files.size() - 1);
        setSelection(item_index);
    } else if (keys & Input::Key::Down) {
        item_index++;
        if (item_index >= (int) files.size()) {
            item_index = 0;
        }
        setSelection(item_index);
    } else if (keys & Input::Key::Left) {
        main->getMenu()->tweenHighlight();
        main->getMenu()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Right) {
        if (main->getPlayer()->isPlaying() && !main->getPlayer()->isFullscreen()) {
            main->setPlayerFullscreen(true);
        }
    } else if (keys & Input::Key::Fire1) {
        if (getSelection().type == Io::Type::Directory) {
            enter();
        } else if (pplay::Utility::isMedia(getSelection())) {
            if (main->getPlayer()->load(getSelection())) {
                main->setPlayerFullscreen(true);
            }
        }
    } else if (keys & Input::Key::Fire2) {
        exit();
    }

    C2DObject::onInput(players);
}

std::string Filer::getPath() {
    return path;
}

Filer::~Filer() {
}
