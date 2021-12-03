# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.7.3
# Note: This is a ParaEngine plugin (dll) that does the job of database proxy and NPL message router (according to msg.nid)

PROJECT(NPLRouter)

# source files for NPLRouter
SET(NPLRouter_SRCS
	# Platforms.h
	RouterClassDesc.cpp
	NPLRouter.cpp	
	NPLRouter.h
	NPLRouterStrategy.cpp
	NPLRouterStrategy.h
	RouterMsgSend.h
	RouterMsgSend.cpp
	NPLRouter.def
)

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${ParaEngineServer_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")

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

if (WIN32)
  set(LINK_LIBRARIES
	${Boost_LIBRARIES}
  )
else (WIN32)
  set(LINK_LIBRARIES
  		dl
  )
endif (WIN32)

link_libraries( ${LINK_LIBRARIES})
ADD_LIBRARY(NPLRouter SHARED ${NPLRouter_SRCS})

if(MSVC)
ADD_CUSTOM_COMMAND(
   TARGET NPLRouter
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Release/NPLRouter.dll ${PROJECT_SOURCE_DIR}/../../../ParaWorld/NPLRouter.dll
)
endif(MSVC)

