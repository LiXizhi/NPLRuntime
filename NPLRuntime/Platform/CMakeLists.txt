

if(ANDROID)
    add_subdirectory(Android)
elseif(IOS)
    add_subdirectory(iOS)
elseif(WIN32)
    add_subdirectory(Windows)
elseif(MAC_SERVER)
	add_subdirectory(Linux)	
elseif(APPLE)
    add_subdirectory(OSX)
else() #Linux
    add_subdirectory(Linux)
endif()


