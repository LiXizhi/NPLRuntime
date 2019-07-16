
if (NPLRUNTIME_SERVER)
	add_subdirectory(Linux)
elseif(ANDROID)
	if (ANDROID_STUDIO)
		add_subdirectory(AndroidStudio/app)
	else()
		add_subdirectory(Android)
	endif()
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


