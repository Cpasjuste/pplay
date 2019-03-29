//
// Created by cpasjuste on 29/03/19.
//

#include "cross2d/c2d.h"
#include "scrapper.h"
#include "p_search.h"

using namespace c2d;
using namespace pplay;

static bool scrap_dir(const std::string &path) {

    std::vector<Io::File> files;

    while (true) {
        files = c2d_renderer->getIo()->getDirList(path);
        if (files.empty()) {
            return false;
        }
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if(files.name)
    }

}

static int scrap_thread(void *ptr) {

    Scrapper *scrapper = (Scrapper *) ptr;

    std::vector<Io::File> files;

    //while (files = c2d_renderer)

    return 0;
}

Scrapper::Scrapper(const std::string &p) {

    path = p;
}

void Scrapper::scrap() {

    if (scrapping) {
        return;
    }

    scrapping = true;
    thread = SDL_CreateThread(scrap_thread, "scrap_thread", (void *) this);
}
