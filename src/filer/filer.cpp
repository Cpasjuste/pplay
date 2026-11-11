//
// Created by cpasjuste on 12/04/18.
//

#include "main.h"
#include "filer.h"
#include "utility.h"
#include "p_search.h"
#include <atomic>

#define ITEM_HEIGHT 50
// max time (seconds) to wait for a remote (http/ftp/smb) directory listing
#define REMOTE_LIST_TIMEOUT 25

using namespace c2d;

// remote listings can block for a long time (dns/connect without timeout on ps4),
// so they are done in a thread while the ui keeps drawing (and can cancel)
struct RemoteListCtx {
    Main *main = nullptr;
    pplay::Io::DeviceType type = pplay::Io::DeviceType::Local;
    std::vector<std::string> ext;
    std::string path;
    std::vector<Io::File> files;
    std::atomic<int> state{0}; // 0: running, 1: done, 2: abandoned by the ui
};

static int remote_list_thread(void *data) {
    auto *ctx = (RemoteListCtx *) data;
    ctx->files = ((pplay::Io *) ctx->main->getIo())->getDirList(ctx->type, ctx->ext, ctx->path, false);
    if (ctx->state.exchange(1) == 2) {
        // ui gave up waiting, nobody will read the result
        delete ctx;
    }
    return 0;
}

static bool list_remote(Main *main, pplay::Io::DeviceType type, const std::vector<std::string> &ext,
                        const std::string &path, std::vector<Io::File> &out, std::string &error) {
    auto *ctx = new RemoteListCtx();
    ctx->main = main;
    ctx->type = type;
    ctx->ext = ext;
    ctx->path = path;

    main->getStatus()->show("Please Wait...", "Loading " + path + "\n\nPress O to cancel", true);
    auto *thread = new C2DThread(remote_list_thread, ctx);
    C2DClock clock;
    bool aborted = false;

    while (ctx->state.load() != 1) {
        if (main->getInput()->update()->keys & Input::Key::Fire2) {
            error = "Loading cancelled.";
            aborted = true;
        } else if (clock.getElapsedTime().asSeconds() > REMOTE_LIST_TIMEOUT) {
            error = "Timeout while loading \"" + path + "\"";
            aborted = true;
        }
        if (aborted) {
            break;
        }
        main->flip(true, false);
    }
    main->getStatus()->hide();

    if (aborted) {
        if (ctx->state.exchange(2) == 1) {
            // finished just now, clean up here
            thread->join();
            delete thread;
            delete ctx;
        }
        // else: thread still blocked in network code, leaked on purpose (cleans up if it ever returns)
        return false;
    }

    thread->join();
    delete thread;
    out = ctx->files;
    delete ctx;
    return true;
}

Filer::Filer(Main *m, const std::string &path, const c2d::FloatRect &rect) :
        Rectangle(m->getScaled(rect)) {

    main = m;

    mutex = new C2DMutex();

    // force scrap view width to scrapped backdrop width
    scrapView = new ScrapView(main, {rect.width - (780 * m->getScaling().x), 0,
                                     780 * m->getScaling().x, rect.height});
    Filer::add(scrapView);

    /// scaling this too much is not pretty, so no scaling for filer
    Vector2f size = {rect.width - (670 * m->getScaling().x), rect.height - (64 * m->getScaling().y)};

    // highlight
    highlight = new Highlight({size.x, (float) ITEM_HEIGHT * m->getScaling().y}, Highlight::CursorPosition::Left);
    Filer::add(highlight);

    // files items
    item_height = highlight->getSize().y;
    item_max = (int) (size.y / item_height);
    if (((float) item_max * item_height) < size.y) {
        item_height = size.y / (float) item_max;
    }

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        FloatRect r = {0, item_height * (float) i, size.x, item_height};
        items.emplace_back(new FilerItem(main, r));
        Filer::add(items[i]);
    }

    // tween
    Filer::add(new TweenAlpha(0, 255, 0.5f));
}

void Filer::setMediaInfo(const MediaFile &target, const MediaInfo &mediaInfo) {
    mutex->lock();
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].path == target.path) {
            files[i].mediaInfo = mediaInfo;
            dirty = true;
            break;
        }
    }
    mutex->unlock();
}

void Filer::setScrapInfo(const Io::File &target, const std::vector<pscrap::Movie> &movies) {
    mutex->lock();
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].path == target.path) {
            files[i].movies = movies;
            dirty = true;
            break;
        }
    }
    mutex->unlock();
}

void Filer::setSelection(int index) {
    item_index = index;
    int page = item_index / item_max;
    unsigned int index_start = (unsigned int) page * item_max;

    mutex->lock();

    for (unsigned int i = 0; i < (unsigned int) item_max; i++) {
        if (index_start + i >= files.size()) {
            items[i]->setVisibility(Visibility::Hidden);
        } else {
            // load media info, set file
            MediaFile file = files[index_start + i];
            items[i]->setFile(file);
            items[i]->setVisibility(Visibility::Visible);
            if (!file.movies.empty()) {
                items[i]->setTitle(file.movies[0].title);
            }
            // set highlight position
            if (index_start + i == (unsigned int) item_index) {
                highlight->tweenTo(items[i]->getPosition());
                if (file.type == Io::Type::File) {
                    if (!scrapView->isVisible()) {
                        scrapView->setVisibility(Visibility::Visible);
                    }
                    scrapView->setMovie(file);
                } else {
                    scrapView->setVisibility(Visibility::Hidden);
                }
            }
        }
    }

    if (files.empty()) {
        highlight->setVisibility(Visibility::Hidden);
    } else {
        highlight->setVisibility(Visibility::Visible);
    }

    mutex->unlock();
}

MediaFile Filer::getSelection() const {
    mutex->lock();
    if (!files.empty() && files.size() > (unsigned int) item_index) {
        return files[item_index];
    }
    mutex->unlock();

    return {};
}

bool Filer::onInput(c2d::Input::Player *players) {
    if (main->getMenuMain()->isMenuVisible()
        || main->getPlayer()->isFullscreen()) {
        return false;
    }

    mutex->lock();
    size_t filesSize = files.size();
    mutex->unlock();

    unsigned int keys = players[0].keys;

    if (keys & c2d::Input::Start || keys & c2d::Input::Select) {
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Up) {
        item_index--;
        if (item_index < 0)
            item_index = (int) (filesSize - 1);
        setSelection(item_index);
        scrapView->unload();
    } else if (keys & Input::Key::Down) {
        item_index++;
        if (item_index >= (int) filesSize) {
            item_index = 0;
        }
        setSelection(item_index);
        scrapView->unload();
    } else if (keys & Input::Key::Left) {
        main->getMenuMain()->setVisibility(Visibility::Visible, true);
    } else if (keys & Input::Key::Right) {
        if (!main->getPlayer()->getMpv()->isStopped()
            && !main->getPlayer()->isFullscreen()) {
            main->getPlayer()->setFullscreen(true);
        }
    } else if (keys & Input::Key::Fire1) {
        if (getSelection().type == Io::Type::Directory) {
            scrapView->unload();
            enter(item_index);
        } else if (pplay::Utility::isMedia(getSelection())) {
            main->getPlayer()->load(files[item_index]);
        }
    } else if (keys & Input::Key::Fire2) {
        scrapView->unload();
        exit();
    } else if (keys & Input::Key::Fire3) {
        main->getScrapper()->scrap(path);
    }

    return true;
}

void Filer::onUpdate() {
    if (dirty) {
        setSelection(item_index);
        dirty = false;
    }

    C2DObject::onUpdate();
}

static bool compare(const MediaFile &a, const MediaFile &b) {
    if (a.type == Io::Type::Directory && b.type != Io::Type::Directory) {
        return true;
    }
    if (a.type != Io::Type::Directory && b.type == Io::Type::Directory) {
        return false;
    }

    std::string aa = a.movies.empty() ? a.name : a.movies[0].title;
    std::string bb = b.movies.empty() ? b.name : b.movies[0].title;

    return Utility::toLower(aa) < Utility::toLower(bb);
}

bool Filer::getDir(const std::string &p) {
    printf("getDir(%s)\n", p.c_str());
    error.clear();

    std::string newPath = p;
    if (newPath.size() > 1 && Utility::endsWith(newPath, "/")) {
        newPath = Utility::removeLastSlash(newPath);
    }

    // do the (possibly slow) listing before touching the current state/mutex
    std::vector<std::string> ext = pplay::Utility::getMediaExtensions();
    pplay::Io::DeviceType type = ((pplay::Io *) main->getIo())->getDeviceType(p);
    std::vector<Io::File> _files;
    if (type == pplay::Io::DeviceType::Local) {
        _files = ((pplay::Io *) main->getIo())->getDirList(type, ext, newPath, false);
    } else if (!list_remote(main, type, ext, newPath, _files, error)) {
        return false;
    }

    // remote listing failed (no network, bad url, server down...): keep the
    // current directory and report the error instead of showing an empty list
    if (type != pplay::Io::DeviceType::Local && _files.empty()) {
        error = "Could not list \"" + p + "\"\n\n"
                + "Check the NETWORK entry in pplay.cfg and the console network connection.";
        printf("getDir: %s\n", error.c_str());
        return false;
    }

    mutex->lock();
    files.clear();
    path = newPath;

    for (auto &file: _files) {
        MediaFile mf(file, MediaInfo(file));
        if (file.type == Io::Type::File) {
            pscrap::Search search;
            std::string scrapPath = pplay::Utility::getMediaScrapPath(file);
            if (main->getIo()->exist(scrapPath)) {
                search.load(scrapPath);
                if (search.total_results > 0) {
                    mf.movies = search.movies;
                }
            }
        }
        files.emplace_back(mf);
    }

    // sort after title have been scrapped
    std::sort(files.begin(), files.end(), compare);

    if (files.empty() || files.at(0).name != "..") {
        Io::File file("..", "..", Io::Type::Directory, 0);
        files.insert(files.begin(), MediaFile{file, MediaInfo(file)});
    }

    mutex->unlock();
    setSelection(0);

    return true;
}

void Filer::enter(int index) {
    MediaFile file = getSelection();
    bool success;

    if (file.name == "..") {
        exit();
        return;
    }

    if (path == "/") {
        success = getDir(path + file.name);
    } else {
        success = getDir(path + "/" + file.name);
    }
    if (success) {
        item_index_prev.push_back(index);
        setSelection(item_index);
    } else if (!error.empty()) {
        main->getStatus()->show("OOPS", error);
    }
}

void Filer::exit() {
    std::string p = path;

    if (p == "/" || p.find('/') == std::string::npos) {
        return;
    }

    pplay::Io::DeviceType type = ((pplay::Io *) main->getIo())->getDeviceType(p);
    if (type != pplay::Io::DeviceType::Local) {
        std::string s = p;
        if (!Utility::endsWith(s, "/")) {
            s += "/";
        }
        if (s == main->getConfig()->getOption(OPT_NETWORK)->getString()) {
            return;
        }
    }

    while (p.back() != '/') {
        p.erase(p.size() - 1);
    }

    if (p.size() > 1 && Utility::endsWith(p, "/")) {
        p.erase(p.size() - 1);
    }

    if (getDir(p)) {
        if (!item_index_prev.empty()) {
            int last = (int) item_index_prev.size() - 1;
            mutex->lock();
            size_t filesSize = files.size();
            mutex->unlock();
            if (item_index_prev[last] < (int) filesSize) {
                item_index = item_index_prev[last];
            }
            item_index_prev.erase(item_index_prev.end() - 1);
        }
        setSelection(item_index);
    }
}

void Filer::clearHistory() {
    item_index_prev.clear();
}

std::string Filer::getPath() {
    return path;
}

Filer::~Filer() {
    delete (mutex);
}
