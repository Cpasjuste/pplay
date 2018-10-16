//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_HTTP_H
#define FILER_HTTP_H

#include "cross2d/c2d.h"

class Browser;

class HttpFiler : public c2d::C2DRectangle {

public:

    HttpFiler(c2d::Io *io, const std::string &path, const c2d::Font &font,
          int fontSize, const c2d::FloatRect &rect);

    ~HttpFiler();

    bool getDir(const std::string &path, bool reopen = true);

    std::string getPath();

    c2d::Io::File step(unsigned int keys);

    c2d::Io::File getSelection();

    void down();

    void up();

    void left();

    void right();

    void enter();

    void exit();

private:

    std::vector<c2d::Io::File> files;
    c2d::Io *io;
    c2d::ListBox *listBox;
    c2d::C2DRectangle *pathRect;
    c2d::Text *pathText;
    Browser *browser;
    int index = 0;
};

#endif //FILER_HTTP_H
