# Title: iOS Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2021.10.26

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE ADDITIONAL_EXT mm m)

set(channel_id $ENV{channel_id})
message("current channel id is: ${channel_id}")

if (channel_id STREQUAL "papa")
    set(INFO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/papa_Info.plist)
    set(IMAGES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/papa_Images.xcassets)
elseif (channel_id STREQUAL "431")
    set(INFO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/edu_Info.plist)
    set(IMAGES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/edu_Images.xcassets)
else()
    set(INFO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
    set(IMAGES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Images.xcassets)
endif()

set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res)
set(ASSETS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(CMAKE_XCODE_BUILD_SYSTEM 12)
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Deployment target for iOS" FORCE)

find_library(UIKit UIKit)
find_library(OpenGLES OpenGLES)
find_library(QuartzCore QuartzCore)
find_library(CoreBluetooth CoreBluetooth)
find_library(WebKit WebKit)
find_library(Security Security)
find_library(CoreGraphics CoreGraphics)
find_library(ReplayKit ReplayKit)
find_library(AVKit AVKit)
find_library(Photos Photos)
find_library(AudioToolbox AudioToolbox)
find_library(AVFoundation AVFoundation)
find_library(CoreTelephony CoreTelephony)
find_library(AdSupport AdSupport)
find_library(AppTrackingTransparency AppTrackingTransparency)

add_executable(Paracraft
    ${app_src}
    ${IMAGES_FILE}
    ${RESOURCE_FILES}
    ${ASSETS_FILES}
)

target_link_libraries(Paracraft PUBLIC
    ${UIKit}
    ${OpenGLES}
    ${QuartzCore}
    ${CoreBluetooth}
    ${WebKit}
    ${Security}
    ${CoreGraphics}
    ${ReplayKit}
    ${AVKit}
    ${Photos}
    ${AudioToolbox}
    ${AVFoundation}
    ${CoreTelephony}
    ${AdSupport}
    ${AppTrackingTransparency}
    RenderSystemOpenGL
    ParaEngine
)

set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/res)
set_source_files_properties(${ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets)

set(RESOURCE_LIST ${IMAGES_FILE} ${RESOURCE_FILES} ${ASSETS_FILES})

if (channel_id STREQUAL "papa")
    set(development_team "5QLPT4C3EB")
    set(product_name "帕帕奇遇记")
elseif (channel_id STREQUAL "431")
    set(development_team "5QLPT4C3EB")
    set(product_name "帕拉卡智慧教育客户端")
else()
    set(development_team "3FHAD7P7A5")
    set(product_name "Paracraft")
endif()

set_target_properties(
    Paracraft PROPERTIES
    MACOSX_BUNDLE TRUE
    RESOURCE "${RESOURCE_LIST}"
    INSTALL_RPATH "@executable_path/Frameworks/"
    BUILD_WITH_INSTALL_RPATH TRUE
    XCODE_ATTRIBUTE_INFOPLIST_FILE ${INFO_FILE}
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME "LaunchImage"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${development_team}
    XCODE_ATTRIBUTE_PRODUCT_NAME ${product_name}
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "No"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/cAudioEngine/DependenciesSource/libogg-1.3.2/include)
target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/cAudioEngine/DependenciesSource/libvorbis-1.3.2/include)

if (NPLRUNTIME_OCE)
    add_definitions(-DNPLRUNTIME_OCE)
endif()

#set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")

# Framework generation option
option(BUILD_FRAMEWORK "Build as Framework" OFF)

if (BUILD_FRAMEWORK)
    # Create Framework target
    add_library(ParacraftFramework SHARED
        ${app_src}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ParacraftFramework.mm
    )
    
    target_link_libraries(ParacraftFramework PUBLIC
        ${UIKit}
        ${OpenGLES}
        ${QuartzCore}
        ${CoreBluetooth}
        ${WebKit}
        ${Security}
        ${CoreGraphics}
        ${ReplayKit}
        ${AVKit}
        ${Photos}
        ${AudioToolbox}
        ${AVFoundation}
        ${CoreTelephony}
        ${AdSupport}
        ${AppTrackingTransparency}
        RenderSystemOpenGL
        ParaEngine
    )
    
    # Set Framework properties
    set_target_properties(ParacraftFramework PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.tatfook.paracraft.framework
        MACOSX_FRAMEWORK_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Framework-Info.plist
        MACOSX_FRAMEWORK_BUNDLE_VERSION "2.0.22"
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING "62"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${development_team}
        XCODE_ATTRIBUTE_ENABLE_BITCODE "No"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        XCODE_ATTRIBUTE_SKIP_INSTALL "No"
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
        XCODE_ATTRIBUTE_MODULEMAP_FILE "${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap"
        XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
        PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/ParacraftFramework.h"
        VERSION ${CMAKE_PROJECT_VERSION}
        SOVERSION 1
    )
    
    # Include directories for Framework
    target_include_directories(ParacraftFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_include_directories(ParacraftFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/cAudioEngine/DependenciesSource/libogg-1.3.2/include)
    target_include_directories(ParacraftFramework PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/cAudioEngine/DependenciesSource/libvorbis-1.3.2/include)
    
    # Add Framework-specific compile definitions
    if (NPLRUNTIME_OCE)
        target_compile_definitions(ParacraftFramework PRIVATE NPLRUNTIME_OCE)
    endif()
    
    # Set Framework headers
    file(GLOB_RECURSE FRAMEWORK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
    set_target_properties(ParacraftFramework PROPERTIES
        PUBLIC_HEADER "${FRAMEWORK_HEADERS}"
    )
    
    # Copy resources to Framework
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/res)
    set_source_files_properties(${ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets)
    set_source_files_properties(${IMAGES_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap PROPERTIES MACOSX_PACKAGE_LOCATION Modules)
    
    target_sources(ParacraftFramework PRIVATE
        ${IMAGES_FILE}
        ${RESOURCE_FILES}
        ${ASSETS_FILES}
        ${CMAKE_CURRENT_SOURCE_DIR}/module.modulemap
    )
endif()

set(APP_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Paracraft.app")
set(DYLIB_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIGURATION>")
set(DEST_FRAMEWORK_PATH "${APP_PATH}/Frameworks/")
