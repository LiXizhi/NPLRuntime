#!/bin/bash

CURRENT_DIRECTORY=`pwd`
cp -fr ${CURRENT_DIRECTORY}/NPLRuntime/externals/EmbedResource ${CURRENT_DIRECTORY}/EmbedResource
cat > ${CURRENT_DIRECTORY}/EmbedResource/CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.14)
project(embed-resource)
include(${CURRENT_DIRECTORY}/NPLRuntime/cmake/ConfigBoost.cmake)
find_package(Boost COMPONENTS filesystem REQUIRED)
include_directories("\${Boost_INCLUDE_DIRS}")
add_executable(embed-resource  embedresource.cpp embedresource.h README.md)
target_link_libraries(embed-resource \${Boost_FILESYSTEM_LIBRARY} \${Boost_SYSTEM_LIBRARY})
EOF
mkdir -p ${CURRENT_DIRECTORY}/EmbedResource/build
cmake -B ${CURRENT_DIRECTORY}/EmbedResource/build -S ${CURRENT_DIRECTORY}/EmbedResource 
cmake --build ${CURRENT_DIRECTORY}/EmbedResource/build
sudo cp ${CURRENT_DIRECTORY}/EmbedResource/build/embed-resource /usr/bin
ls -l /usr/bin/embed-resource
if [ $? -ne 0 ]; then
    exit 1
fi


export NDK_VERSION=23.1.7779620

# GITHUB WORKFLOWS
if [[ "$GITHUB_WORKFLOW" != "" ]]; then
    # Java 环境验证
    echo "Running workflow $GITHUB_WORKFLOW (event: $GITHUB_EVENT_NAME, action: $GITHUB_ACTION)"
    # Force Java to be Java 11 minimum, it defaults to 8 in GitHub runners for some platforms
    export JAVA_HOME=${JAVA_HOME_11_X64}
    java_version=$(java -version 2>&1 | head -1 | cut -d'"' -f2 | sed '/^1\./s///' | cut -d'.' -f1)
    if [[ "$java_version" < 11 ]]; then
        echo "Android builds require Java 11, found version ${java_version} instead"
        exit 1
    fi

    # 安装NDK
    if [[ ! -d "${ANDROID_HOME}/ndk/$NDK_VERSION" ]]; then
        ${ANDROID_HOME}/tools/bin/sdkmanager "ndk;$NDK_VERSION" > /dev/null 2>&1
    fi

    # Install ninja
    NINJA_VERSION=1.10.2
    wget -q https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/ninja-linux.zip
    unzip -q ninja-linux.zip
    sudo cp ninja /usr/bin
    export PATH="$PWD:$PATH"

    # Install CMake
    CMAKE_VERSION=3.22.3
    mkdir -p cmake
    cd cmake
    sudo wget -q https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-Linux-x86_64.sh
    sudo chmod +x ./cmake-$CMAKE_VERSION-Linux-x86_64.sh
    sudo ./cmake-$CMAKE_VERSION-Linux-x86_64.sh --skip-license > /dev/null
    sudo update-alternatives --install /usr/bin/cmake cmake $(pwd)/bin/cmake 1000 --force
    cd -


    export SDK_ROOT=$ANDROID_HOME
    export NDK_ROOT=${SDK_ROOT}/ndk/${NDK_VERSION}
fi

export LIB_ROOT_DIR=${CURRENT_DIRECTORY}/libs
export INSTALL_DIR=${LIB_ROOT_DIR}/android
export SOURCE_DIR=${LIB_ROOT_DIR}/sources
export BUILD_DIR=${LIB_ROOT_DIR}/build

echo ${CURRENT_DIRECTORY}

# android sdk root
if [ "$SDK_ROOT" == "" ]; then
    if [ "$ANDROID_HOME" != "" ]; then
        export SDK_ROOT=$ANDROID_HOME
    else
        echo "android sdk not exist"
        exit 1
    fi
fi

# NDK 环境变量
if [ "$NDK_ROOT" == "" ]; then
    export NDK_ROOT=${SDK_ROOT}/ndk/${NDK_VERSION}
fi

echo "NDK_ROOT=${NDK_ROOT}"

# TOOLCHAIN
export TOOLCHAIN=$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64

# SYSROOT
export SYSROOT=${TOOLCHAIN}/sysroot

# target host
export TARGET=aarch64-linux-android
export ANDROID_ABI=arm64-v8a
if [ "$1" = "arm64-android" ]; then
	TARGET=aarch64-linux-android
	ANDROID_ABI=arm64-v8a
elif [ "$1" = "arm-android" ]; then
	TARGET=armv7a-linux-androideabi
	ANDROID_ABI=armeabi-v7a
elif [ "$1" = "x64-android" ]; then
	TARGET=x86_64-linux-android
	ANDROID_ABI=x86_64
elif [ "$1" = "x86-android" ]; then
	TARGET=i686-linux-android
	ANDROID_ABI=x86
fi

# 更新安装目录
INSTALL_DIR=${INSTALL_DIR}/${ANDROID_ABI}
BUILD_DIR=${BUILD_DIR}/${ANDROID_ABI}

# ANDROID_NATIVE_API_LEVEL
export API_LEVEL=31

PATH=$TOOLCHAIN/bin:$PATH
export CC=$TARGET${API_LEVEL}-clang
export CXX=$TARGET${API_LEVEL}-clang++
export AR=llvm-ar
export AS=$CC
export LD=ld
export RANLIB=llvm-ranlib
export STRIP=llvm-strip

# cmake build
export CMAKE_ROOT=${SDK_ROOT}/cmake/3.20.0
export CMAKE_TOOLCHAIN_FILE=${NDK_ROOT}/build/cmake/android.toolchain.cmake
export ANDROID_NDK=${NDK_ROOT}
export ANDROID_PLATFORM=android-${API_LEVEL}


rm -fr ${BUILD_DIR}
mkdir -p ${LIB_ROOT_DIR}
mkdir -p ${INSTALL_DIR}
mkdir -p ${SOURCE_DIR}
mkdir -p ${BUILD_DIR}



# boost 安装
LIB_VERSION=1.78.0
LIB_NAME=boost_1_78_0
LIB_SRC_TAR_GZ=${SOURCE_DIR}/${LIB_NAME}.tar.gz
LIB_SRC_DIR=${SOURCE_DIR}/${LIB_NAME}
LIB_BUILD_DIR=${BUILD_DIR}/${LIB_NAME}
LIB_INSTALL_DIR=${INSTALL_DIR}/${LIB_NAME}
BOOST_ROOT_DIR=${LIB_INSTALL_DIR}

rm -fr ${LIB_BUILD_DIR}
rm -fr ${LIB_INSTALL_DIR}

# 源码检测
cd ${SOURCE_DIR}
if [ ! -d ${LIB_SRC_DIR} ]; then
	if [ -f ${LIB_SRC_TAR_GZ} ]; then
		tar -vxf ${LIB_SRC_TAR_GZ} -C ${SOURCE_DIR}
	else
		wget -q https://boostorg.jfrog.io/artifactory/main/release/${LIB_VERSION}/source/${LIB_NAME}.tar.gz -O ${LIB_SRC_TAR_GZ}
		tar -xf ${LIB_SRC_TAR_GZ} -C ${SOURCE_DIR}
	fi
fi
cd -


cd ${LIB_SRC_DIR}

./bootstrap.sh > /dev/null 2>&1

mkdir -p ${LIB_BUILD_DIR}
#<compileflags>-D__ANDROID_API__=${API_LEVEL}
cat > ${LIB_BUILD_DIR}/android-user-config.jam <<EOF
import os ;
import option ;
import build-system ;

option.set layout : system ;

modules.poke : NO_BZIP2 : 1 ;

using clang : android : ${CXX} :
<compileflags>--sysroot=${TOOLCHAIN}/sysroot
<compileflags>-fexceptions
<compileflags>-frtti
<compileflags>-fpic
<compileflags>-D_LITTLE_ENDIAN
<compileflags>-ffunction-sections
<compileflags>-funwind-tables
<compileflags>-fno-strict-aliasing
<compileflags>-DANDROID
<compileflags>-D__ANDROID__
<compileflags>-DNDEBUG
<compileflags>-O2
<compileflags>-g
<compileflags>-fvisibility=hidden
<compileflags>-fvisibility-inlines-hidden
<compileflags>-fdata-sections
<archiver>${AR}
<ranlib>${RANLIB}
;

EOF

ADDRESS_MODEL=32
if [ "${ANDROID_ABI}" = "arm64-v8a" ]; then
	ADDRESS_MODEL=64
fi

PARAMS="--user-config=${LIB_BUILD_DIR}/android-user-config.jam --prefix=${LIB_INSTALL_DIR} --build-dir=${LIB_BUILD_DIR} --stagedir=${LIB_BUILD_DIR}/stage toolset=clang-android target-os=android threadapi=pthread threading=multi link=static runtime-link=static address-model=${ADDRESS_MODEL} abi=aapcs"
PARAMS="--with-thread --with-date_time --with-filesystem --with-system --with-chrono --with-regex --with-serialization --with-iostreams --with-log ${PARAMS} define=BOOST_FILESYSTEM_DISABLE_STATX"

./b2 install ${PARAMS} > /dev/null 2>&1

export BOOST_ROOT=${BOOST_ROOT_DIR}
cd ${CURRENT_DIRECTORY}/NPLRuntime/Platform/AndroidStudio


bash gradlew assembleRelease
#mkdir -p ./bin/linux
#cd bin/linux/

#cmake -DCMAKE_BUILD_TYPE=Release ../../NPLRuntime/ -DBOOST_ROOT=${BOOST_ROOT_DIR} -DGITHUB_WORKFLOWS=TRUE -DUSE_NULL_RENDERER=TRUE -DANDROID=TRUE
#if [ $? -ne 0 ]; then
    #exit 1
#fi
#make --jobs=1
#if [ $? -ne 0 ]; then
    #exit 1
#fi
#cd -
