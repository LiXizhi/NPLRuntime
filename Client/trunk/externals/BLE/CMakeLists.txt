cmake_minimum_required(VERSION 3.16.0)

################################
# BLE dll for paraengine
################################
set(CMAKE_SYSTEM_NAME "WindowsStore")
set(CMAKE_SYSTEM_VERSION "10.0")

project(BLE)

set(BoostRoot_DIR "$ENV{BOOST_ROOT}")

####################################################
# ParaEngine related 
# ParaEngineClient_SOURCE_DIR can be used to specify a single base
# folder where the required dependencies may be found.
set(ParaEngineClient_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaEngineClient)
include_directories("${ParaEngineClient_SOURCE_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/Core")
include_directories("${ParaEngineClient_SOURCE_DIR}/NPL")
include_directories("${ParaEngineClient_SOURCE_DIR}/math")
include_directories("${ParaEngineClient_SOURCE_DIR}/util")

set(LibName "ParaBLE")
set(CMAKE_DEBUG_POSTFIX "_d")

add_library(${LibName} SHARED main.cpp)

target_include_directories(${LibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${LibName} PUBLIC ${BoostRoot_DIR})

set_target_properties(${LibName} PROPERTIES
    VS_WINRT_COMPONENT TRUE
    WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

