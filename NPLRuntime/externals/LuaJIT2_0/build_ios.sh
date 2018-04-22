
#http://luajit.org/install.html#cross

SOURCE_ROOT=./code
CURRENT_DIR=`pwd`
BUILD_DIR=$CURRENT_DIR/prebuilt/ios
LOG_DIR=$CURRENT_DIR/log
INCLUDEDIR=$CURRENT_DIR/include

doneSection()
{
    echo
    echo "Done"
    echo "================================================================="
    echo
}

cleanup()
{
    echo Cleaning everything
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    rm *.a 1>/dev/null 2>/dev/null
    rm -rf $LOG_DIR
    mkdir -p $LOG_DIR
}

buildLuajit_iphoneos()
{
    # Build Luajit for iphoneos
    echo Build Luajit for iphoneos
    
    ISDKP=$(xcrun --sdk iphoneos --show-sdk-path)
    ICC=$(xcrun --sdk iphoneos --find clang)
   
    # armv7
    ISDKF="-arch armv7 -isysroot $ISDKP -mios-simulator-version-min=10.0"
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CC="clang -m32 " \
        CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS >> "${LOG_DIR}/ios-build.log" 2>&1
    if [ $? != 0 ]; then echo "Error building iPhoneOS-armv7. Check log."; exit 1; fi
    
    mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/libluajit_armv7.a
    

    # armv7s
    ISDKF="-arch armv7s -isysroot $ISDKP -mios-simulator-version-min=10.0"
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CC="clang -m32" \
        CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS >> "${LOG_DIR}/ios-build.log" 2>&1
    if [ $? != 0 ]; then echo "Error building iPhoneOS-armv7s. Check log."; exit 1; fi
    
    mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/libluajit_armv7s.a
   
    
    lipo -create  $BUILD_DIR/libluajit_armv7.a $BUILD_DIR/libluajit_armv7s.a  \
        -output $BUILD_DIR/libluajit.a
    rm -r $BUILD_DIR/libluajit_armv7.a
    rm -r $BUILD_DIR/libluajit_armv7s.a
    mkdir -p $BUILD_DIR/iphoneos
    mv  $BUILD_DIR/libluajit.a $BUILD_DIR/iphoneos/libluajit.a
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    doneSection
}


buildLuajit_iphonesimulator()
{
    # Build Luajit for iphonesimulator
    echo Build Luajit for iPhoneSimulator
    
    ISDKP=$(xcrun --sdk iphonesimulator --show-sdk-path)
    ICC=$(xcrun --sdk iphonesimulator --find clang)
   
    # i386
    ISDKF="-arch i386 -mios-simulator-version-min=10.0 -isysroot $ISDKP"
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CFLAGS="-arch i386" HOST_LDFLAGS="-arch i386" \
     TARGET=x86 CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS >> "${LOG_DIR}/iphonesim-build.log" 2>&1
    if [ $? != 0 ]; then echo "Error building iphonesim-i386. Check log."; exit 1; fi
    
    mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/libluajit_i386.a
    
    mkdir -p $BUILD_DIR/iphonesim
    mv  $BUILD_DIR/libluajit_i386.a $BUILD_DIR/iphonesim/libluajit.a
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    doneSection
}

cleanup
buildLuajit_iphoneos
buildLuajit_iphonesimulator

echo "Completed successfully"