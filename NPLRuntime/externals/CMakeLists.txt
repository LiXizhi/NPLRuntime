add_subdirectory(curl)
add_subdirectory(jsoncpp)
add_subdirectory(dxEffectsParser)
add_subdirectory(hlsl2glslfork)
add_subdirectory(tinyxml)
add_subdirectory(zlib)
add_subdirectory(tinyxpath)
add_subdirectory(glad)
add_subdirectory(freetype-2.8.1)
add_subdirectory(jpeg)
add_subdirectory(png)
add_subdirectory(EmbedResource)

if(${NPLRUNTIME_LUA_VERSION} STREQUAL "LuaJit2_0")
    add_subdirectory(LuaJIT2_0)
elseif(${NPLRUNTIME_LUA_VERSION} STREQUAL "LuaJit2_1")
    add_subdirectory(LuaJIT2_1)
elseif(${NPLRUNTIME_LUA_VERSION} STREQUAL "Lua51")   
    add_subdirectory(Lua5_1)
endif()
if(ANDROID)
    add_subdirectory(tolua)
    add_subdirectory(openssl)
endif()
add_subdirectory(luabind)








