source /opt/devkitpro/switchvars.sh

./configure --prefix="${PORTLIBS_PREFIX}" \
	--disable-shared --enable-static \
	--enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 \
	--pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
	--target-os=none --disable-encoders --disable-muxers \
	--disable-doc --disable-network \
	--disable-avdevice \
	--disable-demuxers --enable-demuxer=h264,matroska \
	--disable-decoders --enable-decoder=h264,aac \
	--disable-parsers --enable-parser=h264,aac \
	--disable-filters --enable-filter=rotate,transpose
	#--disable-avfilter 

./configure --prefix="${PORTLIBS_PREFIX}" --cross-prefix=aarch64-none-elf- --target-os=linux --arch=aarch64 --disable-shared --enable-static --disable-encoders --disable-muxers --disable-network --disable-swresample --disable-swscale


#./configure --prefix="${PORTLIBS_PREFIX}" --enable-cross-compile --cross-prefix=aarch64-none-elf- --target-os=linux --arch=aarch64 --disable-shared --enable-static --enable-small --enable-gpl --disable-encoders --disable-muxers --disable-network --disable-asm --disable-neon --disable-runtime-cpudetect --extra-cflags="-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE -ffunction-sections" --disable-optimizations --optflags="-O0"

#--disable-optimizations 
#--optflags=

./configure --prefix="${PORTLIBS_PREFIX}" \
      --enable-cross-compile --cross-prefix=aarch64-none-elf- \
      --pkg-config=/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-pkg-config \
      --disable-shared --disable-runtime-cpudetect --disable-armv5te --disable-programs \
      --disable-doc --disable-everything --enable-decoder='mpeg4,h264,aac,ac3,mp3,theora,ogg,flac,vorbis' \
      --enable-demuxer='mov,h264,ogg,flac' --enable-filter='rotate,transpose' --enable-protocol='file' \
      --enable-static --enable-small --arch=armv8a --cpu=cortex-a57 --disable-armv6t2 --target-os=none \
      --extra-cflags='-D__SWITCH__ -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
      --extra-cxxflags='-D__SWITCH__ -O2 -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec' \
      --extra-ldflags='-fPIE -L${PORTLIBS_PREFIX}/lib -L${DEVKITPRO}/libnx/lib' --disable-iconv --disable-lzma \
      --disable-securetransport --disable-xlib --disable-network


source /opt/devkitpro/devkita64.sh
source /opt/devkitpro/switchvars.sh

./configure --prefix=$PORTLIBS_PREFIX --disable-shared --enable-static \
    --cross-prefix=aarch64-none-elf- --enable-cross-compile \
    --arch=aarch64 --target-os=none --enable-pic --disable-asm \
    --disable-runtime-cpudetect --disable-programs --disable-debug --disable-doc \
    --disable-network --disable-hwaccels --disable-encoders --disable-muxers \
    --disable-avfilter --disable-avdevice --enable-swscale --enable-swresample \
    --enable-libass --enable-libfreetype --enable-libfribidi --enable-libtheora \
    --disable-protocols --enable-protocol=file \
    --disable-demuxers --enable-demuxer=h264,matroska \
    --disable-decoders --enable-decoder=h264,aac \
    --disable-parsers --enable-parser=h264,aac
