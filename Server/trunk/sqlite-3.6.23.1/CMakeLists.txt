# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5
# Note: download sqlite src, and then use cmake to compile
# we will build as dll in win32 and static linked lib under linux. 
# Note: add #include "paraengine_sqlite3.h" at the very beginning of sqlite3.h and sqlite3.c to automatically define SQLITE_API according to ParaEngine usage macro. 

PROJECT(sqlite)


# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")
# Note: include the ParaEngine/NPLRuntime include directory
if ("$ENV{NPLRUNTIME_ROOT}" STREQUAL "")
	set(NPLRUNTIME_ROOT ${PROJECT_SOURCE_DIR}/../../../Client/trunk/ParaEngineClient/)
else()
	set(NPLRUNTIME_ROOT $ENV{NPLRUNTIME_ROOT}/Client/trunk/ParaEngineClient)
endif()
include_directories("${NPLRUNTIME_ROOT}/Core")

# include boost if client is not defined, it allows us to use a different boost version than the client on the server build.
if (PARAENGINE_CLIENT OR WIN32)
	if ("$ENV{BOOST_ROOT}" STREQUAL "")
		set(BOOST_ROOT ${SERVER_SOURCE_DIR}/boost_1_61_0)
	endif()
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)
	else()
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
endif(PARAENGINE_CLIENT OR WIN32)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_STATIC_RUNTIME ON)

# Add more boost components here
find_package( Boost 1.55 REQUIRED COMPONENTS thread date_time filesystem system chrono signals regex serialization iostreams) 

# source files for sqlite
SET(sqlite_SRCS
	sqlite.cpp
	# sqlite3.def
	sqlite3.c
	sqlite3.h
	sqlite3ext.h
	paraengine_sqlite3.h
)

include_directories("${Boost_INCLUDE_DIRS}")

IF(MSVC)
	# statically link MSVC to reduce dependancies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	 	if(${flag_var} MATCHES "/MDd")
			string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		  add_definitions(-DSQLITE_DEBUG)
			add_definitions(-DSQLITE_TEST)
			add_definitions(-DSQLITE_ENABLE_IOTRACE)
		endif(${flag_var} MATCHES "/MDd")
	endforeach(flag_var)
	

	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

# the following will build it as dll under win32, since it will make the core game engine update more granular. 
IF(WIN32)
	SET(CMAKE_DEBUG_POSTFIX "_d")
	
	# To turn size optimisations on again
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /opt:ref")
ELSE(WIN32)
	
ENDIF(WIN32)

ADD_LIBRARY(sqlite SHARED ${sqlite_SRCS})


# set(TEST_DIR "D:/code/npl/TableDB/")

ADD_CUSTOM_COMMAND(
	TARGET sqlite
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sqlite> ${OUTPUT_BIN_DIR}
	# COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sqlite> ${TEST_DIR}
)


