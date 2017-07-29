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
        -DBUILD_ON_ANDROID=TRUE                                                     \
        -DPARAENGINE_ROOT=${PARAENGINE_ROOT}                                        \
        ..

make
if [ -f $BUILD_DIR/src/Bullet3Common/libBullet3Common.a ]; then
    mv $BUILD_DIR/src/Bullet3Common/libBullet3Common.a $DESTDIR/libBullet3Common.a
fi;
if [ -f $BUILD_DIR/src/PhysicsBT/libPhysicsBT.a ]; then
    mv $BUILD_DIR/src/PhysicsBT/libPhysicsBT.a $DESTDIR/libPhysicsBT.a
fi;
if [ -f $BUILD_DIR/src/Bullet3Geometry/libBullet3Geometry.a ]; then
    mv $BUILD_DIR/src/Bullet3Geometry/libBullet3Geometry.a $DESTDIR/libBullet3Geometry.a
fi;
if [ -f $BUILD_DIR/src/BulletCollision/libBulletCollision.a ]; then
    mv $BUILD_DIR/src/BulletCollision/libBulletCollision.a $DESTDIR/libBulletCollision.a
fi;
if [ -f $BUILD_DIR/src/Bullet3OpenCL/libBullet3OpenCL_clew.a ]; then
    mv $BUILD_DIR/src/Bullet3OpenCL/libBullet3OpenCL_clew.a $DESTDIR/libBullet3OpenCL_clew.a
fi;
if [ -f $BUILD_DIR/src/BulletDynamics/libBulletDynamics.a ]; then
    mv $BUILD_DIR/src/BulletDynamics/libBulletDynamics.a $DESTDIR/libBulletDynamics.a
fi;
if [ -f $BUILD_DIR/src/Bullet3Dynamics/libBullet3Dynamics.a ]; then
    mv $BUILD_DIR/src/Bullet3Dynamics/libBullet3Dynamics.a $DESTDIR/libBullet3Dynamics.a
fi;
if [ -f $BUILD_DIR/src/Bullet3Serialize/Bullet2FileLoader/libBullet2FileLoader.a ]; then
    mv $BUILD_DIR/src/Bullet3Serialize/Bullet2FileLoader/libBullet2FileLoader.a $DESTDIR/libBullet2FileLoader.a
fi;
if [ -f $BUILD_DIR/src/Bullet3Collision/libBullet3Collision.a ]; then
    mv $BUILD_DIR/src/Bullet3Collision/libBullet3Collision.a $DESTDIR/libBullet3Collision.a
fi;
if [ -f $BUILD_DIR/src/BulletSoftBody/libBulletSoftBody.a ]; then
    mv $BUILD_DIR/src/BulletSoftBody/libBulletSoftBody.a $DESTDIR/libBulletSoftBody.a
fi;
if [ -f $BUILD_DIR/src/LinearMath/libLinearMath.a ]; then
    mv $BUILD_DIR/src/LinearMath/libLinearMath.a $DESTDIR/libLinearMath.a
fi;

