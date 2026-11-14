# Title: externals Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2021.10.28

set(LUA_LIB_LIST)

if (NPLRUNTIME_LUAJIT21) 
	add_subdirectory(LuaJIT2_1 ${PROJECT_BINARY_DIR}/LuaJIT-2.1)
	list(APPEND LUA_LIB_LIST luajit21)
	set(LUA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LuaJIT2_1/code)
endif ()

if (NPLRUNTIME_LUA51)
	add_subdirectory(Lua5_1 ${PROJECT_BINARY_DIR}/lua-5.1.4)
	target_include_directories(lua INTERFACE Lua5_1/src)
	list(APPEND LUA_LIB_LIST lua)
	set(LUA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lua5_1)
endif ()

if (NPLRUNTIME_LUAJIT20)
	add_subdirectory(LuaJIT2_0 ${PROJECT_BINARY_DIR}/LuaJIT)
	list(APPEND LUA_LIB_LIST luajit20)
	set(LUA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LuaJIT2_0/code)
endif ()

set_property(GLOBAL PROPERTY LUA_LIB_LIST ${LUA_LIB_LIST})

add_subdirectory(jsoncpp)
add_subdirectory(tinyxml)

if (NOT EMSCRIPTEN)
	add_subdirectory(zlib)
	set(ZLIB_FOUND 1)
	set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
	set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIRS})
else()
	# Match the SDK's -sUSE_ZLIB port, including FreeType's gzip support.
	# The embedded FreeType zlib lacks the DEBUG tracing definitions.
	find_package(ZLIB REQUIRED)
endif()

add_subdirectory(tinyxpath)
add_subdirectory(curl)
add_subdirectory(luabind)

set(ICONV_LIB_NAME)

if (NPLRUNTIME_ICONV)
	if (APPLE)
		list(APPEND ICONV_LIB_NAME iconv)
	else ()
		add_subdirectory(libiconv)
        list(APPEND ICONV_LIB_NAME libiconv)
        target_compile_definitions(${ICONV_LIB_NAME} PUBLIC USE_ICONV)
	endif ()
endif ()

set_property(GLOBAL PROPERTY ICONV_LIB_NAME ${ICONV_LIB_NAME})

if (NPLRUNTIME_PHYSICS)
	add_subdirectory(bullet3 ${PROJECT_BINARY_DIR}/bullet3)
endif ()

if (NOT EMSCRIPTEN)
	add_subdirectory(png)
	add_subdirectory(jpeg)
endif()
add_subdirectory(freetype-2.8.1)
if (NPLRUNTIME_AUTO_RIGGER)
	add_subdirectory(Pinocchio)
endif()

if (NPLRUNTIME_SERVER OR (${NPLRUNTIME_RENDERER} STREQUAL "NULL" AND NOT NPLRUNTIME_ENABLE_OPENGL AND NOT NPLRUNTIME_ENABLE_D3D11 AND NOT NPLRUNTIME_ENABLE_D3D12 AND NOT NPLRUNTIME_ENABLE_VULKAN AND NOT NPLRUNTIME_ENABLE_METAL AND NOT NPLRUNTIME_ENABLE_WEBGPU))
    # exclude graphics lib for headless server builds
else ()
    add_subdirectory(dxEffectsParser)
    add_subdirectory(hlsl2glslfork)
	if (LINUX)
	else()
    	add_subdirectory(FreeImage3180)
	endif()

	if (NOT EMSCRIPTEN)
	    add_subdirectory(glad)
	endif()

	if (NPLRUNTIME_SUPPORT_FBX) 
		set(ASSIMP_NO_EXPORT OFF CACHE BOOL "Disable Assimp's export functionality." FORCE)
		set(ASSIMP_BUILD_STL_IMPORTER TRUE CACHE BOOL "Enable STL import for robot visual asset conversion" FORCE)
		set(ASSIMP_BUILD_OBJ_IMPORTER TRUE CACHE BOOL "Enable OBJ import for asset conversion" FORCE)
		set(ASSIMP_BUILD_COLLADA_IMPORTER TRUE CACHE BOOL "Enable Collada DAE import for robot visual asset conversion" FORCE)
		set(ASSIMP_BUILD_FBX_EXPORTER TRUE CACHE BOOL "FBX" FORCE)
		set(ASSIMP_BUILD_GLTF_EXPORTER TRUE CACHE BOOL "GLTF" FORCE)
		add_subdirectory(assimp-5.0.1 ${PROJECT_BINARY_DIR}/assimp-5.0.1)
		target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assimp-5.0.1/include)
		target_include_directories(assimp INTERFACE ${PROJECT_BINARY_DIR}/assimp-5.0.1/include)
		target_compile_definitions(assimp INTERFACE SUPPORT_FBX_MODEL_FILE)
	endif ()
endif ()

add_subdirectory(EmbedResource)
add_subdirectory(openssl)
add_subdirectory(antlr)

if (NPLRUNTIME_DARKNET)
	add_subdirectory(darknet)
endif()

if (ANDROID OR HARMONY_OS)
else ()
    #set(SERVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/trunk/)

    #if (NOT NPLRUNTIME_STATIC_LIB) 
	#	include(../cmake/ucm.cmake)
	#	include(../cmake/ConfigBoost.cmake)
    #    add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${PROJECT_BINARY_DIR}/NPLMono/NPLMono2)
    #endif ()
    #if (NOT NPLRUNTIME_STATIC_LIB) 
    #    # add_subdirectory (${SERVER_SOURCE_DIR}luasql ${PROJECT_BINARY_DIR}/luasql)
    #endif ()
endif ()

if(NPLRUNTIME_WEBVIEW)
	add_subdirectory (webview ${PROJECT_BINARY_DIR}/webview)
endif()
