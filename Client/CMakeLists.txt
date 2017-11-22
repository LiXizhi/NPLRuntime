# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 3.3)
project (CLIENT)

set(CMAKE_CXX_STANDARD 11)

# Options
option(PARAENGINE_CLIENT_DLL "Generate Dll Or Exe" OFF)
option(NPLRUNTIME_STATIC_LIB "static lib or not" OFF)
option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" OFF)
option(NPLRUNTIME_LUA51   "build lua5.1.5 with coco" OFF)
option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
option(NPLRUNTIME_PHYSICS "include physics" OFF)


# Using static lib for other modules if main NPLRuntime is built as static lib. 
if(NPLRUNTIME_STATIC_LIB)
set(CURL_STATICLIB TRUE)
set(LUA_STATICLIB TRUE)
set(SQLITE_STATICLIB TRUE)
endif()


# This is the directory to keep all binary
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(OUTPUT_BIN_DIR   ${CLIENT_SOURCE_DIR}/../ParaWorld/bin64/)
else()
set(OUTPUT_BIN_DIR   ${CLIENT_SOURCE_DIR}/../ParaWorld/bin32/)
endif()


# Server source dir
set(SERVER_SOURCE_DIR   ${CLIENT_SOURCE_DIR}/../Server/trunk/)


# == Boost ==
#{
# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if ("$ENV{BOOST_ROOT}" STREQUAL "")
if(IS_DIRECTORY ${SERVER_SOURCE_DIR}/boost_1_65_0)
	set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_65_0)
elseif(IS_DIRECTORY ${SERVER_SOURCE_DIR}/boost_1_61_0)
	set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_61_0)
else()
	message(WARNING "You can define a global environment variable of BOOST_ROOT that specify the boost root dir")
endif()
endif()
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)
else()
set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)

# Add more boost components here
find_package( Boost 1.65 REQUIRED COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams) 
# }
# End of boost




if(WIN32)
	include(cmake/win32.cmake)
elseif(APPLE)
	include(cmake/osx.cmake)
endif()

