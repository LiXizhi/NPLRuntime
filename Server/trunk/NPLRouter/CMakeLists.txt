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
set (CLIENT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../../Client/trunk/ParaEngineClient)
include_directories("${CLIENT_SOURCE_DIR}/Core")
include_directories("${CLIENT_SOURCE_DIR}/util")
include_directories("${Boost_INCLUDE_DIRS}")

IF(MSVC)
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	
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

ADD_CUSTOM_COMMAND(TARGET NPLRouter POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:NPLRouter> ${OUTPUT_BIN_DIR}
)

