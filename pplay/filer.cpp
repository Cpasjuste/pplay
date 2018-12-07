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
    highlight = new Highlight({getSize().x, ITEM_HEIGHT}, Highlight::CursorPosition::Left);
    highlight->setFillColor({255, 255, 255, 40});
    add(highlight);

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

    // tween
    add(new TweenAlpha(0, 255, 1));
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
            // load media info, set file
            int idx = index_start + i;
            if (files[idx].type == Io::Type::File && !files[idx].getMedia().isLoaded()) {
                //printf("Filer: getting media info for %s\n", files[idx].name.c_str());
                files[idx].media = main->getMediaThread()->getMediaInfo(files[idx]);
            }
            items[i]->setFile(files[idx]);
            items[i]->setVisibility(Visibility::Visible);
            // set highlight position
            if (index_start + i == (unsigned int) item_index) {
                highlight->setPosition(items[i]->getPosition());
            }
        }
    }

    if (files.empty()) {
        highlight->setVisibility(Visibility::Hidden);
    } else {
        highlight->setVisibility(Visibility::Visible);
    }
}

void Filer::onDraw(c2d::Transform &transform) {

    setSelection(item_index);
    Shape::onDraw(transform);
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
    } else if (keys & Input::Key::Down) {
        item_index++;
        if (item_index >= (int) files.size()) {
            item_index = 0;
        }
    } else if (keys & Input::Key::Left) {
        main->getMenu()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Right) {
        if (main->getPlayer()->isPlaying() && !main->getPlayer()->isFullscreen()) {
            main->setPlayerFullscreen(true);
        }
    } else if (keys & Input::Key::Fire1) {
        if (getSelection().type == Io::Type::Directory) {
            enter();
        } else if (pplay::Utility::isMedia(getSelection())) {
            main->getMessageBox()->show("Please Wait", "Media is loading...");
            if (main->getPlayer()->load(getSelection())) {
                main->setPlayerFullscreen(true);
            }
            main->getMessageBox()->hide();
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
