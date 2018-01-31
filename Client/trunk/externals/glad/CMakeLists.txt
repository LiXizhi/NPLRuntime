add_subdirectory(core)
if(WIN32 AND NOT ANDROID)
    add_subdirectory(ext/wgl)
endif()
