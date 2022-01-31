//
// Created by cpasjuste on 29/03/19.
//

#include <utility.h>
#include "main.h"
#include "scrapper.h"
#include "p_search.h"

using namespace pplay;
using namespace pscrap;

static std::vector<c2d::Io::File> scrapList;

static void find_medias(Main *main, const std::string &path) {

    std::vector<std::string> ext = pplay::Utility::getMediaExtensions();
    pplay::Io::DeviceType type = ((pplay::Io *) main->getIo())->getDeviceType(path);
    std::vector<c2d::Io::File> files =
            ((pplay::Io *) main->getIo())->getDirList(type, ext, path);

    if (files.empty()) {
        return;
    }

    for (auto &file: files) {
        if (!main->getScrapper()->running) {
            break;
        }
        if (file.type == c2d::Io::Type::Directory) {
            if (file.name == "." || file.name == "..") {
                continue;
            }
            find_medias(main, file.path);
        } else {
            scrapList.emplace_back(file);
        }
    }
}

#define TOKEN_COUNT 11
static const char *tokens[TOKEN_COUNT] = {
        "720p", "1080p", "2160p", "hdrip",
        "dvdrip", "bdrip", "xvid", "divx",
        "web-dl", "webrip", "bluray"
};

static std::string clean_name(const std::string &name) {
    // try to cut movie name to speed up "reverse" search
    // will work until 2030 :)
    bool cut = false;
    std::string search = c2d::Utility::toLower(name);
    search = c2d::Utility::removeExt(search);
    for (int j = 2030; j > 1969; j--) {
        size_t pos = search.rfind(std::to_string(j));
        if ((pos != std::string::npos) && (pos > 3)) {
            search = search.substr(0, pos - 1);
            cut = true;
            break;
        }
    }
    if (!cut) {
        for (auto &token: tokens) {
            if (token != nullptr) {
                size_t pos = search.rfind(token);
                if ((pos != std::string::npos) && (pos > 1)) {
                    search = search.substr(0, pos - 1);
                    break;
                }
            }
        }
    }

    return search;
}

static int scrap_thread(void *ptr) {

    auto scrapper = (Scrapper *) ptr;
    auto main = scrapper->main;

    while (scrapper->running) {

        scrapper->mutex->lock();
        scrapper->cond->wait(scrapper->mutex);

#ifdef __SWITCH__
        appletSetMediaPlaybackState(true);
#endif
        if (scrapper->running) {

            scrapper->scrapping = true;
            main->getStatus()->show("Scrapping...", "Building media list...", true);

            c2d::C2DClock clock;

            scrapList.clear();
            find_medias(main, scrapper->path);

            size_t size = scrapList.size();
            for (size_t i = 0; i < size; i++) {

                if (!main->getScrapper()->running) {
                    break;
                }

                c2d::Io::File file = scrapList.at(i);
                std::string scrap_path = pplay::Utility::getMediaScrapPath(file);
                if (!main->getIo()->exist(scrap_path)) {
                    std::string title = "Scrapping... (" + std::to_string(i) + "/" + std::to_string(size) + ")";
                    main->getStatus()->show(title, "Searching: " + file.name, true);
                    std::string lang = main->getConfig()->getOption(OPT_TMDB_LANGUAGE)->getString();
                    Search search(API_KEY, clean_name(file.name), lang);
                    int res = search.get();
                    if (res == 0) {
                        search.save(scrap_path);
                        if (search.total_results > 0) {
                            main->getStatus()->show(title, "Downloading poster: "
                                                           + search.movies.at(0).title, true);
                            search.movies.at(0).getPoster(pplay::Utility::getMediaPosterPath(file));
                            main->getStatus()->show(title, "Downloading backdrop: "
                                                           + search.movies.at(0).title, true);
                            search.movies.at(0).getBackdrop(pplay::Utility::getMediaBackdropPath(file), 780);
                            // refresh ui now
                            main->getFiler()->setScrapInfo(file, search.movies);
                        }
                    }
                }
            }

            scrapper->main->getStatus()->show(
                    "Scrapping...", "Done in "
                                    + pplay::Utility::formatTime(clock.getElapsedTime().asSeconds()));
            scrapper->scrapping = false;
        }

#ifdef __SWITCH__
        appletSetMediaPlaybackState(false);
#endif
        scrapper->mutex->unlock();
    }

    return 0;
}

int Scrapper::scrap(const std::string &p) {

    if (scrapping) {
        return -1;
    }

    path = p;
    cond->signal();

    return 0;
}

Scrapper::Scrapper(Main *m) {

    main = m;
    mutex = new c2d::C2DMutex();
    cond = new c2d::C2DCond();
    thread = new c2d::C2DThread(scrap_thread, (void *) this);
}

Scrapper::~Scrapper() {

    scrapping = false;
    running = false;
    cond->signal();
    thread->join();

    delete (mutex);
    delete (cond);
    delete (thread);

    printf("Scrapper::~Scrapper\n");
}
