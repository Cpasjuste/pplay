//
// Analog stick -> one direction (pure code, tested on a PC: tests/stick).
//

#ifndef PPLAY_STICK_DIRECTION_H
#define PPLAY_STICK_DIRECTION_H

namespace pplay {

    enum class StickDir {
        None, Up, Down, Left, Right
    };

    // Stick positions are in SDL range (-32768..32767, y positive = down), after the console
    // response curve (S shaped: 4500 is about 27% of the physical travel, 2800 about 22%).
    const int STICK_PRESS = 4500;
    const int STICK_RELEASE = 2800;

    // Direction pressed by the stick. `current` is the direction already reported: once pressed
    // it stays until the stick goes back under STICK_RELEASE (hysteresis, no flicker around the
    // threshold). The strongest axis wins, so a diagonal is one direction, not two.
    inline StickDir stickDirection(int x, int y, StickDir current) {
        const int ax = x < 0 ? -x : x;
        const int ay = y < 0 ? -y : y;
        const int major = ax > ay ? ax : ay;

        if (major < (current != StickDir::None ? STICK_RELEASE : STICK_PRESS)) {
            return StickDir::None;
        }
        if (ay >= ax) {
            return y < 0 ? StickDir::Up : StickDir::Down;
        }
        return x < 0 ? StickDir::Left : StickDir::Right;
    }
}

#endif //PPLAY_STICK_DIRECTION_H
