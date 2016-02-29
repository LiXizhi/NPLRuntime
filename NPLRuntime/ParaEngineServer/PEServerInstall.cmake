# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.21

if(MSVC)

get_target_property(DEBUG_EXE_PATH ParaEngineServer DEBUG_LOCATION)
STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" DEBUG_EXE_NAME "${DEBUG_EXE_PATH}" )

get_target_property(RELEASE_EXE_PATH ParaEngineServer RELEASE_LOCATION)
STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" RELEASE_EXE_NAME "${RELEASE_EXE_PATH}" )

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( PARAWORLD_BIN_DIR "${NPLRuntime_SOURCE_DIR}/../ParaWorld/bin64/" )
else()
	set( PARAWORLD_BIN_DIR "${NPLRuntime_SOURCE_DIR}/../ParaWorld/" )
endif()

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineServer
   POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${DEBUG_EXE_PATH} ${PARAWORLD_BIN_DIR}${DEBUG_EXE_NAME}
	COMMAND ${CMAKE_COMMAND} -E copy ${RELEASE_EXE_PATH} ${PARAWORLD_BIN_DIR}${RELEASE_EXE_NAME}
)

endif(MSVC)