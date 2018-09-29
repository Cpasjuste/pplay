//
// Created by cpasjuste on 29/09/18.
//

#include "c2d.h"
#include "c2dui.h"
#include "main.h"
#include "filer.h"

#define C2DUI_HOME_PATH "./"

using namespace c2d;
using namespace c2dui;

Renderer *renderer;
Input *inp;
Io *io;

UIMain *ui;
UIRomList *uiRomList;
UIMenu *uiMenu;
UIEmu *uiEmu;
UIStateMenu *uiState;

Config *config;
Filer *romList;
Skin *skin;

int main(int argc, char **argv) {

    renderer = new C2DRenderer(Vector2f(SCR_W, SCR_H));
    inp = new C2DInput();
    io = new C2DIo();

    // load configuration
    config = new Config(C2DUI_HOME_PATH, 100);

    // skin
    // buttons used for ui config menu
    std::vector<Skin::Button> buttons;
#ifdef __PSP2__
    // set max cpu speed
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
    // see c2d.h for key id
    buttons.emplace_back(KEY_JOY_UP_DEFAULT, "UP");
    buttons.emplace_back(KEY_JOY_DOWN_DEFAULT, "DOWN");
    buttons.emplace_back(KEY_JOY_LEFT_DEFAULT, "LEFT");
    buttons.emplace_back(KEY_JOY_RIGHT_DEFAULT, "RIGHT");
    buttons.emplace_back(KEY_JOY_FIRE1_DEFAULT, "TRIANGLE");
    buttons.emplace_back(KEY_JOY_FIRE2_DEFAULT, "CIRCLE");
    buttons.emplace_back(KEY_JOY_FIRE3_DEFAULT, "CROSS");
    buttons.emplace_back(KEY_JOY_FIRE4_DEFAULT, "SQUARE");
    buttons.emplace_back(KEY_JOY_FIRE5_DEFAULT, "L");
    buttons.emplace_back(KEY_JOY_FIRE6_DEFAULT, "R");
    buttons.emplace_back(KEY_JOY_COIN1_DEFAULT, "SELECT");
    buttons.emplace_back(KEY_JOY_START1_DEFAULT, "START");
#elif __SWITCH__
    // see c2d.h for key id
    buttons.emplace_back(KEY_JOY_UP_DEFAULT, "UP");
    buttons.emplace_back(KEY_JOY_DOWN_DEFAULT, "DOWN");
    buttons.emplace_back(KEY_JOY_LEFT_DEFAULT, "LEFT");
    buttons.emplace_back(KEY_JOY_RIGHT_DEFAULT, "RIGHT");
    buttons.emplace_back(KEY_JOY_FIRE1_DEFAULT, "A");
    buttons.emplace_back(KEY_JOY_FIRE2_DEFAULT, "B");
    buttons.emplace_back(KEY_JOY_FIRE3_DEFAULT, "X");
    buttons.emplace_back(KEY_JOY_FIRE4_DEFAULT, "Y");
    buttons.emplace_back(KEY_JOY_FIRE5_DEFAULT, "L");
    buttons.emplace_back(KEY_JOY_FIRE6_DEFAULT, "R");
    buttons.emplace_back(KEY_JOY_COIN1_DEFAULT, "-");
    buttons.emplace_back(KEY_JOY_START1_DEFAULT, "+");
    // switch special keys
    buttons.emplace_back(KEY_JOY_ZL_DEFAULT, "ZL");
    buttons.emplace_back(KEY_JOY_ZR_DEFAULT, "ZR");
    buttons.emplace_back(KEY_JOY_LSTICK_DEFAULT, "LSTICK");
    buttons.emplace_back(KEY_JOY_RSTICK_DEFAULT, "RSTICK");
#endif

#ifdef __PSP2__
    skin = new Skin("app0:/", buttons);
#else
    skin = new Skin(C2DUI_HOME_PATH, buttons);
#endif

    // gui
    ui = new UIMain(renderer, io, inp, config, skin);
    std::string nestopia_version = "Nestopia 1.0";
    romList = new Filer(ui, nestopia_version);
    romList->build();
    uiRomList = new UIRomList(ui, romList, renderer->getSize());
    uiMenu = new UIMenu(ui);
    uiEmu = new UIEmu(ui);
    uiState = new UIStateMenu(ui);
    ui->init(uiRomList, uiMenu, uiEmu, uiState);
    ui->run();

    // quit
    delete (ui);
    delete (skin);
    delete (config);
    delete (io);
    delete (inp);
    delete (renderer);

#ifdef __PSP2__
    scePowerSetArmClockFrequency(266);
    scePowerSetBusClockFrequency(166);
    scePowerSetGpuClockFrequency(166);
    scePowerSetGpuXbarClockFrequency(111);
#endif

    return 0;
}
