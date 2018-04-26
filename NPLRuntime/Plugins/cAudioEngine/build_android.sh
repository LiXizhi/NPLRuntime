#!/bin/sh
SOURCE_DIR=`pwd`

#set config path
NDK="/opt/android-ndk-r9d"
PARAENGINE_ROOT="/opt/ParaEngine-Git"
CMAKE_TOOLCHAIN_ROOT="/opt/android-cmake"
ANDROID_ABI="armeabi"
ANDROID_NATIVE_API_LEVEL="android-9"
DESTDIR="$SOURCE_DIR/prebuilt/android/${ANDROID_ABI}"

rm -rf $DESTDIR
mkdir -p $DESTDIR

#build openal and install it
BUILD_DIR="${SOURCE_DIR}/DependenciesSource/openal-soft-1.16.0/android"
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
#cmake openal
cmake   -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_ROOT}/android.toolchain.cmake      \
        -DANDROID_NDK=${NDK}                                                        \
        -DCMAKE_BUILD_TYPE=Release                                                  \
        -DANDROID_ABI=${ANDROID_ABI}                                                \
        -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}                      \
        -DLIBTYPE="STATIC"                                                          \
        -DBUILD_ON_ANDROID=TRUE                                                     \
        ..
make
make install

if [ -f $BUILD_DIR/libcommon.a ]; then
    mv $BUILD_DIR/libcommon.a $DESTDIR/libcommon.a
fi;
if [ -f $BUILD_DIR/libopenal.a ]; then
    mv $BUILD_DIR/libopenal.a $DESTDIR/libopenal.a
fi;

#build cAudioEngine
BUILD_DIR="${SOURCE_DIR}/android"
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR


#cmake
cmake   -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_ROOT}/android.toolchain.cmake      \
        -DANDROID_NDK=${NDK}                                                        \
        -DCMAKE_BUILD_TYPE=Release                                                  \
        -DANDROID_ABI=${ANDROID_ABI}                                                \
        -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}                      \
        -DCAUDIO_STATIC=TRUE                                                        \
        -DBUILD_ON_ANDROID=TRUE                                                     \
        -DPARAENGINE_ROOT=${PARAENGINE_ROOT}                                        \
        ..

make
if [ -f $BUILD_DIR/cAudio/libcAudio.a ]; then
    mv $BUILD_DIR/cAudio/libcAudio.a $DESTDIR/libcAudio.a
fi;
if [ -f $BUILD_DIR/DependenciesSource/libogg-1.3.2/libOgg.a ]; then
    mv $BUILD_DIR/DependenciesSource/libogg-1.3.2/libOgg.a $DESTDIR/libOgg.a
fi;
if [ -f $BUILD_DIR/DependenciesSource/libvorbis-1.3.2/libVorbis.a ]; then
    mv $BUILD_DIR/DependenciesSource/libvorbis-1.3.2/libVorbis.a $DESTDIR/libVorbis.a
fi;
if [ -f $BUILD_DIR/Plugins/mp3Decoder/libcAp_mp3Decoder.a ]; then
    mv $BUILD_DIR/Plugins/mp3Decoder/libcAp_mp3Decoder.a $DESTDIR/libcAp_mp3Decoder.a
fi;