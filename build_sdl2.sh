
#!/bin/bash

SOURCE_DIR=`pwd`
LIB_VERSION=2.0.22
LIB_NAME=SDL2-${LIB_VERSION}
LIB_SRC_TAR_GZ=${SOURCE_DIR}/${LIB_NAME}.tar.gz 
LIB_SRC_DIR=${SOURCE_DIR}/${LIB_NAME}
LIB_BUILD_DIR=${LIB_SRC_DIR}/out
LIB_INSTALL_DIR=${LIB_SRC_DIR}/install

if [ ! -d ${LIB_INSTALL_DIR} ]; then
    if [ ! -d ${LIB_SRC_DIR} ]; then
        if [ -f ${LIB_SRC_TAR_GZ} ]; then
            tar -vxf ${LIB_SRC_TAR_GZ} -C ${SOURCE_DIR}
        else
            wget https://github.com/libsdl-org/SDL/releases/download/release-${LIB_VERSION}/${LIB_NAME}.tar.gz -O ${LIB_SRC_TAR_GZ}
            tar -vxf ${LIB_SRC_TAR_GZ} -C ${SOURCE_DIR}
        fi
    fi

	cmake -S ${LIB_SRC_DIR} -B ${LIB_BUILD_DIR} -DCMAKE_INSTALL_PREFIX=${LIB_INSTALL_DIR}
	cmake --build ${LIB_BUILD_DIR}
	cmake --install ${LIB_BUILD_DIR}
fi

export SDL_ROOT=${LIB_INSTALL_DIR}

#export BOOST_ROOT=${SOURCE_DIR}/bin/boost_1_78_0

cmake -S NPLRuntime/ -B out -DLINUX=true -DSDL_ROOT=${SDL_ROOT} -DALSOFT_BACKEND_SDL2=TRUE -DALSOFT_BACKEND_OSS=FALSE -DALSOFT_BACKEND_WAVE=FALSE -DALSOFT_BACKEND_ALSA=FALSE

cmake --build out
