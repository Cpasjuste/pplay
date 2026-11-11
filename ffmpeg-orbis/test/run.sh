#!/usr/bin/env bash
# Host test of the h264_orbis decoder against a mock of libSceVideodec2.
# Builds FFmpeg 5.0 + the patch for the PC, generates test videos with libx264
# (ffmpeg and libx264 must be installed) and checks that the decoder returns
# bit-exact pictures with the right timestamps. No console needed.
set -euo pipefail
T=$(cd "$(dirname "$0")" && pwd)
PATCH=$T/../0001-h264_orbis-decoder.patch
WORK=${WORK:-$(mktemp -d)}
SRC=${FFMPEG_SRC:-}

if [ -z "$SRC" ]; then
  cd "$WORK"
  [ -f ffmpeg-5.0.tar.xz ] || curl -fsSL --retry 3 -o ffmpeg-5.0.tar.xz https://ffmpeg.org/releases/ffmpeg-5.0.tar.xz
  echo "51e919f7d205062c0fd4fae6243a84850391115104ccf1efc451733bc0ac7298  ffmpeg-5.0.tar.xz" | sha256sum -c -
  [ -d ffmpeg-5.0 ] || { tar xf ffmpeg-5.0.tar.xz; patch -d ffmpeg-5.0 -p1 < "$PATCH"; }
  SRC=$WORK/ffmpeg-5.0
fi

B=$WORK/build; mkdir -p "$B"; cd "$B"
if [ ! -f libavcodec/libavcodec.a ]; then
  # inline asm off: ffmpeg 5.0 does not assemble with recent binutils
  "$SRC/configure" --disable-everything --disable-programs --disable-doc --disable-debug \
    --disable-avdevice --disable-swscale --disable-swresample --disable-avfilter \
    --enable-avformat --enable-decoder=h264,h264_orbis --enable-bsf=h264_mp4toannexb \
    --enable-parser=h264 --enable-demuxer=matroska,mov,mpegts --enable-protocol=file \
    --enable-static --disable-shared --disable-x86asm --disable-inline-asm --extra-cflags="-O1 -g" > conf.log 2>&1
  make -j"$(nproc)" > make.log 2>&1
fi
gcc -g -O1 -I"$SRC" -I"$B" -I"$SRC/libavcodec" "$T/harness.c" "$T/mock_sys.c" -o "$B/orbis_test" \
  "$B/libavformat/libavformat.a" "$B/libavcodec/libavcodec.a" "$B/libavutil/libavutil.a" \
  -lz -lbz2 -lpthread -lm

V=$WORK/vid; mkdir -p "$V"; cd "$V"
gen() { local n=$1 s=$2; shift 2
  [ -f "$n" ] || ffmpeg -v error -y -f lavfi -i "testsrc2=size=$s:rate=24" -frames:v 96 -c:v libx264 "$@" "$n"; }
gen hp1080.mp4  1920x1080 -profile:v high -bf 3 -refs 4 -g 48 -pix_fmt yuv420p
gen mp720.mkv   1280x720  -profile:v main -bf 2 -refs 3 -g 30 -pix_fmt yuv420p
gen hp1080.ts   1920x1080 -profile:v high -bf 3 -refs 4 -g 48 -pix_fmt yuv420p -x264-params aud=1
gen odd1366.mp4 1366x768  -profile:v high -bf 2 -refs 3 -g 40 -pix_fmt yuv420p
gen base480.mp4 640x480   -profile:v baseline -pix_fmt yuv420p
gen hi10.mkv    1280x720  -profile:v high10 -pix_fmt yuv420p10le
gen y422.mkv    1280x720  -profile:v high422 -pix_fmt yuv422p
gen mp720.ts    1280x720  -profile:v main -bf 2 -refs 3 -g 30 -pix_fmt yuv420p
[ -f concat.ts ] || cat hp1080.ts mp720.ts > concat.ts
gen big4k.mp4   3840x2160 -profile:v high -pix_fmt yuv420p -frames:v 4
gen ilace.mkv   1920x1080 -profile:v high -flags +ildct+ilme -x264-params tff=1 -pix_fmt yuv420p

fail=0
t() { echo "== $*"; "$B/orbis_test" "$@" || fail=1; }
t hp1080.mp4; t hp1080.mp4 seek; t hp1080.mp4 pad; t hp1080.mp4 reopen
t hp1080.mp4 noreset; t hp1080.mp4 noflush; t hp1080.mp4 failau; t hp1080.mp4 errframe
t concat.ts midchange; t mp720.mkv; t hp1080.ts; t odd1366.mp4; t base480.mp4
for f in hi10.mkv y422.mkv big4k.mp4 ilace.mkv; do t "$f" reject; done
[ $fail -eq 0 ] && echo "ALL TESTS PASSED" || { echo "TESTS FAILED"; exit 1; }
