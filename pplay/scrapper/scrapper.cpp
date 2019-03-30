//
// Created by cpasjuste on 29/03/19.
//

#include <utility.h>
#include "main.h"
#include "scrapper.h"
#include "p_search.h"

using namespace c2d;
using namespace pplay;
using namespace pscrap;

#define API_KEY "80435e61cc52ad8850355379108a92d0"

static void scrap_dir(Main *main, const std::string &path) {

    std::vector<Io::File> files;

    files = main->getIo()->getDirList(path);
    if (files.empty()) {
        return;
    }

    for (auto &file : files) {
        if (!main->getScrapper()->running) {
            break;
        }
        if (file.type == Io::Type::Directory) {
            if (file.name == "." || file.name == "..") {
                continue;
            }
            scrap_dir(main, file.path);
        } else {
            std::string scrap_path = pplay::Utility::getMediaScrapPath(file);
            if (!main->getIo()->exist(scrap_path)) {
                main->getStatus()->show("Scrapping...", "Searching: " + file.name, true);
                Search search(API_KEY, c2d::Utility::removeExt(file.name));
                int res = search.get();
                if (res == 0) {
                    search.save(scrap_path);
                    if (search.total_results > 0) {
                        printf("%s\n", search.movies.at(0).title.c_str());
                        main->getStatus()->show("Scrapping...", "Downloading poster: "
                                                                + search.movies.at(0).title, true);
                        search.movies.at(0).getPoster(pplay::Utility::getMediaPosterPath(file));
                        main->getStatus()->show("Scrapping...", "Downloading backdrop: "
                                                                + search.movies.at(0).title, true);
                        search.movies.at(0).getBackdrop(pplay::Utility::getMediaBackdropPath(file), 780);
                    }
                }
            }
        }
    }

    main->getStatus()->hide();
}

static int scrap_thread(void *ptr) {

    auto scrapper = (Scrapper *) ptr;

    while (scrapper->running) {

        SDL_LockMutex(scrapper->mutex);
        SDL_CondWait(scrapper->cond, scrapper->mutex);

        if (scrapper->running) {
            printf("START: scrap_dir: %s\n", scrapper->path.c_str());
            scrapper->scrapping = true;
            scrap_dir(scrapper->main, scrapper->path);
            scrapper->scrapping = false;
            printf("END: scrap_dir: %s\n", scrapper->path.c_str());
        }

        SDL_UnlockMutex(scrapper->mutex);
    }

    return 0;
}

Scrapper::Scrapper(Main *m) {

    main = m;
    mutex = SDL_CreateMutex();
    cond = SDL_CreateCond();
    thread = SDL_CreateThread(scrap_thread, "scrap_thread", (void *) this);
}

int Scrapper::scrap(const std::string &p) {

    if (scrapping) {
        return -1;
    }

    path = p;
    SDL_CondSignal(cond);

    return 0;
}

Scrapper::~Scrapper() {

    scrapping = false;
    running = false;
    SDL_CondSignal(cond);
    SDL_WaitThread(thread, nullptr);
    SDL_DestroyCond(cond);
    printf("Scrapper::~Scrapper\n");
}
