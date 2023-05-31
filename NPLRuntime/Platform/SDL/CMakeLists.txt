include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO app_src RECURSIVE)

if (EMSCRIPTEN)
	ucm_add_dirs(emscripten TO app_main_src RECURSIVE)
else()
	ucm_add_dirs(sdl TO app_main_src RECURSIVE)
endif()

if (EMSCRIPTEN)
	set (TARGET_NAME ParaCraft)
else()
	set (TARGET_NAME ParaCraftSDL2)
endif()

add_executable(${TARGET_NAME} ${app_src} ${app_main_src})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${TARGET_NAME} PUBLIC RenderSystemOpenGL ParaEngine)

if (SDL AND NOT EMSCRIPTEN) 
	message("SDL_ROOT:" ${SDL_ROOT})
	target_include_directories(${TARGET_NAME} PUBLIC "${SDL_ROOT}/include")
	target_link_libraries(${TARGET_NAME} PUBLIC ${SDL_ROOT}/lib/SDL2-static.lib ${SDL_ROOT}/lib/SDL2main.lib ws2_32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib dinput8.lib OpenGL32.lib)
elseif (EMSCRIPTEN)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 --use-preload-cache -sFULL_ES2=1 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=256MB -sALLOW_MEMORY_GROWTH -sPTHREAD_POOL_SIZE=16 -sSTACK_SIZE=16000000 -sASYNCIFY_STACK_SIZE=16000000 -lopenal -sFETCH -lidbfs.js -lwebsocket.js -pthread -sFORCE_FILESYSTEM -sASYNCIFY")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 --use-preload-cache -sFULL_ES2=1 -sMAX_WEBGL_VERSION=2 -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=256MB -sALLOW_MEMORY_GROWTH -sPTHREAD_POOL_SIZE=16 -sSTACK_SIZE=16000000 -sASYNCIFY_STACK_SIZE=16000000 -lopenal -sFETCH -lidbfs.js -lwebsocket.js -pthread -sFORCE_FILESYSTEM -sASYNCIFY")
	# -sSTACK_SIZE=32000000 
	target_compile_definitions(${TARGET_NAME} PUBLIC EMSCRIPTEN)
	target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/emscripten)
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

if (EMSCRIPTEN)
	# 程序根目录 文件系统
	if (NOT APP_ROOT)
		if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
			set(APP_ROOT /mnt/d/workspace/npl/paracraft_root)
		else()
			set(APP_ROOT D:/workspace/npl/paracraft_root)
		endif()
	endif()

	# 指定HTML根目录 也是输出目录
	if (NOT HTML_ROOT)
		set(HTML_ROOT /mnt/d/workspace/npl/webparacraft/)
	endif()

	# 拷贝js到html根目录
	ADD_CUSTOM_COMMAND(
		TARGET ${TARGET_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/bin ${HTML_ROOT}
	)

	# 拷贝文件系统到根目录
	add_custom_target(
		paracraft_asset ALL
		COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset.data --preload "${APP_ROOT}@/" --js-output=paracraft_asset.js
		COMMENT "打包资源文件"
		OUTPUT_DIRECTORY ${HTML_ROOT}
		WORKING_DIRECTORY ${HTML_ROOT}
	)

	# script
	add_custom_target(
		paracraft_script
		COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_script.data --preload "/mnt/d/workspace/npl/paracraft_script/Mod@/Mod" --js-output=paracraft_script.js
		COMMENT "打包资源文件"
		OUTPUT_DIRECTORY ${HTML_ROOT}
		WORKING_DIRECTORY ${HTML_ROOT}
	)

	# 拷贝缓存文件系统到根目录
	add_custom_target(
		paracraft_asset_temp
		COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset_temp.data --preload "${APP_ROOT}_temp@/" --js-output=paracraft_asset_temp.js
		COMMENT "打包缓存资源文件"
		OUTPUT_DIRECTORY ${HTML_ROOT}
		WORKING_DIRECTORY ${HTML_ROOT}
	)
endif()

# emcmake cmake -S NPLRuntime -B build_emscripten_release -DEMSCRIPTEN=ON -DCMAKE_BUILD_TYPE=RELEASE -DLOCAL_DEBUG=TRUE
# emcmake cmake -S NPLRuntime -B build_emscripten -DEMSCRIPTEN=ON -DLOCAL_DEBUG=TRUE