include(cmake/ucm.cmake)

if (ANDROID)
	cmake_minimum_required(VERSION 3.10)
else()
	cmake_minimum_required(VERSION 3.5)
endif()

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)


set(NPLRUNTIME_PROJECT_NAME "NPLRuntime" CACHE STRING "Custom Project Name")

option(PARAENGINE_CLIENT_DLL "Generate Dll Or Exe" OFF)
option(NPLRUNTIME_STATIC_LIB "static lib or not" OFF)
option(NPLRUNTIME_ICONV "include iconv" ON)

project(${NPLRUNTIME_PROJECT_NAME})

if(ANDROID) 
    set(CMAKE_ANDROID_API_MIN 19)
    set(CMAKE_ANDROID_API 19)
    set(CMAKE_ANDROID_STL_TYPE "gnustl_shared")
    set(CMAKE_ANDROID_ARCH_ABI "armeabi-v7a")
    set(CMAKE_ANDROID_NATIVE_LIB_DIRECTORIES  ${CMAKE_BINARY_DIR}/lib/debug)
    set(CMAKE_ANDROID_ARM_NEON OFF)
    add_compile_options(-fexceptions -frtti -Wno-psabi -fsigned-char)
    add_definitions(-D__ANDROID__ -DANDROID)
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)
elseif(IOS)
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL NULL)   
elseif(APPLE)
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL NULL)
elseif(WIN32)
    # Ugly warnings
    IF(MSVC)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
    ENDIF()
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL NULL)
else()
    set(NPLRUNTIME_RENDERER "NULL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL NULL)
endif()

set(MAC_SERVER APPLE AND ${NPLRUNTIME_RENDERER} STREQUAL "NULL")

# Win32 Default Option
if(WIN32)
	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
	option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
	option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)
	option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" ON)
	option(NPLRUNTIME_PHYSICS "include physics" ON)
elseif(IOS)
    option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
    option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
    option(NPLRUNTIME_PHYSICS "include physics" OFF)
    option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
    option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)	
elseif (MAC_SERVER)
	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" ON)
    option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
    option(NPLRUNTIME_PHYSICS "include physics" OFF)
    option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
    option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)

# OSX Default Option
elseif(APPLE)
	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" OFF)
	option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
	option(NPLRUNTIME_PHYSICS "include physics" OFF)
	option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
    option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)	
elseif (ANDROID)
	option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" ON)
    option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" ON)
    option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
    option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)
	option(NPLRUNTIME_PHYSICS "include physics" ON)
else()
    option(NPLRUNTIME_LUAJIT21 "build luajit21 support GC64" ON)
    option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
    option(NPLRUNTIME_PHYSICS "include physics" OFF)
    option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
    option(NPLRUNTIME_LUA51   "build lua5.1.5" OFF)
endif()

if(APPLE)
    # making executable relocatable 
    set(CMAKE_MACOSX_RPATH TRUE)
else()
    # making executable relocatable 
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    SET(CMAKE_BUILD "$ORIGIN/")
endif()

# Using static lib for other modules if main NPLRuntime is built as static lib. 
if(NPLRUNTIME_STATIC_LIB OR ANDROID)
	set(SQLITE_STATICLIB TRUE)
	set(PHYSICS_STATICLIB TRUE)
endif()

# This is the directory to keep all binary
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(OUTPUT_BIN_DIR   ${PROJECT_SOURCE_DIR}/../ParaWorld/bin64/)
else()
	set(OUTPUT_BIN_DIR   ${PROJECT_SOURCE_DIR}/../ParaWorld/bin32/)
endif()
 

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if (MSVC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MinSizeRel ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RelWithDebInfo ${CMAKE_BINARY_DIR}/bin)
endif (MSVC)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")



if(${NPLRUNTIME_RENDERER} STREQUAL "NULL")
	
else()
	option(PARAENGINE_CLIENT_DLL "Generate Dll Or Exe" OFF)

endif()



if(MSVC)
    ucm_set_runtime(STATIC)
endif()


add_subdirectory(externals)
add_subdirectory(Plugins)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)
#add_subdirectory(PhysicsBT)
#add_subdirectory(ParaEngineClientApp)

if(PARAENGINE_CLIENT_DLL)
	if(NOT NPLRUNTIME_STATIC_LIB) 
			# this app is for testing the client dll
		add_subdirectory (ParaEngineClientApp)
	endif()
endif()

