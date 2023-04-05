# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 3.3)
set(NPLRUNTIME_PROJECT_NAME "CLIENT" CACHE STRING "Custom Project Name")
project(${NPLRUNTIME_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)


# Server source dir
set(SERVER_SOURCE_DIR   ${PROJECT_SOURCE_DIR}/../Server/trunk/)


# Options
################################
option(PARAENGINE_CLIENT_DLL "Generate Dll Or Exe" OFF)
option(NPLRUNTIME_STATIC_LIB "static lib or not" OFF)

# Win32 Default Option
if(WIN32)

	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
	option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
	option(NPLRUNTIME_LUA51   "build lua5.1.5 with coco" ON)
	option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" ON)
	option(NPLRUNTIME_PHYSICS "include physics" ON)
	option(NPLRUNTIME_CAUDIOENGINE "include audioengine plugin" OFF)
	option(NPLRUNTIME_HAQI_LAUNCHER "haqi launcher" OFF)
	option(NPLRUNTIME_WEBVIEW "microsoft edge webview2 support" OFF)
	
endif()
# OSX Default Option
if(APPLE)
	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
	option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
	option(NPLRUNTIME_PHYSICS "include physics" OFF)
	option(NPLRUNTIME_CAUDIOENGINE "include audioengine plugin" ON)
	option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
	option(NPLRUNTIME_LUA51   "build lua5.1.5 with coco" ON)	
	option(NPLRUNTIME_HAQI_LAUNCHER "haqi launcher" OFF)
endif()
# End of Options









#Renderer
################################
if(WIN32)

	set(NPLRUNTIME_RENDERER "DIRECTX" CACHE STRING "Render API")
	set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL)

endif()

if(APPLE)

	set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
	set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)

endif()


# Other platform depend options
################################
if(APPLE)

	# making executable relocatable 
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "$ORIGIN/") 

endif()





# Using static lib for other modules if main NPLRuntime is built as static lib. 

if(NPLRUNTIME_STATIC_LIB)
	set(CURL_STATICLIB TRUE)
	set(LUA_STATICLIB TRUE)
	set(SQLITE_STATICLIB TRUE)
endif()


# This is the directory to keep all binary
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
set(OUTPUT_BIN_DIR   ${PROJECT_SOURCE_DIR}/../ParaWorld/bin64/)
else()
set(OUTPUT_BIN_DIR   ${PROJECT_SOURCE_DIR}/../ParaWorld/bin32/)
endif()





#Boost
################################
# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if ("$ENV{BOOST_ROOT}" STREQUAL "")
	if(IS_DIRECTORY ${SERVER_SOURCE_DIR}/boost_1_65_0)
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_65_0)
	elseif(IS_DIRECTORY ${SERVER_SOURCE_DIR}/boost_1_61_0)
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_61_0)
	else()
		message(WARNING "You can define a global environment variable of BOOST_ROOT that specify the boost root dir")
	endif()
else()
	message("BOOST_ROOT env is at: $ENV{BOOST_ROOT}")
	set(BOOST_ROOT "$ENV{BOOST_ROOT}")
	string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib64)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)	
	endif()
else()
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib32)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)

if (MSVC)
	set(Boost_USE_STATIC_RUNTIME ON)
else()
	set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# Add more boost components here
find_package( Boost 1.61 REQUIRED COMPONENTS thread date_time filesystem system chrono regex serialization iostreams) 

# End of boost

#ZLIB
################################
if(WIN32)

	add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.11 ${PROJECT_BINARY_DIR}/zlib-1.2.11)
	set(ZLIB_FOUND 1)
	set(ZLIB_LIBRARY zlib)
	set(ZLIB_LIBRARIES zlib)
	set(ZLIB_INCLUDE_DIR ${SERVER_SOURCE_DIR}zlib-1.2.11 ${PROJECT_BINARY_DIR}/zlib-1.2.11)
	set(ZLIB_SOURCE_DIR ${ZLIB_INCLUDE_DIR})

endif()


# CURL
################################
if(WIN32)

	option(BUILD_CURL_EXE OFF)
	option(BUILD_CURL_TESTS OFF)
	add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${PROJECT_BINARY_DIR}/curl-7.47.1)
endif()


if(APPLE)

	FIND_PACKAGE(CURL REQUIRED)

endif()
# END OF CURL



# Lua
################################
if (NPLRUNTIME_LUAJIT21) 
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT-2.1 ${PROJECT_BINARY_DIR}/LuaJIT-2.1)
endif()
if (NPLRUNTIME_LUA51)
	add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${PROJECT_BINARY_DIR}/lua-5.1.4)
endif()
if(NPLRUNTIME_LUAJIT20)
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${PROJECT_BINARY_DIR}/LuaJIT)
endif()


add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${PROJECT_BINARY_DIR}/luabind-0.9)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${PROJECT_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${PROJECT_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${PROJECT_BINARY_DIR}/tinyxpath_1_3_1)
add_subdirectory (${SERVER_SOURCE_DIR}/pcl/common ${PROJECT_BINARY_DIR}/common)
add_subdirectory (${SERVER_SOURCE_DIR}/pcl/features ${PROJECT_BINARY_DIR}/features)
add_subdirectory (${SERVER_SOURCE_DIR}/Pinocchio ${PROJECT_BINARY_DIR}/Pinocchio)



if(NOT NPLRUNTIME_STATIC_LIB) 
	add_subdirectory (${SERVER_SOURCE_DIR}luasql ${PROJECT_BINARY_DIR}/luasql)
endif()

if(NPLRUNTIME_SUPPORT_FBX) 
	#add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/externals/assimp-4.0.0 ${PROJECT_BINARY_DIR}/assimp-4.0.0)
	add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/externals/assimp-5.0.1 ${PROJECT_BINARY_DIR}/assimp-5.0.1)
	# define ASSIMP in FBXParser.cpp to work with v5, the Y up issue is not fixed though in 5.2.4.  5.0.1 and 4.0.0 are fine. 
	#add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/externals/assimp-5.2.4 ${PROJECT_BINARY_DIR}/assimp-5.2.4)
endif()


# main NPL runtime using ParaEngineClient
add_subdirectory (${PROJECT_SOURCE_DIR}/../NPLRuntime/ParaEngineClient ${PROJECT_BINARY_DIR}/ParaEngineClient)

# expose the core include directory 
set (ParaEngineClient_SOURCE_DIR ${PROJECT_SOURCE_DIR}/trunk/ParaEngineClient)

if(NPLRUNTIME_PHYSICS)
	add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/externals/bullet3 ${PROJECT_BINARY_DIR}/bullet3)
	add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/PhysicsBT ${PROJECT_BINARY_DIR}/PhysicsBT)
endif()

if(NPLRUNTIME_CAUDIOENGINE)
	add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/cAudio_2.4.0 ${PROJECT_BINARY_DIR}/cAudio_2.4.0)
endif()	

if (NPLRUNTIME_WEBVIEW)
	add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/externals/webview ${PROJECT_BINARY_DIR}/webview)
endif()

if(PARAENGINE_CLIENT_DLL)
	if(NOT NPLRUNTIME_STATIC_LIB) 
		# this app is for testing the client dll
		add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/ParaEngineClientApp ${PROJECT_BINARY_DIR}/ParaEngineClientApp)
	endif()
endif()

if(WIN32)
	if (NPLRUNTIME_HAQI_LAUNCHER) 
		add_subdirectory (${PROJECT_SOURCE_DIR}/trunk/HaqiLauncher ${PROJECT_BINARY_DIR}/HaqiLauncher)
		add_subdirectory (${SERVER_SOURCE_DIR}/AutoUpdater ${PROJECT_BINARY_DIR}/AutoUpdater)
	endif()
endif()	

# following are plugins that should be build after main runtime	
if(NOT NPLRUNTIME_STATIC_LIB) 
	add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${PROJECT_BINARY_DIR}/NPLMono/NPLMono2)
endif()

# export these two variables to its parent scope just in case some app project reference NPLRuntime statically. 
set(NPLRUNTIME_LINK_LIBRARIES ${NPLRUNTIME_LINK_LIBRARIES} PARENT_SCOPE)
set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} PARENT_SCOPE)
