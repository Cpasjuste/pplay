//
// Created by cpasjuste on 29/09/18.
//

#ifndef PPLAY_PLAYER_H
#define PPLAY_PLAYER_H

class Player : public c2dui::UIEmu {

public:
    explicit Player(c2dui::UIMain *ui);

    int run(c2dui::RomList::Rom *rom);

    void stop();

    int update();

private:

};


#endif //PPLAY_PLAYER_H
