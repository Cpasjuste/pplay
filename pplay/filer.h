//
// Created by cpasjuste on 29/09/18.
//

#ifndef PPLAY_FILER_H
#define PPLAY_FILER_H

class Filer : public c2dui::RomList {

public:
    Filer(c2dui::UIMain *ui, const std::string &emuVersion);

    ~Filer();

    void build() override;

    bool isHardware(int hardware, int type) override {
        return true;
    }
};

#endif //PPLAY_FILER_H
