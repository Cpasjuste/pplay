//
// Created by cpasjuste on 12/04/18.
//

#if 0

#include <iostream>

#include "main.h"
#include "filer_paths.h"

using namespace c2d;

FilerPaths::FilerPaths(PPLAYConfig *config, Font *font, int fontSize, const FloatRect &rect)
        : Filer("", font, fontSize, rect) {

    setOutlineThickness(0);
    pathRect->setVisibility(Visibility::Hidden);

    // path selection
    std::vector<Io::File> paths;
    paths.emplace_back("sdmc:", "sdmc:");
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
    listBox->setFillColor({38, 38, 38, 255});
    listBox->setOutlineColor(COLOR_ORANGE);
    listBox->getHighlight()->setFillColor(COLOR_GRAY_LIGHT);
    listBox->getHighlight()->setOutlineColor(Color::Orange);
    listBox->getHighlight()->setOutlineThickness(2);
    listBox->getHighlight()->add(new TweenAlpha(80, 150, 0.6f, TweenLoop::PingPong));
    add(listBox);

    // for fun
    add(new TweenScale({1, 0}, {1, 1}, 0.3f));
}

void FilerPaths::enter() {

    //Io::File *file = listBox->getSelection();

}
#endif