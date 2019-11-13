//
// Created by cpasjuste on 12/04/18.
//

#ifndef FILER_FTP_H
#define FILER_FTP_H

#include "cross2d/c2d.h"
#include "filer.h"
#include "ftplib/ftplib.h"

class FilerFtp : public Filer {

public:

    FilerFtp(Main *main, const c2d::FloatRect &rect);

    ~FilerFtp() override;

    bool getDir(const std::string &path) override;

    std::string getError() override;

private:

    void enter(int prev_index) override;

    void exit() override;

    std::string error;
    netbuf *ftp_con = nullptr;
};

#endif //FILER_FTP_H
