source /opt/devkitpro/switchvars.sh

./configure --prefix="${PORTLIBS_PREFIX}" --cross-prefix=aarch64-none-elf- --target-os=linux --arch=aarch64 --disable-shared --enable-static --disable-encoders --disable-muxers --disable-network --disable-swresample --disable-swscale

#./configure --prefix="${PORTLIBS_PREFIX}" --enable-cross-compile --cross-prefix=aarch64-none-elf- --target-os=linux --arch=aarch64 --disable-shared --enable-static --enable-small --enable-gpl --disable-encoders --disable-muxers --disable-network --disable-asm --disable-neon --disable-runtime-cpudetect --extra-cflags="-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE -ffunction-sections" --disable-optimizations --optflags="-O0"

#--disable-optimizations 
#--optflags=

