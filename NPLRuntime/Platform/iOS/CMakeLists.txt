

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


ucm_add_dirs(src TO app_src RECURSIVE ADDITIONAL_EXT mm m)

set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/Images.xcassets)


find_library(UIKit UIKit)
find_library(OpenGLES OpenGLES)
find_library(QuartzCore QuartzCore)

add_executable(Paracraft ${app_src} ${RESOURCE_FILES})
target_link_libraries(Paracraft PUBLIC
    ${UIKit} ${OpenGLES} ${QuartzCore}
    RenderSystemOpenGL
    ParaEngine
)


set_target_properties(Paracraft PROPERTIES 
    MACOSX_BUNDLE YES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/info.plist
    RESOURCE "${RESOURCE_FILES}"
    INSTALL_RPATH "@executable_path/Frameworks/"
    BUILD_WITH_INSTALL_RPATH TRUE
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME "LaunchImage"
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")


set(APP_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Paracraft.app")
set(DYLIB_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIGURATION>")
set(DEST_FRAMEWORK_PATH "${APP_PATH}/Frameworks/")

add_custom_command(TARGET Paracraft POST_BUILD 
COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${APP_PATH}/assets" 
MAIN_DEPENDENCY Paracraft
)
