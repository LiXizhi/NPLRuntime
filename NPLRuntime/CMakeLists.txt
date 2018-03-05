cmake_minimum_required(VERSION 3.10)

set(NPLRUNTIME_PROJECT_NAME "NPLRuntime" CACHE STRING "Custom Project Name")


project(${NPLRUNTIME_PROJECT_NAME})
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)

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



# Ugly warnings
IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
ENDIF()

if(ANDROID)
    set(CMAKE_ANDROID_API 19)
    set(CMAKE_ANDROID_STL_TYPE "gnustl_shared")
    set(CMAKE_ANDROID_ARCH_ABI "armeabi-v7a")
    set(CMAKE_ANDROID_NATIVE_LIB_DIRECTORIES  ${CMAKE_BINARY_DIR}/lib/debug)
    set(CMAKE_ANDROID_ARM_NEON OFF)
    add_compile_options(-fexceptions -frtti -Wno-psabi)
    add_definitions(-D__ANDROID__ -DANDROID)
endif()

if(ANDROID) 
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)
elseif(WIN32)
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL)
endif()

set(NPLRUNTIME_LUA_VERSION "LuaJit2_0" CACHE STRING "LUA VERSION")
set_property(CACHE NPLRUNTIME_LUA_VERSION PROPERTY STRINGS LuaJit2_0 LuaJit2_1 Lua51)

add_subdirectory(externals)
add_subdirectory(Plugins)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)
#add_subdirectory(PhysicsBT)
#add_subdirectory(ParaEngineClientApp)

