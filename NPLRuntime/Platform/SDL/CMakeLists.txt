include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

add_executable(ParaCraft ${app_src})

target_include_directories(ParaCraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ParaCraft PUBLIC RenderSystemOpenGL ParaEngine)

target_compile_definitions(ParaCraft PUBLIC PLATFORM_LINUX)
target_compile_definitions(ParaCraft PUBLIC LINUX)

ADD_CUSTOM_COMMAND(
	TARGET ParaCraft
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaCraft> ${OUTPUT_BIN_DIR}
)
