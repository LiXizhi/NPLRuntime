set(LUA_LIB_LIST)


if (NPLRUNTIME_LUAJIT21) 
	add_subdirectory (LuaJIT2_1 ${PROJECT_BINARY_DIR}/LuaJIT-2.1)
	list(APPEND LUA_LIB_LIST luajit21)
endif()
	
if (NPLRUNTIME_LUA51)
	add_subdirectory (Lua5_1 ${PROJECT_BINARY_DIR}/lua-5.1.4)
	target_include_directories(lua INTERFACE Lua5_1/src)
	list(APPEND LUA_LIB_LIST lua)
endif()
		
if(NPLRUNTIME_LUAJIT20)
	add_subdirectory (LuaJIT2_0 ${PROJECT_BINARY_DIR}/LuaJIT)
	list(APPEND LUA_LIB_LIST luajit20)
endif()

set_property(GLOBAL PROPERTY LUA_LIB_LIST ${LUA_LIB_LIST})

add_subdirectory(jsoncpp)
add_subdirectory(tinyxml)

add_subdirectory(zlib)
SET(ZLIB_FOUND 1)

add_subdirectory(tinyxpath)
add_subdirectory(curl)
add_subdirectory(luabind)


set(ICONV_LIB_NAME)

if (NPLRUNTIME_ICONV)
	if (APPLE)
		list(APPEND ICONV_LIB_NAME iconv)
	else()
		add_subdirectory(libiconv)
        list(APPEND ICONV_LIB_NAME libiconv)
        target_compile_definitions(${ICONV_LIB_NAME} PUBLIC USE_ICONV)
	endif()
	
	
endif()

set_property(GLOBAL PROPERTY ICONV_LIB_NAME ${ICONV_LIB_NAME})


if(NPLRUNTIME_PHYSICS)
	add_subdirectory(bullet3 ${PROJECT_BINARY_DIR}/bullet3)
endif()

add_subdirectory(freetype-2.8.1)
add_subdirectory(jpeg)
add_subdirectory(png)
add_subdirectory(Pinocchio)
add_subdirectory(FreeImage3180)

if(${NPLRUNTIME_RENDERER} STREQUAL "NULL")
    # exclude graphics lib if no renderer are specified, such as under linux
else()
    add_subdirectory(dxEffectsParser)
    add_subdirectory(hlsl2glslfork)
    
    add_subdirectory(glad)
	if(NPLRUNTIME_SUPPORT_FBX) 
		add_subdirectory(assimp-4.0.0 ${PROJECT_BINARY_DIR}/assimp-4.0.0)
		target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assimp-4.0.0/include)
		target_include_directories(assimp INTERFACE ${PROJECT_BINARY_DIR}/assimp-4.0.0/include)
		target_compile_definitions(assimp INTERFACE SUPPORT_FBX_MODEL_FILE)
	endif()

endif()

add_subdirectory(EmbedResource)
add_subdirectory(openssl)
if(ANDROID)
   
else()
    #set(SERVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/trunk/)


    #if(NOT NPLRUNTIME_STATIC_LIB) 
	#	include(../cmake/ucm.cmake)
	#	include(../cmake/ConfigBoost.cmake)
    #    add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${PROJECT_BINARY_DIR}/NPLMono/NPLMono2)
    #endif()
    #if(NOT NPLRUNTIME_STATIC_LIB) 
    #    # add_subdirectory (${SERVER_SOURCE_DIR}luasql ${PROJECT_BINARY_DIR}/luasql)
    #endif()
endif()










