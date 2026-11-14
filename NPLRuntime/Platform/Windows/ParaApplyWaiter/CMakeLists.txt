project(ParaApplyWaiter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_executable(ParaApplyWaiter main.cpp version.rc)

set_target_properties(ParaApplyWaiter PROPERTIES
	WIN32_EXECUTABLE true
	OUTPUT_NAME "ParaApplyWaiter"
	DEBUG_OUTPUT_NAME "ParaApplyWaiter_d"
)

if(MSVC)
	ADD_CUSTOM_COMMAND(
		TARGET ParaApplyWaiter
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaApplyWaiter> ${OUTPUT_BIN_DIR}
	)
endif()
