//
// See pplay_input.h
//

#include "pplay_input.h"
#include "remote_server.h"

using namespace c2d;

PplayInput::PplayInput() : C2DInput() {
}

void PplayInput::press(unsigned int key) {
    if (pending.size() < 32) {
        pending.push_back(key);
    }
}

c2d::Input::Player *PplayInput::update(int rotate) {

#ifdef __PS4__
    // controller turned on after pplay started (or before the console gave it to us)
    if (players[0].data == nullptr) {
        const long long now = pplay::RemoteServer::nowMs();
        if (now - lastPadSearch > 2000 || now < lastPadSearch) {
            lastPadSearch = now;
            if (SDL_NumJoysticks() > 0) {
                players[0].data = SDL_JoystickOpen(0);
                players[0].id = 0;
            }
        }
    }
#endif

    Player *p = C2DInput::update(rotate);

    if (!pending.empty()) {
        unsigned int keys = p[0].keys;
        if (keys == Key::Delay) {
            keys = 0; // a held key waiting for its repeat: the remote key goes first
        }
        p[0].keys = keys | pending.front();
        pending.pop_front();
    }

    return p;
}

#ifdef __PS4__

// replaces libcross2d's: left stick only, one direction at a time
void PplayInput::process_axis(c2d::Input::Player &player, int rotate) {
    if (!player.enabled || player.data == nullptr || player.id != 0) {
        return;
    }

    const int x = SDL_JoystickGetAxis((SDL_Joystick *) player.data, player.lx.id);
    const int y = SDL_JoystickGetAxis((SDL_Joystick *) player.data, player.ly.id);
    stick = pplay::stickDirection(x, y, stick);
    switch (stick) {
        case pplay::StickDir::Up:
            player.keys |= Input::Key::Up;
            break;
        case pplay::StickDir::Down:
            player.keys |= Input::Key::Down;
            break;
        case pplay::StickDir::Left:
            player.keys |= Input::Key::Left;
            break;
        case pplay::StickDir::Right:
            player.keys |= Input::Key::Right;
            break;
        default:
            break;
    }
}

#endif
