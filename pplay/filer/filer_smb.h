//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_SMB_H
#define FILER_SMB_H

#include "cross2d/c2d.h"
#include "filer.h"

class FilerSmb : public Filer {

public:

    FilerSmb(Main *main, const c2d::FloatRect &rect);

    ~FilerSmb() override;

    bool getDir(const std::string &path) override;

    std::string getError() override;

private:

    void enter(int prev_index) override;

    void exit() override;

    std::string error;
};

#endif //FILER_SMB_H
