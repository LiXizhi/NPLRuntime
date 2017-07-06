# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 2.6)

project (CLIENT)

# this is the directory to keep all binary
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(OUTPUT_BIN_DIR   ${CLIENT_SOURCE_DIR}/../ParaWorld/bin64/)
else()
	set(OUTPUT_BIN_DIR   ${CLIENT_SOURCE_DIR}/../ParaWorld/bin32/)
endif()

set(SERVER_SOURCE_DIR   ${CLIENT_SOURCE_DIR}/../Server/trunk/)

# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if (PARAENGINE_CLIENT OR WIN32)
	if ("$ENV{BOOST_ROOT}" STREQUAL "")
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_61_0)
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
find_package( Boost 1.61 REQUIRED COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams) 

if (WIN32)
	option(BUILD_CURL_EXE OFF)
	option(BUILD_CURL_TESTS OFF)
	option(PARAENGINE_CLIENT_DLL OFF)
	add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.3 ${CLIENT_BINARY_DIR}/zlib-1.2.3)
	set(ZLIB_FOUND 1)
	set(ZLIB_LIBRARY zlib)
	set(ZLIB_LIBRARIES zlib)
	set(ZLIB_INCLUDE_DIR ${SERVER_SOURCE_DIR}zlib-1.2.3)
  
	add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${CLIENT_BINARY_DIR}/curl-7.47.1)
else()
	FIND_PACKAGE(CURL REQUIRED)
	# making executable relocatable 
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "$ORIGIN/")
endif()

add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${CLIENT_BINARY_DIR}/lua-5.1.4)
add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${CLIENT_BINARY_DIR}/LuaJIT)
add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT-2.1 ${CLIENT_BINARY_DIR}/LuaJIT-2.1)
add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${CLIENT_BINARY_DIR}/luabind-0.9)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${CLIENT_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${CLIENT_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${CLIENT_BINARY_DIR}/tinyxpath_1_3_1)
add_subdirectory (${SERVER_SOURCE_DIR}luasql ${CLIENT_BINARY_DIR}/luasql)

add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/assimp-3.1.1 ${CLIENT_BINARY_DIR}/assimp-3.1.1)

# main NPL runtime using ParaEngineClient
add_subdirectory (${CLIENT_SOURCE_DIR}/../NPLRuntime/ParaEngineClient ${CLIENT_BINARY_DIR}/ParaEngineClient)

# expose the core include directory 
set (ParaEngineClient_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient)

if(PARAENGINE_CLIENT_DLL)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/bullet3 ${CLIENT_BINARY_DIR}/bullet3)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/PhysicsBT ${CLIENT_BINARY_DIR}/PhysicsBT)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/ParaEngineClientApp ${CLIENT_BINARY_DIR}/ParaEngineClientApp)
endif()
	
# following are plugins that should be build after main runtime
add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${CLIENT_BINARY_DIR}/NPLMono/NPLMono2)
