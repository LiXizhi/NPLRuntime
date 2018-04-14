#Boost
################################



if(IOS)
	set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/src/boost_1_64_0)
	if (IOS_PLATFORM STREQUAL "SIMULATOR")
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/iphonesim-build/stage/lib)
	elseif(IOS_PLATFORM STREQUAL "OS")
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/iphone-build/stage/lib)
	endif() 

elseif(APPLE)
	set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/src/boost_1_64_0)
	set(BOOST_LIBRARYDIR ${BOOST_ROOT}/macos-build/stage/lib)
else()
	if ("$ENV{BOOST_ROOT}" STRGREATER "")
		set(BOOST_ROOT $ENV{BOOST_ROOT})
		string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
	else()
		set(BOOST_ROOT CACHE PATH "boost root path")
	endif()
	if(BOOST_ROOT STREQUAL "")
		message(FATAL_ERROR "please set boost root path")
	endif()
	message(STATUS "BOOST_ROOT  is at: ${BOOST_ROOT}")
endif()


if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib64)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)	
	endif()
else()
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib32)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
endif()

if(ANDROID)
	set(BOOST_LIBRARYDIR ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/android)
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_Debug ON)

if (MSVC)
	set(Boost_USE_STATIC_RUNTIME ON)
else()
	set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# Add more boost components here. Boost 1.65.1 or above is recommended. 1.55 is minimum for server build

if(IOS)
find_host_package(Boost 1.64.0 REQUIRED 
COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams log
)
else()
find_package(Boost 1.64.0 REQUIRED 
COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams log
) 
endif()



#message(STATUS "BOOST_INCLUDE_DIR:" ${Boost_INCLUDE_DIR})
#message(STATUS "BOOST_LIBS:" ${Boost_LIBRARIES} )
# End of boost