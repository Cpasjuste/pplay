source /opt/devkitpro/switchvars.sh
./configure --prefix="${PORTLIBS_PREFIX}" \
    --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
    --disable-shared --enable-static \
    --target-os=linux \
    --disable-encoders --disable-muxers --disable-network
