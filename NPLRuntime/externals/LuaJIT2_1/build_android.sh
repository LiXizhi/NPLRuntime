#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
#host_os=`uname -s | tr "[:upper:]" "[:lower:]"`
host_os=linux

SRCDIR=$DIR/code
cd "$SRCDIR"

NDK=/home/lanzhihong/桌面/ndk/android-ndk-r10e
NDKABI=8
NDKTRIPLE=arm-linux-androideabi
NDKVER=$NDK/toolchains/arm-linux-androideabi-4.8
NDKP=$NDKVER/prebuilt/${host_os}-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"



# Android/ARM, armeabi (ARMv5TE soft-float), Android 2.2+ (Froyo)
DESTDIR=$DIR/prebuilt/android/armeabi
rm "$DESTDIR"/*.a
make clean

make -j8 HOST_CC="gcc -m32" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF "

if [ -f $SRCDIR/src/libluajit.a ]; then
   mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

# Android/ARM, armeabi-v7a (ARMv7 VFP), Android 4.0+ (ICS)
NDKARCH="-march=armv7-a -mfloat-abi=softfp -Wl,--fix-cortex-a8"
DESTDIR=$DIR/prebuilt/android/armeabi-v7a
rm "$DESTDIR"/*.a
make clean
make -j8 HOST_CC="gcc -m32" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF $NDKARCH"

if [ -f $SRCDIR/src/libluajit.a ]; then
   mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

# Android/ARM, arm64-v8a
DESTDIR=$DIR/prebuilt/android/arm64-v8a

NDK=/home/lanzhihong/桌面/ndk/android-ndk-r14b
NDKABI=21
NDKTRIPLE=aarch64-linux-android
NDKVER=$NDK/toolchains/$NDKTRIPLE-4.9
NDKP=$NDKVER/prebuilt/${host_os}-x86_64/bin/$NDKTRIPLE-
NDKF="-isystem $NDK/sysroot/usr/include/$NDKTRIPLE -D__ANDROID_API__=$NDKABI"
NDK_SYSROOT_BUILD=$NDK/sysroot
NDK_SYSROOT_LINK=$NDK/platforms/android-$NDKABI/arch-arm64

make clean
make -j8 HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF" TARGET_SYS=Linux TARGET_SHLDFLAGS="--sysroot $NDK_SYSROOT_LINK"  TARGET_LDFLAGS="--sysroot $NDK_SYSROOT_LINK" TARGET_CFLAGS="--sysroot $NDK_SYSROOT_BUILD"

if [ -f $SRCDIR/src/libluajit.a ]; then
    mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

# Android/x86, x86 (i686 SSE3), Android 4.0+ (ICS)
#NDKABI=14
#DESTDIR=$DIR/prebuilt/android/x86
#NDKVER=$NDK/toolchains/x86-4.8
#NDKP=$NDKVER/prebuilt/${host_os}-x86_64/bin/i686-linux-android-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86"
#rm "$DESTDIR"/*.a
#make clean
#make HOST_CC="gcc -m32" CROSS=$NDKP TARGET_SYS=Linux TARGET_FLAGS="$NDKF"

#if [ -f $SRCDIR/src/libluajit.a ]; then
#    mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
#fi;

make clean
