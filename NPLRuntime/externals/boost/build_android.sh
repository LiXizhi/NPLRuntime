#!/bin/bash
#USE MINGW (MSYS2) on Windows
#NDK VERSION: r12b_x64
CURRENT_DIR=`pwd`
BOOST_SRC="./prebuild/android/src/boost_1_64_0"
IOS_OUTPUT_DIR="$OUTPUT_DIR/ios"
TOOLCHAIN_DIR="$CURRENT_DIR/prebuild/android/toolchain/"
LOG_PATH="$CURRENT_DIR/prebuild/android/android-build.log"


doneSection()
{
    echo
    echo "Done"
    echo "================================================================="
    echo
}



updateBoost()
{
    echo Updating boost into $BOOST_SRC...



cat > "$BOOST_SRC/android-config.jam" <<EOF
import os ;
using clang : android
:
$TOOLCHAIN_DIR/bin/clang++
:
<compileflags>-fexceptions
<compileflags>-frtti
<compileflags>-fpic
<compileflags>-ffunction-sections
<compileflags>-funwind-tables
<compileflags>-march=armv7-a
<compileflags>-mfloat-abi=softfp
<compileflags>-mfpu=vfpv3-d16
<compileflags>-fomit-frame-pointer
<compileflags>-fno-strict-aliasing
<compileflags>-I$TOOLCHAIN_DIR/sysroot/usr/include
<compileflags>-Wa,--noexecstack
<compileflags>-DANDROID
<compileflags>-D__ANDROID__
<compileflags>-DNDEBUG
<compileflags>-O2
#<compileflags>-g
<compileflags>-I$TOOLCHAIN_DIR/include/C++/4.9x
<compileflags>-I$TOOLCHAIN_DIR/include/C++/4.9x/arm-linux-androideabi
<architecture>arm
<compileflags>-fvisibility=hidden
<compileflags>-fvisibility-inlines-hidden
<compileflags>-fdata-sections
<cxxflags>-D__arm__
;
EOF
    


    doneSection
}


checkSrc()
{
    if [ ! -d $BOOST_SRC ]; then
        echo " Not found boost src at $BOOST_SRC"
        exit 1
    fi 
}

makeToolchain()
{
    if [ ! -d $TOOLCHAIN_DIR ]; then
        echo Making android ndk toolchain 
        $ANDROID_NDK\\build\\tools\\make-standalone-toolchain.sh --use-llvm  --arch=arm  --platform=android-21 --install-dir=$TOOLCHAIN_DIR
    fi
}


buildAndroid()
{
    cd "$BOOST_SRC"
    echo Building Boost for Android 

    ./bootstrap.sh --with-toolset=gcc >> $LOG_PATH 2>&1
    ./b2  --user-config=android-config.jam --build-dir=android-build --stagedir=android-build/stage \
    toolset=clang-android \
    target-os=linux \
    threadapi=pthread \
    threading=multi link=static runtime-link=static \
    --with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-signals --with-regex --with-serialization --with-iostreams --with-log \
    >> $LOG_PATH 2>&1
    if [ $? != 0 ]; then echo "Error staging Android. Check log."; exit 1; fi
    doneSection
}

checkSrc
makeToolchain
updateBoost
buildAndroid
echo "Completed successfully"