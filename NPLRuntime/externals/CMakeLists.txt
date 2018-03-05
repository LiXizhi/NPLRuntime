
add_subdirectory(jsoncpp)
add_subdirectory(tinyxml)
add_subdirectory(zlib)
add_subdirectory(tinyxpath)

if(${NPLRUNTIME_RENDERER} STREQUAL "NULL")
    # exclude graphics lib if no renderer are specified, such as under linux
else()
    add_subdirectory(dxEffectsParser)
    add_subdirectory(hlsl2glslfork)
    add_subdirectory(freetype-2.8.1)
    add_subdirectory(jpeg)
    add_subdirectory(png)
    add_subdirectory(glad)
endif()

add_subdirectory(EmbedResource)

if(ANDROID)
    add_subdirectory(curl)
    add_subdirectory(LuaJIT2_0)
    add_subdirectory(luabind)

    add_subdirectory(tolua)
    add_subdirectory(openssl)
else()
    set(SERVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Server/trunk/)
    if (NPLRUNTIME_LUAJIT21) 
        add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT-2.1 ${PROJECT_BINARY_DIR}/LuaJIT-2.1)
    endif()
    ## lua 5.1.4 is always compiled for ABI interface
    add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${PROJECT_BINARY_DIR}/lua-5.1.4)
    if(NPLRUNTIME_LUAJIT20)
        add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${PROJECT_BINARY_DIR}/LuaJIT)
    endif()
    ## luabind
    add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 ${PROJECT_BINARY_DIR}/luabind-0.9)

    if(NOT NPLRUNTIME_STATIC_LIB) 
        add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${PROJECT_BINARY_DIR}/NPLMono/NPLMono2)
    endif()
    if(NOT NPLRUNTIME_STATIC_LIB) 
        # add_subdirectory (${SERVER_SOURCE_DIR}luasql ${PROJECT_BINARY_DIR}/luasql)
    endif()

    ## curl is always compiled on win32 and found in other platforms
    if(WIN32)
        option(BUILD_CURL_EXE OFF)
        option(BUILD_CURL_TESTS OFF)
        add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 ${PROJECT_BINARY_DIR}/curl-7.47.1)
    else()
        FIND_PACKAGE(CURL REQUIRED)
    endif()
endif()










