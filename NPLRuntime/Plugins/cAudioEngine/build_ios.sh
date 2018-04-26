#!/bin/sh
SOURCE_DIR=`pwd`

#set config path
PARAENGINE_ROOT="/Users/work/Documents/ParaEngine"
DESTDIR="$SOURCE_DIR/prebuilt/ios"
IOS_SDK="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS8.2.sdk"
rm -rf $DESTDIR
mkdir -p $DESTDIR



#build cAudioEngine
BUILD_DIR="${SOURCE_DIR}/ios"
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR


#cmake
cmake   -DCAUDIO_IOS_BUILD=TRUE                                                         \
        -DCAUDIO_STATIC=TRUE                                                            \
        -DPARAENGINE_ROOT=${PARAENGINE_ROOT}                                            \
        -GXcode                                                                         \
        -DIOS_SDK=${IOS_SDK}                                                \
        ..

cd ..
chmod -R 777 $BUILD_DIR
cd $BUILD_DIR
#xcodebuild
xcodebuild -configuration Release

mv $BUILD_DIR/cAudio/Release-iphoneos/libcAudio.a $DESTDIR/libcAudio.a
mv $BUILD_DIR/DependenciesSource/libogg-1.3.2/Release-iphoneos/libOgg.a $DESTDIR/libOgg.a
mv $BUILD_DIR/DependenciesSource/libvorbis-1.3.2/Release-iphoneos/libVorbis.a $DESTDIR/libVorbis.a
mv $BUILD_DIR/Plugins/mp3Decoder/Release-iphoneos/libcAp_mp3Decoder.a $DESTDIR/libcAp_mp3Decoder.a
