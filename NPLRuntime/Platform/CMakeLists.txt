

if(ANDROID)
    add_subdirectory(Android)
else(IOS)
    add_subdirectory(iOS)
elseif(WIN32)
    add_subdirectory(Windows)
elseif(APPLE)
    add_subdirectory(OSX)
endif()


