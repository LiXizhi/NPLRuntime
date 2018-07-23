add_subdirectory(core_debug)
if(WIN32 AND NOT ANDROID)
    add_subdirectory(ext/wgl)
endif()
