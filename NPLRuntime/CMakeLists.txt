cmake_minimum_required(VERSION 3.10)

project(NPLRuntime)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)



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



add_subdirectory(externals)
add_subdirectory(Plugins)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)
#add_subdirectory(PhysicsBT)
#add_subdirectory(ParaEngineClientApp)

