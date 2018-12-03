//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"

using namespace c2d;

Filer::Filer(Main *main, const std::string &path, const c2d::FloatRect &rect) : RectangleShape(rect) {

    // set default bg colors
    setFillColor(Color::GrayLight);

    item_height = 64;

    item_max = (int) (getSize().y / item_height);
    if ((item_max * item_height) < getSize().y) {
        item_height = getSize().y / (float) item_max;
    }

    // add lines of text
    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        FloatRect r = {1, (item_height * i) + 1, getSize().x - 2, item_height - 2};
        items.emplace_back(new FilerItem(main, r));
        add(items[i]);
    }

    setSelection(0);
};

const FilerItem Filer::getSelection() const {

    if (!files.empty() && files.size() > (unsigned int) item_index) {
        return *items[item_index];
    }

    return FilerItem(main, {});
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
            Io::File *file = files[index_start + i];
            items[i]->setVisibility(Visibility::Visible);
            items[i]->setString(file->name);
            // set text color based on file color
            items[i]->setIcon(file->icon);
            items[i]->setColor(file->color);
            // set highlight position and color
            if (index_start + i == (unsigned int) item_index) {
                highlight->setPosition(items[i]->getPosition());
                Color color = highlight_use_files_color ?
                              file->color : highlight->getFillColor();
                color.a = highlight->getAlpha();
                highlight->setFillColor(color);
                color = highlight_use_files_color ?
                        file->color : highlight->getOutlineColor();
                color.a = highlight->getAlpha();
                highlight->setOutlineColor(color);
            }
        }
    }

    if (files.empty()) {
        highlight->setVisibility(Visibility::Hidden, false);
    } else {
        if (use_highlight) {
            highlight->setVisibility(Visibility::Visible, false);
        }
    }
}

bool Filer::step(unsigned int keys) {

    if (keys & c2d::Input::KEY_START || keys & c2d::Input::KEY_COIN) {
        main->getMenu()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::KEY_UP) {
        up();
    } else if (keys & Input::Key::KEY_DOWN) {
        down();
    } else if (keys & Input::Key::KEY_LEFT) {
        main->getMenu()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::KEY_RIGHT) {
        if (main->getPlayer()->isPlaying() && !main->getPlayer()->isFullscreen()) {
            main->setPlayerFullscreen(true);
        }
    } else if (keys & Input::Key::KEY_FIRE1) {
        if (getSelection().getFile().type == Io::Type::File) {
            return true;
        }
        enter();
    } else if (keys & Input::Key::KEY_FIRE2) {
        exit();
    }

    return false;
}

void Filer::down() {

    item_index++;
    if (item_index >= (int) items.size()) {
        item_index = 0;
    }

    setSelection(item_index);
}

void Filer::up() {

    item_index--;
    if (item_index < 0)
        item_index = (int) (items.size() - 1);

    setSelection(item_index);
}

std::string Filer::getPath() {
    return path;
}

Filer::~Filer() {
}
