#Boost
################################



set(Boost_Version 1.69.0)

if ("$ENV{BOOST_ROOT}" STRGREATER "")
	set(BOOST_ROOT $ENV{BOOST_ROOT})
	string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
else()
	set(Boost_Path boost_1_69_0)
	set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/src/${Boost_Path})
endif()


if(IOS)
	if (IOS_PLATFORM STREQUAL "SIMULATOR")
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/iphonesim-build/stage/lib)
	elseif(IOS_PLATFORM STREQUAL "OS")
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/iphone-build/stage/lib)
	endif() 
elseif(APPLE)
	set(BOOST_LIBRARYDIR ${BOOST_ROOT}/macos-build/stage/lib)
elseif(WIN32 AND NOT ANDROID)
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/build_win32/stage/x64/lib)	
	else()
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/build_win32/stage/x86/lib)	
	endif()
elseif(ANDROID)
	set(BOOST_LIBRARYDIR ${BOOST_ROOT}/android-build/stage/${CMAKE_ANDROID_ARCH_ABI}/lib)
	
	set(Boost_INCLUDE_DIR ${BOOST_ROOT})
	set(Boost_LIBRARY_DIR ${BOOST_LIBRARYDIR})
else()
	message(STATUS "BOOST_ROOT  is at: ${BOOST_ROOT}")
endif()


set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
#set(Boost_DEBUG ON)

if (MSVC)
	set(Boost_USE_STATIC_RUNTIME ON)
else()
	set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# Add more boost components here. Boost 1.65.1 or above is recommended. 1.55 is minimum for server build

if(IOS)
find_host_package(Boost 1.69.0 REQUIRED 
COMPONENTS thread date_time filesystem system chrono regex serialization iostreams log
)
else()
find_package(Boost 1.65.0 REQUIRED 
COMPONENTS thread date_time filesystem system chrono regex serialization iostreams log
) 
endif()



#message(STATUS "BOOST_INCLUDE_DIR:" ${Boost_INCLUDE_DIR})
#message(STATUS "BOOST_LIBS:" ${Boost_LIBRARIES} )
# End of boost
