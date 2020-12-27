#!/bin/sh
cmake -DPLATFORM_SWITCH=ON . &&
make -j &&
elf2nro pplay pplay.nro --romfsdir=data_romfs/ --icon=data/switch/icon.jpg &&
nxlink -s pplay.nro
