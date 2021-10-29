# Title: AndroidStudio Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2019.07.16
# ModifyDate: 2021.10.28

include(CheckSymbolExists)
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

project(Paracraft)

ucm_add_dirs(cpp TO app_src RECURSIVE)

# set(CMAKE_DEBUG_POSTFIX "")

add_library(Paracraft SHARED
    ${app_src}
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp)

target_link_libraries(Paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    android
    EGL 
    log
    cAudioEngine
)

target_compile_definitions(Paracraft PUBLIC 
    PARAENGINE_MOBILE
    PLATFORM_ANDROID
    STATIC_PLUGIN_CAUDIOENGINE
    AUTOUPDATE_PARAENGINE
	OPENAL_STATIC
)

check_symbol_exists(__swbuf, stdio.h, HAVE_SWBUF)
check_symbol_exists(__srget, stdio.h, HAVE_SRGET)

if (NOT HAVE_SWBUF)
	add_definitions(-DNOT_HAVE_SWBUF)
endif ()

if (NOT HAVE_SRGET)
    add_definitions(-DNOT_HAVE_SRGET)
endif ()
