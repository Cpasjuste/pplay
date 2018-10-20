//
// Created by cpasjuste on 12/04/18.
//

#ifndef NXFILER_FILER_H
#define NXFILER_FILER_H

#include "cross2d/c2d.h"
#include "outline_rect.h"

class Filer : public c2d::C2DRectangle {

public:

    Filer(const std::string &path, c2d::Font *font,
          int fontSize, const c2d::FloatRect &rect);

    ~Filer();

    virtual bool getDir(const std::string &path) { return false; };

    virtual std::string getPath();

    virtual bool step(unsigned int keys);

    virtual c2d::Io::File getSelection();

private:

    friend class FilerSdmc;

    friend class FilerHttp;

    void down();

    void up();

    void left();

    void right();

    virtual void enter() {};

    virtual void exit() {};

    std::string path;
    std::vector<c2d::Io::File> files;
    c2d::ListBox *listBox;
    OutlineRect *pathRect;
    c2d::Text *pathText;
    int index = 0;
};

#endif //NXFILER_FILER_H
