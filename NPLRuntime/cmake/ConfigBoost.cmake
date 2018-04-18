#Boost
################################



if(IOS)
    set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/src/boost_1_64_0) 
	set(BOOST_INCLUDEDIR ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/build/boost/.64.0/ios/prefix/include)
	set(BOOST_LIBRARYDIR ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/build/boost/1.64.0/ios/prefix/lib)
elseif(APPLE)
	set(BOOST_ROOT ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/src/boost_1_64_0)
	set(BOOST_INCLUDEDIR ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/build/boost/1.64.0/macos/prefix/include)
	set(BOOST_LIBRARYDIR ${PROJECT_SOURCE_DIR}/externals/boost/prebuild/apple/build/boost/1.64.0/macos/prefix/lib)
else()
	if ("$ENV{BOOST_ROOT}" STRGREATER "")
		set(BOOST_ROOT $ENV{BOOST_ROOT})
		string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
	else()
		set(BOOST_ROOT CACHE PATH "boost root path")
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

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)


# Add more boost components here. Boost 1.65.1 or above is recommended. 1.55 is minimum for server build
find_package(Boost 1.64.0 REQUIRED 
COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams log
) 

#message(STATUS "BOOST_INCLUDE_DIR:" ${Boost_INCLUDE_DIR})
#message(STATUS "BOOST_LIBS:" ${Boost_LIBRARIES} )
# End of boost