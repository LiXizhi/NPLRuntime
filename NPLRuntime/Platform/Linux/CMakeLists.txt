include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

add_executable(ParaEngineServer
    ${app_src}
)

target_include_directories(ParaEngineServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(EXTRA_LIBRARIES )

IF (MAC_SERVER)
	target_compile_definitions(ParaEngineServer PUBLIC PLATFORM_MAC)
	
	## this is for file monitor API in MAC OSX
	set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} "-framework CoreFoundation" "-framework CoreServices")
	# -Os to optimize for size. or -O3 to turn on all optimizations.
	# no -s option since it has the same effect as gcc -s if not a single -g is provided
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
ENDIF()

target_link_libraries(ParaEngineServer PUBLIC
    RenderSystemRHICore
    RenderSystemNull
    ParaEngine
	${EXTRA_LIBRARIES}
)

if (WIN32)
	target_compile_definitions(ParaEngineServer PRIVATE
		PLATFORM_WINDOWS
		USE_WINDOWS_SSPI
		WIN32_LEAN_AND_MEAN
		_WIN32_WINNT=0x0A00
		_WINSOCK_DEPRECATED_NO_WARNINGS
	)
	target_link_libraries(ParaEngineServer PRIVATE
		gdiplus.lib
		comctl32.lib
		Dbghelp.lib
		VFW32.lib
		SHLWAPI.lib
		Psapi.lib
		Ws2_32
		winmm
		wsock32
		Dnsapi
		Secur32
		Crypt32
		Wldap32
	)
elseif (UNIX AND NOT APPLE)
	target_link_libraries(ParaEngineServer PRIVATE
		dl
	)
endif ()

target_compile_definitions(ParaEngineServer PUBLIC 
	PARAENGINE_SERVER
	PARA_RHI_MULTI_BACKEND
)



ADD_CUSTOM_COMMAND(
	TARGET ParaEngineServer
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineServer> ${OUTPUT_BIN_DIR}
)