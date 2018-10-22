//
// Created by cpasjuste on 12/04/18.
//

#include <iostream>

#include "main.h"
#include "filer_paths.h"

using namespace c2d;

FilerPaths::FilerPaths(PPLAYConfig *config, Font *font, int fontSize, const FloatRect &rect)
        : Filer("", font, fontSize, rect) {

    pathRect->setVisibility(Visibility::Hidden);

    // path selection
    std::vector<Io::File> paths;
    paths.emplace_back("local", "local");
    config::Group *group = config->getGroup("HTTP_SERVERS");
    for (config::Option &option : *group->getOptions()) {
        if (!option.getString().empty()) {
            paths.emplace_back(option.getString(), option.getString());
        }
    }

    delete (listBox);
    FloatRect r = rect;
    r.height = FONT_SIZE * paths.size() + 10 * paths.size();
    listBox = new ListBox(font, fontSize, r, paths);
    listBox->setPosition(0, 4);
    Color c = Color::Black;
    c.a = 120;
    listBox->setFillColor(c);
    add(listBox);
    setOutlineColor(COLOR_ORANGE);

    // for fun
    add(new TweenScale({1, 0}, {1, 1}, 0.2f));
}

void FilerPaths::enter() {

    Io::File file = listBox->getSelection();

}

void FilerPaths::exit() {

}
