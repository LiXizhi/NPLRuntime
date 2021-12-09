# Author: gosling
# Company: ParaEngine.com
# Date: 2009.9.18
# Note: This is a ParaEngine plugin (dll) that does the job of interfacing TaoMee

PROJECT(TMInterface)

# source files for TMInterface
SET(TMInterface_SRCS
	TMInterface.cpp	
	TMInterface.h	
	TMService.cpp
	TMService.hpp
	queue.cpp
	queue.hpp
	connection.cpp
	connection.hpp
	MsgTM.cpp
	MsgTM.hpp
	MD5.cpp
	MD5.h
	# Platforms.h
	pdumanip.hpp
	byteswap.hpp
	ConnectionManager.h
	ConnectionManager.cpp
	TMInterface.def
)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${ParaEngineServer_SOURCE_DIR}")
set (CLIENT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../Client/trunk/ParaEngineClient)
include_directories("${CLIENT_SOURCE_DIR}/Core")
include_directories("${CLIENT_SOURCE_DIR}/util")

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
	
	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref")
ENDIF()

if(WIN32)
link_libraries( ${LINK_LIBRARIES} 
	${Boost_LIBRARIES} 
	Ws2_32.lib
	Rpcrt4.lib
)
else(WIN32)
link_libraries( ${LINK_LIBRARIES} 
	dl
)
endif(WIN32)

#ADD_EXECUTABLE(TMInterface ${TMInterface_SRCS})

ADD_LIBRARY(TMInterface SHARED ${TMInterface_SRCS})

ADD_CUSTOM_COMMAND(TARGET TMInterface POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:TMInterface> ${OUTPUT_BIN_DIR}
)