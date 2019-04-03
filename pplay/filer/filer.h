//
// Created by cpasjuste on 12/04/18.
//

#ifndef NXFILER_FILER_H
#define NXFILER_FILER_H

#include "cross2d/c2d.h"

#include "outline_rect.h"
#include "pplay_config.h"
#include "filer_item.h"
#include "media_file.h"
#include "highlight.h"
#include "scrap_view.h"

class Main;

class Filer : public c2d::Rectangle {

public:

    Filer(Main *main, const std::string &path, const c2d::FloatRect &rect);

    void setMediaInfo(const MediaFile &target, const MediaInfo &mediaInfo);

    void setScrapInfo(const c2d::Io::File &target, const std::vector<pscrap::Movie> &movies);

    virtual bool getDir(const std::string &path);

    virtual std::string getPath();

    virtual const MediaFile getSelection() const;

    virtual void setSelection(int index);

    virtual void clearHistory();

    virtual const std::string getError() { return ""; };

    bool onInput(c2d::Input::Player *players) override;

    void onUpdate() override;

private:

    virtual void enter(int index);

    virtual void exit();

    Main *main;
    std::string path;
    std::vector<FilerItem *> items;
    std::vector<MediaFile> files;
    Highlight *highlight;
    ScrapView *scrapView;
    float item_height;
    int item_max;
    int item_index = 0;
    std::vector<int> item_index_prev;

    bool dirty = false;
};

#endif //NXFILER_FILER_H
