#!/usr/bin/env bash
source /opt/devkitpro/switchvars.sh
./configure --prefix="${PORTLIBS_PREFIX}" \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static --target-os=linux --enable-pic \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol='file,http' \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-encoders --disable-muxers \
    --disable-programs --disable-debug --disable-doc

    # some hackery, sorry WntrMute :)
    sed 's/#define HAVE_INET_ATON 0/#define HAVE_INET_ATON 1/g' -i config.h
    sed 's/#define HAVE_GETADDRINFO 0/#define HAVE_GETADDRINFO 1/g' -i config.h
    sed 's/#include <sys\/poll.h>/#include <poll.h>/g' -i libavformat/rtpproto.c
    sed '/#include <sys\/socket.h>/a typedef __uint32_t in_addr_t;' -i libavformat/network.h
    sed '/#include <sys\/socket.h>/a typedef __uint16_t in_port_t;' -i libavformat/network.h
