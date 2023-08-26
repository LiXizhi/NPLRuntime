# Title: iOS Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2021.10.26

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE ADDITIONAL_EXT mm m)

set(INFO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
set(IMAGES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Images.xcassets)
set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res)
set(ASSETS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(CMAKE_XCODE_BUILD_SYSTEM 12)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Deployment target for iOS" FORCE)

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
    RenderSystemOpenGL
    ParaEngine
)

set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/res)
set_source_files_properties(${IMAGES_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/Images.cassets)
set_source_files_properties(${ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets)

set(RESOURCE_LIST ${IMAGES_FILE} ${RESOURCE_FILES} ${ASSETS_FILES})

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
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "No"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

#set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")

set(APP_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Paracraft.app")
set(DYLIB_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIGURATION>")
set(DEST_FRAMEWORK_PATH "${APP_PATH}/Frameworks/")
