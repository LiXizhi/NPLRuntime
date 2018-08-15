
option(GLAD_DEBUG "Use debug version glad" OFF)

if(GLAD_DEBUG)
add_subdirectory(core_debug)
else()
add_subdirectory(core)
endif()



if(WIN32 AND NOT ANDROID)
    add_subdirectory(ext/wgl)
endif()
