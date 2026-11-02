# Author: hyz, Kimi
# Company: ParaEngine.com
# Date: 2026.11.2
# Desc: tiny native helper exe that waits for the game process to exit and then
#       starts the real launcher (ParaCraft.exe) to apply 430/431 updates.
#       It replaces the old PowerShell waiter script and needs no dependencies.

project (ParaApplyWaiter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match parent's runtime library setting (static linking)
IF(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	# statically link MSVC runtime to reduce dependencies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
	 		string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif()
	 	if(${flag_var} MATCHES "/MDd")
	 		string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		endif()
	endforeach(flag_var)
ENDIF()

add_executable(ParaApplyWaiter main.cpp)

# no console window
set_target_properties(ParaApplyWaiter PROPERTIES WIN32_EXECUTABLE true)

set_target_properties(ParaApplyWaiter PROPERTIES OUTPUT_NAME "ParaApplyWaiter")
set_target_properties(ParaApplyWaiter PROPERTIES DEBUG_OUTPUT_NAME "ParaApplyWaiter_d")

if(MSVC)
	ADD_CUSTOM_COMMAND(
		TARGET ParaApplyWaiter
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaApplyWaiter> ${OUTPUT_BIN_DIR}
	 )
endif(MSVC)
