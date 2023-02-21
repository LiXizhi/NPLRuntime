include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

if (EMSCRIPTEN)
	ucm_add_dirs(emscripten TO app_main_src RECURSIVE)
else()
	ucm_add_dirs(sdl TO app_main_src RECURSIVE)
endif()

#set(VCPKG_TARGET_TRIPLET x64-windows-static)
#set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

if (EMSCRIPTEN)
	set (TARGET_NAME ParaCraftEmscripten)
else()
	set (TARGET_NAME ParaCraftSDL2)
endif()
add_executable(${TARGET_NAME} ${app_src} ${app_main_src})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${TARGET_NAME} PUBLIC RenderSystemOpenGL ParaEngine)

if (SDL AND NOT EMSCRIPTEN) 
	message("SDL_ROOT:" ${SDL_ROOT})
	target_include_directories(${TARGET_NAME} PUBLIC "${SDL_ROOT}/include")
	target_link_libraries(${TARGET_NAME} PUBLIC ${SDL_ROOT}/lib/SDL2-staticd.lib ${SDL_ROOT}/lib/SDL2maind.lib ws2_32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib dinput8.lib OpenGL32.lib)
elseif (EMSCRIPTEN)
	# -sTOTAL_STACK=32MB 
	if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
		set(APP_ROOT /mnt/d/workspace/npl/paracraft_root@/)
	else()
		set(APP_ROOT D:/workspace/npl/paracraft_root@/)
	endif()
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -g --profiling-funcs -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=512MB -sALLOW_MEMORY_GROWTH -sPTHREAD_POOL_SIZE=32 -sSTACK_SIZE=32000000  -pthread --embed-file ${APP_ROOT}")
	set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Og -g --profiling-funcs -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=512MB -sALLOW_MEMORY_GROWTH -sPTHREAD_POOL_SIZE=32 -sSTACK_SIZE=32000000  -pthread --embed-file ${APP_ROOT}")
	# -sSTACK_SIZE=32000000 
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	target_compile_definitions(${TARGET_NAME} PUBLIC EMSCRIPTEN)
endif()

target_compile_definitions(${TARGET_NAME} PUBLIC PLATFORM_LINUX)
target_compile_definitions(${TARGET_NAME} PUBLIC LINUX)
target_compile_definitions(${TARGET_NAME} PUBLIC PLATFORM_SDL2)

ADD_CUSTOM_COMMAND(
	TARGET ${TARGET_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${OUTPUT_BIN_DIR}
)
