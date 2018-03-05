add_subdirectory(curl)
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

if(NPLRUNTIME_LUAJIT20)
    add_subdirectory(LuaJIT2_0)
endif()
if (NPLRUNTIME_LUAJIT21) 
    add_subdirectory(LuaJIT2_1)
endif()
if (NPLRUNTIME_LUA51)
    add_subdirectory(Lua5_1)
endif()

if(ANDROID)
    add_subdirectory(tolua)
    add_subdirectory(openssl)
endif()
add_subdirectory(luabind)








