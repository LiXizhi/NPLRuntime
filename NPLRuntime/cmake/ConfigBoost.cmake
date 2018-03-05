#Boost
################################
# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if ("$ENV{BOOST_ROOT}" STREQUAL "")
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/boost)
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/../../bin/boost)
	else()
		# message(WARNING "You can define a global environment variable of BOOST_ROOT that specify the boost root dir")
	endif()
else()
    set(BOOST_ROOT $ENV{BOOST_ROOT})
    message(STATUS "BOOST_ROOT env is at: ${BOOST_ROOT}")
    string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
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
set(Boost_USE_STATIC_RUNTIME OFF)

# Add more boost components here
find_package( Boost 1.55 REQUIRED COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams) 

#message(STATUS "BOOST_INCLUDE_DIR:" ${Boost_INCLUDE_DIR})
#message(STATUS "BOOST_LIBS:" ${Boost_LIBRARIES} )
# End of boost