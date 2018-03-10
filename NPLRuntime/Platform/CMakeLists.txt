

if(ANDROID)
    add_subdirectory(Android)
elseif(WIN32)
    add_subdirectory(Windows)
elseif(APPLE)
    add_subdirectory(OSX)
endif()


