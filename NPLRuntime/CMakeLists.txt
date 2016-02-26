# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2015.2.21
# Note: Build only paraengine server or NPL runtime
# Debian install:
# apt-get install cmake
# apt-get install libboost-all-dev
# apt-get install libssl-dev
# apt-get install libssh2-1-dev
# apt-get install build-essential
# curlbuild.h file must be deleted or fuzzy errors will appear when build curl
# rm -f Server/trunk/curl-7.47.1/include/curl/curlbuild.h
# apt-get install freeglut3 freeglut3-dev libglew1.5 libglew1.5-dev libglu1-mesa libglu1-mesa-dev libgl1-mesa-glx libgl1-mesa-dev

cmake_minimum_required (VERSION 2.6)

project (NPLRuntime)

set(SERVER_SOURCE_DIR   ${NPLRuntime_SOURCE_DIR}/../Server/trunk/)

# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if (PARAENGINE_CLIENT OR WIN32)
	set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_55_0)
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)
	else()
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
else(PARAENGINE_CLIENT OR WIN32)
	# set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_55_0)
endif(PARAENGINE_CLIENT OR WIN32)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

# Add more boost components here
find_package( Boost 1.55 REQUIRED COMPONENTS thread date_time filesystem system regex signals serialization iostreams chrono) 

# Note: LiXizhi 2010.3. there is a bug in FindBoost that one needs to link with ${Boost_LIBRARIES_static_runtime} in order to use static vc runtime time. 
STRING(REGEX REPLACE "\\-mt\\-" "-mt-s-" Boost_LIBRARIES_static_runtime "${Boost_LIBRARIES}" )
STRING(REGEX REPLACE "\\-mt\\-s\\-s\\-" "-mt-s-" Boost_LIBRARIES_static_runtime "${Boost_LIBRARIES_static_runtime}" )
STRING(REGEX REPLACE "s\\-gd" "sgd" Boost_LIBRARIES_static_runtime "${Boost_LIBRARIES_static_runtime}" )

# since we are linking statically everywhere, replace Boost_LIBRARIES with static one. 
set(Boost_LIBRARIES ${Boost_LIBRARIES_static_runtime})

if (WIN32)
	option(BUILD_CURL_EXE OFF)
	option(BUILD_CURL_TESTS OFF)
    add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.3 ${NPLRuntime_BINARY_DIR}/zlib-1.2.3)
	add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${NPLRuntime_BINARY_DIR}/curl-7.47.1)
else()
	FIND_PACKAGE(CURL REQUIRED)
endif()

add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${NPLRuntime_BINARY_DIR}/lua-5.1.4)
add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${NPLRuntime_BINARY_DIR}/luabind-0.9)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${NPLRuntime_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${NPLRuntime_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${NPLRuntime_BINARY_DIR}/tinyxpath_1_3_1)
# add_subdirectory (${SERVER_SOURCE_DIR}NPLMono  ${NPLRuntime_BINARY_DIR}/NPLMono)
#add_subdirectory (${SERVER_SOURCE_DIR}luaprofiler-2.0.2  ${NPLRuntime_BINARY_DIR}/luaprofiler-2.0.2)
#add_subdirectory (${SERVER_SOURCE_DIR}luasql ${NPLRuntime_BINARY_DIR}/luasql)
add_subdirectory (ParaEngineServer ${NPLRuntime_BINARY_DIR}/ParaEngineServer)
