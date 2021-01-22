#!/usr/bin/env bash
source /opt/devkitpro/switchvars.sh
./configure --prefix="${PORTLIBS_PREFIX}" \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=${PORTLIBS_PREFIX}/bin/aarch64-none-elf-pkg-config \
    --extra-cflags='-D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
    --extra-cxxflags='-D_GNU_SOURCE -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
    --extra-ldflags='-fPIE -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib' \
    --disable-shared --enable-static --target-os=linux --enable-pic \
    --enable-asm --enable-neon \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol='file,http,ftp' \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-encoders --disable-muxers \
    --disable-programs --disable-debug --disable-doc --disable-runtime-cpudetect --disable-autodetect

#TODO: fix configure
sed 's/#define HAVE_INET_ATON 0/#define HAVE_INET_ATON 1/g' -i config.h
sed 's/#define HAVE_CLOSESOCKET 1/#define HAVE_CLOSESOCKET 0/g' -i config.h
sed 's/#define HAVE_MMAP 1/#define HAVE_MMAP 0/g' -i config.h
