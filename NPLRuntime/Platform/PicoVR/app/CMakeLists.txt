# Title: PicoVR Cmake Lists
# Author(s): big
# CreateDate: 2026.10.28
# Desc: same native app as AndroidStudio/app plus the OpenXR frame loop in cpp/.
# Built with -DANDROID_STUDIO=TRUE -DPICO_VR=ON from Platform/PicoVR/app/build.gradle.

include(CheckSymbolExists)
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

set(CMAKE_ANDROID_PAGE_SIZE 16384)

project(Paracraft)

set(ANDROID_APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../AndroidStudio/app)

ucm_add_dirs(${ANDROID_APP_DIR}/cpp TO app_src RECURSIVE)
ucm_add_dirs(cpp TO pico_src RECURSIVE)

# Khronos OpenXR loader, from the org.khronos.openxr:openxr_loader_for_android prefab package.
# gradle passes -DCMAKE_FIND_ROOT_PATH= (needed by boost), which drops the prefab root AGP adds, so look it up next to the build dir.
find_package(OpenXR REQUIRED CONFIG
    PATHS ${CMAKE_BINARY_DIR}/../prefab/${ANDROID_ABI}/prefab
    CMAKE_FIND_ROOT_PATH_BOTH
)

add_library(Paracraft SHARED
    ${app_src}
    ${pico_src}
)

target_include_directories(Paracraft PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp
    ${ANDROID_APP_DIR}/cpp
)

target_link_libraries(Paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    android
    EGL
    GLESv3
    log
    cAudioEngine
    OpenXR::openxr_loader
)

target_compile_definitions(Paracraft PUBLIC
    PARAENGINE_MOBILE
    PLATFORM_ANDROID
    STATIC_PLUGIN_CAUDIOENGINE
    AUTOUPDATE_PARAENGINE
    OPENAL_STATIC
    PICO_VR
    XR_USE_PLATFORM_ANDROID
    XR_USE_GRAPHICS_API_OPENGL_ES
)

check_symbol_exists(__swbuf, stdio.h, HAVE_SWBUF)
check_symbol_exists(__srget, stdio.h, HAVE_SRGET)

if (NOT HAVE_SWBUF)
	add_definitions(-DNOT_HAVE_SWBUF)
endif ()

if (NOT HAVE_SRGET)
    add_definitions(-DNOT_HAVE_SRGET)
endif ()
