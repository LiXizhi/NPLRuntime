# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5
# Note: download sqlite src, and then use cmake to compile
# we will build as dll in win32 and static linked lib under linux. 
# Note: add #include "paraengine_sqlite3.h" at the very beginning of sqlite3.h and sqlite3.c to automatically define SQLITE_API according to ParaEngine usage macro. 

PROJECT(sqlite)

# source files for sqlite
SET(sqlite_SRCS
	sqlite3.c
	sqlite3.h
	sqlite3ext.h
	paraengine_sqlite3.h
)

IF(MSVC)
	# statically link MSVC to reduce dependancies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	 	if(${flag_var} MATCHES "/MDd")
	 		string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
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

if(NPL_STATIC_LINK_ALL)
	ADD_LIBRARY(sqlite STATIC ${sqlite_SRCS})
else()
	ADD_LIBRARY(sqlite SHARED ${sqlite_SRCS})

	ADD_CUSTOM_COMMAND(
	   TARGET sqlite
	   POST_BUILD
	   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:sqlite> ${OUTPUT_BIN_DIR}
	)
endif()



