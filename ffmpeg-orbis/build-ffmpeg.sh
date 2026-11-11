#!/usr/bin/env bash
# Rebuild PacBrew's FFmpeg 5.0 with the h264_orbis (libSceVideodec2) decoder and
# install it over the toolchain, so libmpv/pplay pick it up when they link.
# Same configure line as pacbrew-packages/ps4-openorbis-portlibs/ffmpeg/PKGBUILD.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$(mktemp -d)}
SHA256=51e919f7d205062c0fd4fae6243a84850391115104ccf1efc451733bc0ac7298

# shellcheck disable=SC1091
source /opt/pacbrew/ps4/openorbis/ps4vars.sh

cd "$WORK"
curl -fsSL --retry 3 -o ffmpeg-5.0.tar.xz https://ffmpeg.org/releases/ffmpeg-5.0.tar.xz
echo "$SHA256  ffmpeg-5.0.tar.xz" | sha256sum -c -
tar xf ffmpeg-5.0.tar.xz
cd ffmpeg-5.0
patch -p1 < "$HERE/0001-h264_orbis-decoder.patch"

./configure --prefix="${OPENORBIS}/usr" \
  --enable-cross-compile --cross-prefix=${OPENORBIS}/bin/ \
  --arch=x86_64 --target-os=freebsd \
  --disable-shared --enable-static \
  --pkg-config=openorbis-pkg-config \
  --disable-programs --disable-debug --disable-doc \
  --disable-avdevice --disable-hwaccels --disable-encoders --disable-muxers \
  --enable-swscale --enable-swresample --enable-network \
  --disable-protocols --enable-protocol=file,http,ftp,tcp,udp,rtmp \
  --enable-zlib --enable-bzlib --enable-libass --enable-libfreetype --enable-libfribidi \
  --cc=$CC --cxx=$CXX --nm=$NM --strip=$STRIP --ar=$AR --ranlib=$RANLIB

# same workaround as the PacBrew package
sed -i 's|#define HAVE_SYSCTL 1|//#define HAVE_SYSCTL 1|g' config.h

grep -q 'CONFIG_H264_ORBIS_DECODER 1' config.h \
  || { echo "h264_orbis decoder not enabled by configure"; exit 1; }

make --quiet -j"$(getconf _NPROCESSORS_ONLN)"
sudo -E env "PATH=$PATH" make install
rm -rf "${OPENORBIS}/usr/share/ffmpeg" 2>/dev/null || sudo rm -rf "${OPENORBIS}/usr/share/ffmpeg"

# (no "nm | grep -q": grep exiting early would SIGPIPE nm and trip pipefail)
nm "${OPENORBIS}/usr/lib/libavcodec.a" > nm.txt 2>/dev/null || true
grep -c ff_h264_orbis_decoder nm.txt | grep -qv '^0$' \
  || { echo "installed libavcodec.a lacks h264_orbis"; exit 1; }
echo "FFmpeg with h264_orbis installed in ${OPENORBIS}/usr"
