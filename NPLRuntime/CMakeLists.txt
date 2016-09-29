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

# this is the directory to keep all binary
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(OUTPUT_BIN_DIR   ${NPLRuntime_SOURCE_DIR}/../ParaWorld/bin64/)
else()
	set(OUTPUT_BIN_DIR   ${NPLRuntime_SOURCE_DIR}/../ParaWorld/)
endif()

set(SERVER_SOURCE_DIR   ${NPLRuntime_SOURCE_DIR}/../Server/trunk/)

# static linking to all libraries, such as boost, sqlite3, curl. This will produce big executable, but easy to deploy as binary. 
option(NPL_STATIC_LINK_ALL "link statically as much as possible" ON)

# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if (PARAENGINE_CLIENT OR WIN32)
	if ("$ENV{BOOST_ROOT}" STREQUAL "")
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_55_0)
	endif()
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)
	else()
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
endif(PARAENGINE_CLIENT OR WIN32)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)

# Add more boost components here
find_package( Boost 1.55 REQUIRED COMPONENTS thread date_time filesystem system chrono signals serialization iostreams regex) 

if (WIN32)
	option(BUILD_CURL_EXE OFF)
	option(BUILD_CURL_TESTS OFF)
	add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${NPLRuntime_BINARY_DIR}/curl-7.47.1)
    add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.3 ${NPLRuntime_BINARY_DIR}/zlib-1.2.3)
else()
	FIND_PACKAGE(CURL REQUIRED)
	# making executable relocatable 
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "$ORIGIN/")
endif()

add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${NPLRuntime_BINARY_DIR}/lua-5.1.4)
add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${NPLRuntime_BINARY_DIR}/LuaJIT)
if (APPLE)  # only use the latest luabind on macosx/macos-sierra
    add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9.2beta ${NPLRuntime_BINARY_DIR}/luabind-0.9.2beta)
else()
    add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${NPLRuntime_BINARY_DIR}/luabind-0.9)
endif()
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${NPLRuntime_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${NPLRuntime_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${NPLRuntime_BINARY_DIR}/tinyxpath_1_3_1)
#add_subdirectory (${SERVER_SOURCE_DIR}luaprofiler-2.0.2  ${NPLRuntime_BINARY_DIR}/luaprofiler-2.0.2)
add_subdirectory (${SERVER_SOURCE_DIR}luasql ${NPLRuntime_BINARY_DIR}/luasql)

# main NPL runtime using ParaEngineServer
add_subdirectory (ParaEngineServer ${NPLRuntime_BINARY_DIR}/ParaEngineServer)

# following are plugins that should be build after main runtime
add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${NPLRuntime_BINARY_DIR}/NPLMono/NPLMono2)
