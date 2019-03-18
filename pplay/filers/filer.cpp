//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"
#include "utility.h"

#define ITEM_HEIGHT 90

using namespace c2d;

Filer::Filer(Main *m, const std::string &path, const c2d::FloatRect &rect) : Rectangle(rect) {

    main = m;

    // highlight
    highlight = new Highlight({getSize().x, ITEM_HEIGHT * main->getScaling()}, Highlight::CursorPosition::Left);
    add(highlight);

    // items
    item_height = ITEM_HEIGHT * main->getScaling();
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
    add(new TweenAlpha(0, 255, 0.5f));
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
            // media info only cached in FilerSdmc now..
            if (files[idx].type == Io::Type::File && !files[idx].getMedia().isLoaded()) {
                files[idx].media = main->getMediaThread()->getMediaInfo(files[idx], true);
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

void Filer::onDraw(c2d::Transform &transform, bool draw) {

    setSelection(item_index);
    Rectangle::onDraw(transform);
}

bool Filer::onInput(c2d::Input::Player *players) {

    if (main->getMenuMain()->isMenuVisible() || main->getPlayer()->isFullscreen()) {
        return false;
    }

    unsigned int keys = players[0].keys;

    if (keys & c2d::Input::Start || keys & c2d::Input::Select) {
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
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
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Right) {
        if (!main->getPlayer()->isStopped() && !main->getPlayer()->isFullscreen()) {
            main->getPlayer()->setFullscreen(true);
        }
    } else if (keys & Input::Key::Fire1) {
        if (getSelection().type == Io::Type::Directory) {
            enter(item_index);
        } else if (pplay::Utility::isMedia(getSelection())) {
            std::string msg = "Loading..." + getSelection().name;
            main->getStatus()->show("Please Wait...", msg, true, true);
            if (!files[item_index].media.isLoaded()) {
                files[item_index].setMedia(main->getMediaThread()->getMediaInfo(files[item_index], false, true));
            }
            if (main->getPlayer()->load(files[item_index])) {
                main->getPlayer()->setFullscreen(true);
                main->getStatus()->hide();
            }
        }
    } else if (keys & Input::Key::Fire2) {
        exit();
    }

    return true;
}

void Filer::enter(int index) {
    item_index_prev.push_back(index);
}

void Filer::exit() {
    if (!item_index_prev.empty()) {
        int last = (int) item_index_prev.size() - 1;
        if (item_index_prev[last] < (int) files.size()) {
            item_index = item_index_prev[last];
        }
        item_index_prev.erase(item_index_prev.end() - 1);
    }
}

void Filer::clearHistory() {
    item_index_prev.clear();
}

std::string Filer::getPath() {
    return path;
}

Filer::~Filer() {
}
