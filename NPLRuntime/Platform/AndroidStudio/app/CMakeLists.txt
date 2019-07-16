include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)

project(Paracraft)

ucm_add_dirs(cpp TO app_src RECURSIVE)

#SET(CMAKE_DEBUG_POSTFIX "")

add_library(Paracraft SHARED
    ${app_src}
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp)

target_link_libraries(Paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    native_app_glue
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

CHECK_SYMBOL_EXISTS(__swbuf, stdio.h, HAVE_SWBUF)
CHECK_SYMBOL_EXISTS(__srget, stdio.h, HAVE_SRGET)

if (NOT HAVE_SWBUF)
	ADD_DEFINITIONS(-DNOT_HAVE_SWBUF)
endif()

if (NOT HAVE_SRGET)
	ADD_DEFINITIONS(-DNOT_HAVE_SRGET)
endif()




