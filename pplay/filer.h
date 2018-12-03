//
// Created by cpasjuste on 12/04/18.
//

#ifndef NXFILER_FILER_H
#define NXFILER_FILER_H

#include "cross2d/c2d.h"
#include "outline_rect.h"
#include "config.h"

class Main;

class Filer : public c2d::C2DRectangle {

public:

    Filer(Main *main, const std::string &path, const c2d::FloatRect &rect);

    ~Filer();

    virtual bool getDir(const std::string &path) { return false; };

    virtual std::string getPath();

    virtual bool step(unsigned int keys);

    virtual c2d::Io::File *getSelection();

    virtual c2d::ListBox *getListBox();

private:

    friend class FilerSdmc;

    friend class FilerHttp;

    //friend class FilerPaths;

    void down();

    void up();

    void left();

    void right();

    virtual void enter() {};

    virtual void exit() {};

    Main *main;
    std::string path;
    std::vector<c2d::Io::File> files;
    c2d::ListBox *listBox;
    OutlineRect *pathRect;
    c2d::Text *pathText;
    int index = 0;
};

#endif //NXFILER_FILER_H
