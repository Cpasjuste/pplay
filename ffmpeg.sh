source /opt/devkitpro/switchvars.sh
./configure --prefix="${PORTLIBS_PREFIX}" \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
    --disable-shared --enable-static \
    --target-os=linux --enable-pic \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-filters --enable-filter='rotate,transpose' \
    --disable-protocols --enable-protocol=file \
    --disable-encoders --disable-muxers --disable-network \
    --disable-programs --disable-debug --disable-doc
