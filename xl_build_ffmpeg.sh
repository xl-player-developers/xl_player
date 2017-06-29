#!/bin/bash

set -e

# Set your own NDK here
#NDK=~/android-ndk-r10e
NDK=/Users/tianchi/Library/Android/sdk/ndk-bundle

ARM_PLATFORM=$NDK/platforms/android-16/arch-arm/
ARM_PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64

ARM64_PLATFORM=$NDK/platforms/android-21/arch-arm64/
ARM64_PREBUILT=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64

X86_PLATFORM=$NDK/platforms/android-16/arch-x86/
X86_PREBUILT=$NDK/toolchains/x86-4.9/prebuilt/darwin-x86_64

X86_64_PLATFORM=$NDK/platforms/android-21/arch-x86_64/
X86_64_PREBUILT=$NDK/toolchains/x86_64-4.9/prebuilt/darwin-x86_64

MIPS_PLATFORM=$NDK/platforms/android-9/arch-mips/
MIPS_PREBUILT=$NDK/toolchains/mipsel-linux-android-4.8/prebuilt/darwin-x86_64

BUILD_DIR=`pwd`/android-ffmpeg-lib

FFMPEG_VERSION="3.3.2"

if [ ! -e "ffmpeg-${FFMPEG_VERSION}.tar.bz2" ]; then
    echo "Downloading ffmpeg-${FFMPEG_VERSION}.tar.bz2"
    curl -LO http://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2
else
    echo "Using ffmpeg-${FFMPEG_VERSION}.tar.bz2"
    echo "Clean old files"
    rm -rf ffmpeg-${FFMPEG_VERSION}
fi

tar -xvf ffmpeg-${FFMPEG_VERSION}.tar.bz2

# reset so name  libavcodec.so.57 => libavcodec-57.so  only worked with version 3.3.2
pushd ffmpeg-$FFMPEG_VERSION
cp configure configure.bak
sed_version=`sed --version`
if [[ $sed_version =~ "GNU" ]]
then
    sed -i "s#^SLIBNAME_WITH_MAJOR=\'\$(SLIBNAME).\$(LIBMAJOR)\'#SLIBNAME_WITH_MAJOR=\'\$(SLIBPREF)\$(FULLNAME)-\$(LIBMAJOR)\$(SLIBSUF)\'#" configure
    sed -i "s#^SLIB_INSTALL_NAME=\'\$(SLIBNAME_WITH_VERSION)\'#SLIB_INSTALL_NAME=\'\$(SLIBNAME_WITH_MAJOR)\'#" configure
    sed -i "s#^SLIB_INSTALL_LINKS=\'\$(SLIBNAME_WITH_MAJOR) \$(SLIBNAME)\'#SLIB_INSTALL_LINKS=\'\$(SLIBNAME)\'#" configure

else
    sed -i "" "s#^SLIBNAME_WITH_MAJOR=\'\$(SLIBNAME).\$(LIBMAJOR)\'#SLIBNAME_WITH_MAJOR=\'\$(SLIBPREF)\$(FULLNAME)-\$(LIBMAJOR)\$(SLIBSUF)\'#" configure
    sed -i "" "s#^SLIB_INSTALL_NAME=\'\$(SLIBNAME_WITH_VERSION)\'#SLIB_INSTALL_NAME=\'\$(SLIBNAME_WITH_MAJOR)\'#" configure
    sed -i "" "s#^SLIB_INSTALL_LINKS=\'\$(SLIBNAME_WITH_MAJOR) \$(SLIBNAME)\'#SLIB_INSTALL_LINKS=\'\$(SLIBNAME)\'#" configure
fi
popd


function build_one
{
if [ $ARCH == "arm" ]
then
    PLATFORM=$ARM_PLATFORM
    PREBUILT=$ARM_PREBUILT
    HOST=arm-linux-androideabi
#added by alexvas
elif [ $ARCH == "arm64" ]
then
    PLATFORM=$ARM64_PLATFORM
    PREBUILT=$ARM64_PREBUILT
    HOST=aarch64-linux-android
elif [ $ARCH == "mips" ]
then
    PLATFORM=$MIPS_PLATFORM
    PREBUILT=$MIPS_PREBUILT
    HOST=mipsel-linux-android
#alexvas
elif [ $ARCH == "x86_64" ]
then
    PLATFORM=$X86_64_PLATFORM
    PREBUILT=$X86_64_PREBUILT
    HOST=x86_64-linux-android
else
    PLATFORM=$X86_PLATFORM
    PREBUILT=$X86_PREBUILT
    HOST=i686-linux-android
fi

#    --prefix=$PREFIX \

# TODO Adding aac decoder brings "libnative.so has text relocations. This is wasting memory and prevents security hardening. Please fix." message in Android.
pushd ffmpeg-$FFMPEG_VERSION
./configure --target-os=linux \
    --incdir=$BUILD_DIR/include \
    --libdir=$BUILD_DIR/lib/$CPU \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --arch=$ARCH \
    --cc=$PREBUILT/bin/$HOST-gcc \
    --cross-prefix=$PREBUILT/bin/$HOST- \
    --nm=$PREBUILT/bin/$HOST-nm \
    --sysroot=$PLATFORM \
    --extra-cflags="-fdata-sections -ffunction-sections -Os -fPIC -DANDROID -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS " \
    --enable-shared \
	--disable-static \
    --enable-small \
    --extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib -nostdlib -lc -lm -ldl -llog" \
	--enable-runtime-cpudetect \
	--disable-logging \
	--disable-gray \
	--disable-swscale-alpha \
	--disable-doc \
	--disable-postproc \
	--disable-avresample \
	--disable-encoders \
	--disable-hwaccels \
	--disable-muxers \
	--disable-iconv \
	--disable-audiotoolbox \
	--disable-videotoolbox \
    --disable-ffplay \
    --disable-ffmpeg \
    --disable-ffprobe \
    --disable-avfilter \
    --disable-avdevice \
    --disable-ffserver \
    $ADDITIONAL_CONFIGURE_FLAG

make clean
make -j8 install V=1
popd
}

#arm v5te
#CPU=armv5te
#ARCH=arm
#OPTIMIZE_CFLAGS="-marm -march=$CPU"
#PREFIX=$BUILD_DIR/$CPU
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#arm v6
#CPU=armv6
#ARCH=arm
#OPTIMIZE_CFLAGS="-marm -march=$CPU"
#PREFIX=`pwd`/ffmpeg-android/$CPU 
#ADDITIONAL_CONFIGURE_FLAG=
#build_one


#arm v7vfpv3
#CPU=armv7-a
#ARCH=arm
#OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfpv3-d16 -marm -march=$CPU "
#PREFIX=$BUILD_DIR/$CPU
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#arm v7vfp
#CPU=armv7-a
#ARCH=arm
#OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU "
#PREFIX=`pwd`/ffmpeg-android/$CPU-vfp
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#arm v7n
CPU=armv7-a
ARCH=arm
OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -marm -march=$CPU -mtune=cortex-a8"
PREFIX=`pwd`/ffmpeg-android/$CPU
ADDITIONAL_CONFIGURE_FLAG=--enable-neon
build_one

#arm v6+vfp
#CPU=armv6
#ARCH=arm
#OPTIMIZE_CFLAGS="-DCMP_HAVE_VFP -mfloat-abi=softfp -mfpu=vfp -marm -march=$CPU"
#PREFIX=`pwd`/ffmpeg-android/${CPU}_vfp
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#arm64-v8a
#CPU=arm64-v8a
#ARCH=arm64
#OPTIMIZE_CFLAGS=
#PREFIX=$BUILD_DIR/$CPU
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#x86_64
#CPU=x86_64
#ARCH=x86_64
#OPTIMIZE_CFLAGS="-fomit-frame-pointer"
#PREFIX=$BUILD_DIR/$CPU
#ADDITIONAL_CONFIGURE_FLAG=
#build_one

#x86
#CPU=i686
#ARCH=i686
#OPTIMIZE_CFLAGS="-fomit-frame-pointer"
#PREFIX=$BUILD_DIR/$CPU
#ADDITIONAL_CONFIGURE_FLAG=
#build_one
