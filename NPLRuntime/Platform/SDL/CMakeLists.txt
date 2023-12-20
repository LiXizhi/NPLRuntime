include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
if (EMSCRIPTEN)
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
else()
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_C_STANDARD 11)
endif()

ucm_add_dirs(src TO app_src RECURSIVE)

if (EMSCRIPTEN)
	ucm_add_dirs(emscripten TO app_main_src RECURSIVE)
else()
	ucm_add_dirs(sdl TO app_main_src RECURSIVE)
endif()

if (EMSCRIPTEN)
	if (EMSCRIPTEN_SINGLE_THREAD)
		set (TARGET_NAME ParaCraftSingleThread)
	else()
		set (TARGET_NAME ParaCraft)
	endif()
else()
	set (TARGET_NAME ParaCraftSDL2)
endif()

add_executable(${TARGET_NAME} ${app_src} ${app_main_src})

target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${TARGET_NAME} PUBLIC RenderSystemOpenGL ParaEngine)

if (NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
endif()

if (SDL AND NOT EMSCRIPTEN) 
	message("SDL_ROOT:" ${SDL_ROOT})
	target_include_directories(${TARGET_NAME} PUBLIC "${SDL_ROOT}/include")
	target_link_libraries(${TARGET_NAME} PUBLIC ${SDL_ROOT}/lib/SDL2-static.lib ${SDL_ROOT}/lib/SDL2main.lib ws2_32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib uuid.lib advapi32.lib setupapi.lib shell32.lib dinput8.lib OpenGL32.lib)
elseif (EMSCRIPTEN)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 --use-preload-cache -sFULL_ES2=1 -sMAX_WEBGL_VERSION=2 -lembind -lopenal -sFETCH -lidbfs.js -lwebsocket.js -sFORCE_FILESYSTEM -sASYNCIFY -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=256MB -sSTACK_SIZE=16000000 -sASYNCIFY_STACK_SIZE=16000000")
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 --use-preload-cache -sFULL_ES2=1 -sMAX_WEBGL_VERSION=2 -lembind -lopenal -sFETCH -lidbfs.js -lwebsocket.js -sFORCE_FILESYSTEM -sASYNCIFY -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=128MB -sTOTAL_MEMORY=256MB -sSTACK_SIZE=16000000 -sASYNCIFY_STACK_SIZE=16000000")
	if (EMSCRIPTEN_SINGLE_THREAD)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++20")
		target_compile_definitions(${TARGET_NAME} PUBLIC EMSCRIPTEN_SINGLE_THREAD)
	else()
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -sPTHREAD_POOL_SIZE=16 ")
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -sPTHREAD_POOL_SIZE=16 ")
	endif()
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

	if (LOCAL_DEBUG)
		add_custom_target(
			download_main150727_pkg ALL
			COMMAND curl http://10.28.18.3/main_dev.pkg -o main150727.pkg
			COMMENT "下载  main150727.pkg"
			OUTPUT_DIRECTORY ${APP_ROOT}
			WORKING_DIRECTORY ${APP_ROOT}
		)

		add_custom_target(
			download_assets_manifest_txt ALL
			COMMAND curl http://10.28.18.3/assets_manifest.txt._P_E_0 -o assets_manifest.txt
			COMMENT "下载 assets_manifest.txt"
			OUTPUT_DIRECTORY ${APP_ROOT}
			WORKING_DIRECTORY ${APP_ROOT}
		)

		add_custom_target(
			download_ParacraftBuildinMod_zip ALL
			COMMAND curl http://10.28.18.3/ParacraftBuildinMod.zip -o npl_packages/ParacraftBuildinMod.zip
			COMMENT "下载  ParacraftBuildinMod.zip"
			OUTPUT_DIRECTORY ${APP_ROOT}
			WORKING_DIRECTORY ${APP_ROOT}
		)
	endif()
	
	# 拷贝文件系统到根目录
	add_custom_target(
		paracraft_asset ALL
		COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset.data --preload "${APP_ROOT}@/" --js-output=paracraft_asset.js
		# COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset.data --embed "${APP_ROOT}@/" --js-output=paracraft_asset.js
		COMMENT "打包资源文件"
		OUTPUT_DIRECTORY ${HTML_ROOT}
		WORKING_DIRECTORY ${HTML_ROOT}
	)

	# # script
	# add_custom_target(
	# 	paracraft_script
	# 	COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_script.data --preload "/mnt/d/workspace/npl/paracraft_script/Mod@/Mod" --js-output=paracraft_script.js
	# 	COMMENT "打包资源文件"
	# 	OUTPUT_DIRECTORY ${HTML_ROOT}
	# 	WORKING_DIRECTORY ${HTML_ROOT}
	# )

	# 拷贝缓存文件系统到根目录
	add_custom_target(
		# paracraft_asset_temp
		paracraft_asset_temp ALL
		COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset_temp.data --preload "${APP_ROOT}_temp@/" --js-output=paracraft_asset_temp.js
		# paracraft_asset_temp ALL
		# COMMAND $ENV{EMSDK}/upstream/emscripten/tools/file_packager paracraft_asset_temp.data --embed "${APP_ROOT}_temp@/" --js-output=paracraft_asset_temp.js
		COMMENT "打包缓存资源文件"
		OUTPUT_DIRECTORY ${HTML_ROOT}
		WORKING_DIRECTORY ${HTML_ROOT}
	)
endif()

# emcmake cmake -S NPLRuntime_Emscripten/NPLRuntime -B build/emscripten -DEMSCRIPTEN=ON -DCMAKE_BUILD_TYPE=RELEASE -DLOCAL_DEBUG=TRUE
# emcmake cmake -S NPLRuntime_EmscriptenSingleThread/NPLRuntime -B build/emscripten -DEMSCRIPTEN=ON -DCMAKE_BUILD_TYPE=RELEASE -DLOCAL_DEBUG=TRUE
# emcmake cmake -S NPLRuntime_EmscriptenSingleThread/NPLRuntime -B build/emscripten_single_thread -DEMSCRIPTEN=ON -DEMSCRIPTEN_SINGLE_THREAD=ON -DCMAKE_BUILD_TYPE=RELEASE -DLOCAL_DEBUG=TRUE

# 多线程
# emcmake cmake -S NPLRuntime_EmscriptenSingleThread/NPLRuntime -B build/emscripten -DEMSCRIPTEN=ON -DCMAKE_BUILD_TYPE=RELEASE 
# 单线程
# emcmake cmake -S NPLRuntime_EmscriptenSingleThread/NPLRuntime -B build/emscripten_single_thread -DEMSCRIPTEN=ON -DEMSCRIPTEN_SINGLE_THREAD=ON -DCMAKE_BUILD_TYPE=RELEASE 