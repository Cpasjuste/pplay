//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"

using namespace c2d;

Filer::Filer(const std::string &path, Font *font,
             int fontSize, const c2d::FloatRect &rect) : C2DRectangle(rect) {

    this->path = path;
    this->setFillColor(Color::Transparent);

    // create current path box
    pathRect = new OutlineRect({rect.width, fontSize + 16});
    pathRect->setFillColor(COLOR_GRAY_DARK);
    pathRect->setOutlineColor(COLOR_ORANGE);
    pathRect->setOutlineThickness(2);
    pathText = new C2DText("CURRENT PATH: /", (unsigned int) fontSize, font);
    pathText->setOutlineThickness(1);
    pathText->setOrigin(Origin::Left);
    pathText->setPosition(4, (pathRect->getSize().y / 2));
    pathText->setWidth(rect.width - 8);
    pathRect->add(pathText);
    add(pathRect);

    // add the filer listbox
    float y = pathRect->getGlobalBounds().top + pathRect->getGlobalBounds().height;
    FloatRect r = {0, y + 8, rect.width, rect.height - y - 8};
    listBox = new ListBox(font, fontSize, r, std::vector<Io::File>());
    listBox->setFillColor(COLOR_GRAY_DARK);
    listBox->setOutlineColor(COLOR_BLUE);
    listBox->setOutlineThickness(2);
    listBox->setTextOutlineThickness(1);
    listBox->getHighlight()->setFillColor(COLOR_GRAY_LIGHT);
    listBox->getHighlight()->setOutlineColor(COLOR_BLUE);
    listBox->getHighlight()->setOutlineThickness(2);
    listBox->getHighlight()->add(new TweenAlpha(80, 150, 0.6f, TweenLoop::PingPong));
    auto *border = new RectangleShape(listBox->getLocalBounds());
    border->setFillColor(Color::Transparent);
    border->setOutlineColor(Color::Black);
    border->setOutlineThickness(1);
    border->setSize(r.width + 4, r.height + 4);
    listBox->add(border);

    add(listBox);
}

bool Filer::step(unsigned int keys) {

    if (keys & Input::Key::KEY_UP) {
        up();
    } else if (keys & Input::Key::KEY_DOWN) {
        down();
    } else if (keys & Input::Key::KEY_RIGHT) {
        right();
    } else if (keys & Input::Key::KEY_LEFT) {
        left();
    } else if (keys & Input::Key::KEY_FIRE1) {
        if (getSelection().type == Io::Type::File) {
            return true;
        }
        enter();
    } else if (keys & Input::Key::KEY_FIRE2) {
        exit();
    }

    return false;
}

c2d::Io::File Filer::getSelection() {
    return listBox->getSelection();
}

void Filer::down() {
    index++;
    if (index >= (int) listBox->getFiles().size()) {
        index = 0;
    }
    listBox->setSelection(index);
}

void Filer::up() {
    index--;
    if (index < 0)
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

void Filer::left() {
    index -= listBox->getMaxLines();
    if (index < 0)
        index = 0;
    listBox->setSelection(index);
}

void Filer::right() {
    index += listBox->getMaxLines();
    if (index >= (int) listBox->getFiles().size())
        index = (int) (listBox->getFiles().size() - 1);
    listBox->setSelection(index);
}

std::string Filer::getPath() {
    return path;
}

c2d::ListBox *Filer::getListBox() {
    return listBox;
}

Filer::~Filer() {
    files.clear();
}

