#!/usr/bin/env bash
source /opt/devkitpro/switchvars.sh
./configure --prefix="${PORTLIBS_PREFIX}" \
    --extra-cflags='-DTCP_MAXSEG=2' \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static --target-os=linux --enable-pic \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol='file,http,ftp' \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-encoders --disable-muxers \
    --disable-programs --disable-debug --disable-doc
    #TODO: --enable-encoder=png

    #TODO: fix configure
    sed 's/#define HAVE_INET_ATON 0/#define HAVE_INET_ATON 1/g' -i config.h

