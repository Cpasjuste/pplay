//
// Created by cpasjuste on 12/04/18.
//

#ifndef NXFILER_FILER_H
#define NXFILER_FILER_H

#include "cross2d/c2d.h"
#include "outline_rect.h"
#include "config.h"
#include "filer_item.h"

class Main;

class Filer : public c2d::C2DRectangle {

public:

    Filer(Main *main, const std::string &path, const c2d::FloatRect &rect);

    ~Filer();

    virtual bool getDir(const std::string &path) { return false; };

    virtual std::string getPath();

    virtual bool step(unsigned int keys);

    virtual const FilerItem getSelection() const;

    virtual void setSelection(int index);

private:

    friend class FilerSdmc;

    friend class FilerHttp;

    void down();

    void up();

    virtual void enter() {};

    virtual void exit() {};

    Main *main;
    std::string path;
    std::vector<FilerItem *> items;
    std::vector<c2d::Io::File> files;
    float item_height;
    int item_max;
    int item_index;
};

#endif //NXFILER_FILER_H
