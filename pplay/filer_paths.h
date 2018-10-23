//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_PATHS_H
#define FILER_PATHS_H

#include "cross2d/c2d.h"
#include "filer.h"

class FilerPaths : public Filer {

public:

    FilerPaths(PPLAYConfig *config, c2d::Font *font, int fontSize, const c2d::FloatRect &rect);

private:

    void enter() override;
};

#endif //FILER_PATHS_H
