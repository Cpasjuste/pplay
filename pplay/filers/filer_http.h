//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_HTTP_H
#define FILER_HTTP_H

#include "cross2d/c2d.h"
#include "filer.h"

class Browser;

class FilerHttp : public Filer {

public:

    FilerHttp(Main *main, const c2d::FloatRect &rect);

    ~FilerHttp() override;

    bool getDir(const std::string &path) override;

    const std::string getError() override;

private:

    void enter(int prev_index) override;

    void exit() override;

    Browser *browser;
};

#endif //FILER_HTTP_H
