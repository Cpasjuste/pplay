//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_SDMC_H
#define FILER_SDMC_H

#include "cross2d/c2d.h"
#include "filer.h"

class FilerSdmc : public Filer {

public:

    FilerSdmc(Main *main, const std::string &path, const c2d::FloatRect &rect);

    bool getDir(const std::string &path) override;

private:

    void enter() override;

    void exit() override;
};

#endif //FILER_SDMC_H
