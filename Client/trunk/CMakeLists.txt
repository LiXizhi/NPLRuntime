cmake_minimum_required(VERSION 3.10)

project(Client)
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
    set(CMAKE_ANDROID_STL_TYPE "gnustl_shared")
    add_compile_options(-fexceptions -frtti -Wno-psabi)
    add_definitions(-D__ANDROID__ -DANDROID -D__ARM_NEON__)
endif()


add_subdirectory(externals)
add_subdirectory(Plugins)
add_subdirectory(ParaEngineClient)
#add_subdirectory(PhysicsBT)
#add_subdirectory(ParaEngineClientApp)

