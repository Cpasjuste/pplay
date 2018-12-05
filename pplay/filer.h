//
// Created by cpasjuste on 12/04/18.
//

#ifndef NXFILER_FILER_H
#define NXFILER_FILER_H

#include "cross2d/c2d.h"

#include "outline_rect.h"
#include "config.h"
#include "filer_item.h"
#include "media_file.h"
#include "highlight.h"

class Main;

class Filer : public c2d::C2DRectangle {

public:

    Filer(Main *main, const std::string &path, const c2d::FloatRect &rect);

    ~Filer();

    virtual bool getDir(const std::string &path) { return false; };

    virtual std::string getPath();

    virtual const MediaFile getSelection() const;

    virtual void setSelection(int index);

    void onInput(c2d::Input::Player *players) override;

private:

    friend class FilerSdmc;

    friend class FilerHttp;

    virtual void enter() {};

    virtual void exit() {};

    Main *main;
    std::string path;
    std::vector<FilerItem *> items;
    std::vector<MediaFile> files;
    Highlight *highlight;
    float item_height;
    int item_max;
    int item_index;
};

#endif //NXFILER_FILER_H
