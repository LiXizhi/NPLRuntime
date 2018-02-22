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
add_subdirectory(luajit)
if(ANDROID)
    add_subdirectory(tolua)
    add_subdirectory(openssl)
endif()
add_subdirectory(luabind)








