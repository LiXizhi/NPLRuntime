include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

add_executable(ParaEngineServer
    ${app_src}
)

target_include_directories(ParaEngineServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ParaEngineServer PUBLIC
    RenderSystemNull
    ParaEngine
)
target_compile_definitions(ParaEngineServer PUBLIC 
	PARAENGINE_SERVER
)

ADD_CUSTOM_COMMAND(
	TARGET ParaEngineServer
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineServer> ${OUTPUT_BIN_DIR}
)