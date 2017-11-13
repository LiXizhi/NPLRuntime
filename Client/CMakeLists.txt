# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 2.6)

project (CLIENT)

# build static library instead of dynamic ones
option(PARAENGINE_CLIENT_DLL "Generate Dll Or Exe" OFF)
option(NPLRUNTIME_STATIC_LIB "static lib or not" OFF)
option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" ON)
option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
option(NPLRUNTIME_LUA51   "build lua5.1.5 with coco" ON)
option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" ON)

if (WIN32)
	option(NPLRUNTIME_PHYSICS "include physics" ON)
	option(NPLRUNTIME_DIRECTX_RENDERER "use openGL or directX renderer" ON)
else()
	option(NPLRUNTIME_PHYSICS "include physics" OFF)
	option(NPLRUNTIME_DIRECTX_RENDERER "use openGL or directX renderer" OFF)
endif()

# using static lib for other modules if main NPLRuntime is built as static lib. 
if(NPLRUNTIME_STATIC_LIB)
	set(CURL_STATICLIB TRUE)
	set(LUA_STATICLIB TRUE)
	set(SQLITE_STATICLIB TRUE)
endif()


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
endif(PARAENGINE_CLIENT OR WIN32)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)

# Add more boost components here
find_package( Boost 1.61 REQUIRED COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams) 


if (WIN32)
	option(BUILD_CURL_EXE OFF)
	option(BUILD_CURL_TESTS OFF)
    add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.11 ${CLIENT_BINARY_DIR}/zlib-1.2.11)
	set(ZLIB_FOUND 1)
	set(ZLIB_LIBRARY zlib)
	set(ZLIB_LIBRARIES zlib)
	set(ZLIB_INCLUDE_DIR ${SERVER_SOURCE_DIR}zlib-1.2.11 ${CLIENT_BINARY_DIR}/zlib-1.2.11)
	set(ZLIB_SOURCE_DIR ${ZLIB_INCLUDE_DIR})
  
	add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${CLIENT_BINARY_DIR}/curl-7.47.1)

	if(NOT NPLRUNTIME_DIRECTX_RENDERER)
		option(GLFW_BUILD_DOCS OFF)
		option(GLFW_BUILD_EXAMPLES OFF)
		option(GLFW_BUILD_TESTS OFF)
		option(GLFW_INSTALL OFF)

		
		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glfw-3.2.1 ${CLIENT_BINARY_DIR}/glfw-3.2.1)

		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${CLIENT_BINARY_DIR}/lpng1634)
		set(PNG_FOUND 1)
		set(PNG_LIBRARY libpng)
		set(PNG_LIBRARIES libpng)
		set(PNG_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${CLIENT_BINARY_DIR}/lpng1634)
		set(PNG_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${CLIENT_BINARY_DIR}/lpng1634)
		

		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1 ${CLIENT_BINARY_DIR}/freetype-2.8.1)
		set(FREETYPE_LIBRARY freetype)
		set(FREETYPE_LIBRARIES freetype)
		set(FREETYPE_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/include ${CLIENT_BINARY_DIR}/freetype-2.8.1/include)
		set(FREETYPE_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/src ${CLIENT_BINARY_DIR}/freetype-2.8.1/src)


		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/build/cmake ${CLIENT_BINARY_DIR}/glew-2.1.0)
		set(GLEW_LIBRARY glew)
		set(GLEW_LIBRARIES glew)
		set(GLEW_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/include ${CLIENT_BINARY_DIR}/glew-2.1.0/include)
		set(GLEW_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/src ${CLIENT_BINARY_DIR}/glew-2.1.0/src)
		
	endif()
else()
	FIND_PACKAGE(CURL REQUIRED)
	# making executable relocatable 
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "$ORIGIN/") 
endif()

if (NPLRUNTIME_LUAJIT21) 
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT-2.1 ${CLIENT_BINARY_DIR}/LuaJIT-2.1)
endif()
if (NPLRUNTIME_LUA51)
	add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${CLIENT_BINARY_DIR}/lua-5.1.4)
endif()
if(NPLRUNTIME_LUAJIT20)
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${CLIENT_BINARY_DIR}/LuaJIT)
endif()

add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${CLIENT_BINARY_DIR}/luabind-0.9)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${CLIENT_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${CLIENT_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${CLIENT_BINARY_DIR}/tinyxpath_1_3_1)



if(NPLRUNTIME_STATIC_LIB) 
	# add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/png ${CLIENT_BINARY_DIR}/png)
else()
	add_subdirectory (${SERVER_SOURCE_DIR}luasql ${CLIENT_BINARY_DIR}/luasql)
endif()

if(NPLRUNTIME_SUPPORT_FBX) 
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/assimp-4.0.0 ${CLIENT_BINARY_DIR}/assimp-4.0.0)
endif()

# main NPL runtime using ParaEngineClient
add_subdirectory (${CLIENT_SOURCE_DIR}/../NPLRuntime/ParaEngineClient ${CLIENT_BINARY_DIR}/ParaEngineClient)

# expose the core include directory 
set (ParaEngineClient_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient)

if(NPLRUNTIME_PHYSICS)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/bullet3 ${CLIENT_BINARY_DIR}/bullet3)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/PhysicsBT ${CLIENT_BINARY_DIR}/PhysicsBT)
endif()
	
if(PARAENGINE_CLIENT_DLL)
	if(NPLRUNTIME_STATIC_LIB) 
	else()
		# this app is for testing the client dll
		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/ParaEngineClientApp ${CLIENT_BINARY_DIR}/ParaEngineClientApp)
	endif()
endif()

	
# following are plugins that should be build after main runtime	
if(NPLRUNTIME_STATIC_LIB) 
else()	
	add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${CLIENT_BINARY_DIR}/NPLMono/NPLMono2)
endif()

# export these two variables to its parent scope just in case some app project reference NPLRuntime statically. 
set(NPLRUNTIME_LINK_LIBRARIES ${NPLRUNTIME_LINK_LIBRARIES} PARENT_SCOPE)
set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} PARENT_SCOPE)


# dxEffects2glEffects
if(NOT NPLRUNTIME_DIRECTX_RENDERER)
	add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/dxEffects2glEffects)
endif()

# glEffectsParser
if(NOT NPLRUNTIME_DIRECTX_RENDERER)
	add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/glEffects)
endif()