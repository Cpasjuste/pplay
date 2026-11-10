//
// pplay input: the platform input (SDL2 on the PS4) plus
//  * the buttons pressed from the phone remote control (press()),
//  * on the PS4, the left analog stick as direction keys, next to the D-pad (both work at the
//    same time): one direction at a time (no diagonals, they opened menus by accident),
//    a lower threshold than libcross2d's and a hysteresis so it does not flicker.
//    The right stick is not used for navigation.
//  * on the PS4, a controller turned on after pplay started is picked up (libcross2d only
//    looks for controllers at startup).
//

#ifndef PPLAY_INPUT_H
#define PPLAY_INPUT_H

#include <deque>
#include "cross2d/c2d.h"
#include "stick_direction.h"

class PplayInput : public c2d::C2DInput {

public:

    PplayInput();

    // press a button as if it was done on the controller: the key reaches the application
    // once, at the next update() (no auto-repeat, the phone sends one command per tap)
    void press(unsigned int key);

    c2d::Input::Player *update(int rotate = 0) override;

#ifdef __PS4__
private:

    void process_axis(c2d::Input::Player &player, int rotate) override;

    long long lastPadSearch = 0;
    pplay::StickDir stick = pplay::StickDir::None;
#else
private:
#endif

    std::deque<unsigned int> pending;
};

#endif //PPLAY_INPUT_H
