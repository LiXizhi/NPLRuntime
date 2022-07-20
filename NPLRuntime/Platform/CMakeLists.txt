# Title: Platform Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2021.10.28

if (NPLRUNTIME_SERVER)
	add_subdirectory(Linux)
elseif (SDL)
	add_subdirectory(SDL)
elseif (ANDROID)
	if (ANDROID_STUDIO)
		add_subdirectory(AndroidStudio/app)
	else ()
		add_subdirectory(Android)
	endif ()
elseif (IOS)
    add_subdirectory(iOS)
elseif (WIN32)
    add_subdirectory(Windows)
elseif (MAC_SERVER)
	add_subdirectory(Linux)	
elseif (APPLE)
    add_subdirectory(OSX)
elseif (LINUX)
	add_subdirectory(SDL)
else ()
	#Linux
    add_subdirectory(Linux)
endif ()


