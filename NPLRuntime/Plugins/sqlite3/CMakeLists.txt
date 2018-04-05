# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5
# Note: download sqlite src, and then use cmake to compile
# we will build as dll in win32 and static linked lib under linux. 
# Note: add #include "paraengine_sqlite3.h" at the very beginning of sqlite3.h and sqlite3.c to automatically define SQLITE_API according to ParaEngine usage macro. 
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
PROJECT(sqlite)


# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")
# Note: include the ParaEngine/NPLRuntime include directory
set(NPLRUNTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaEngine)

include_directories("${NPLRUNTIME_ROOT}/Framework/Interface/")
include_directories("${NPLRUNTIME_ROOT}/Core/")
include_directories("${NPLRUNTIME_ROOT}/Engine/")
include_directories("${NPLRUNTIME_ROOT}/")

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
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MDd")
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

if(SQLITE_STATICLIB)
	SET(SQLITE_LIB_MODE "STATIC")
	ADD_DEFINITIONS(-DSQLITE_STATICLIB)
else()
	SET(SQLITE_LIB_MODE "SHARED")
endif()

add_library(sqlite ${SQLITE_LIB_MODE} ${sqlite_SRCS})
target_include_directories(sqlite PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(sqlite ${Boost_LIBRARIES}) 


# set(TEST_DIR "D:/code/npl/TableDB/")

if(SQLITE_STATICLIB)
else()
	ADD_CUSTOM_COMMAND(
		TARGET sqlite
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sqlite> ${OUTPUT_BIN_DIR}
		# COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sqlite> ${TEST_DIR}
	)
endif()
