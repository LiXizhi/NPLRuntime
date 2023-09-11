
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
    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null

    # arm64
    ISDKF="-arch arm64 -isysroot $ISDKP -miphoneos-version-min=13.0"
    make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CC="clang " CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET=arm64 TARGET_SYS=iOS clean >> "${LOG_DIR}/ios-build.log" 2>&1
    make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CC="clang " CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET=arm64 TARGET_SYS=iOS >> "${LOG_DIR}/ios-build.log" 2>&1

    if [ $? != 0 ]; then echo "Error building iPhoneOS-arm64. Check log."; exit 1; fi

    mkdir $BUILD_DIR/iphoneos/
    mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/iphoneos/libluajit.a

    make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
    doneSection
}

# buildLuajit_iphonesimulator()
# {
#     # Build Luajit for iphonesimulator
#     echo Build Luajit for iPhoneSimulator
    
#     ISDKP=$(xcrun --sdk iphonesimulator --show-sdk-path)
#     ICC=$(xcrun --sdk iphonesimulator --find clang)
   
#     # i386
#     ISDKF="-arch i386 -mios-simulator-version-min=10.0 -isysroot $ISDKP"
#     make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
#     make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CFLAGS="-arch i386" HOST_LDFLAGS="-arch i386" \
#      TARGET=x86 CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS >> "${LOG_DIR}/iphonesim-build.log" 2>&1
#     if [ $? != 0 ]; then echo "Error building iphonesim-i386. Check log."; exit 1; fi
    
#     mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/libluajit_i386.a
    

#     # x86-64
#     ISDKF="-arch x86_64 -mios-simulator-version-min=10.0 -isysroot $ISDKP"
#     make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
#     make -C $SOURCE_ROOT DEFAULT_CC=clang HOST_CFLAGS="-arch x86_64" HOST_LDFLAGS="-arch x86_64" \
#      TARGET=x86_64 CROSS="$(dirname $ICC)/" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS >> "${LOG_DIR}/iphonesim-build.log" 2>&1
#     if [ $? != 0 ]; then echo "Error building iphonesim-x86_64. Check log."; exit 1; fi
    
#     mv $SOURCE_ROOT/src/libluajit.a $BUILD_DIR/libluajit_x86_64.a
    
   
#     lipo -create  $BUILD_DIR/libluajit_i386.a $BUILD_DIR/libluajit_x86_64.a \
#         -output $BUILD_DIR/libluajit.a
#     rm -r $BUILD_DIR/libluajit_x86_64.a
#     rm -r $BUILD_DIR/libluajit_i386.a

#     mkdir -p $BUILD_DIR/iphonesim
#     mv  $BUILD_DIR/libluajit.a $BUILD_DIR/iphonesim/libluajit.a
#     make -C $SOURCE_ROOT clean 1>/dev/null 2>/dev/null
#     doneSection
# }

cleanup
buildLuajit_iphoneos
# buildLuajit_iphonesimulator

echo "Completed successfully"