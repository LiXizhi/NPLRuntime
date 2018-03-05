add_library(lua INTERFACE)
target_include_directories(lua INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(ANDROID)
    target_link_libraries(lua INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/${CMAKE_ANDROID_ARCH_ABI}/libluajit.a)
elseif(WIN32)
    target_link_libraries(lua INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win32/lua51.lib)
    add_custom_target(Lua
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win32/lua51.dll ${CMAKE_BINARY_DIR}/bin/lua51.dll
    )
endif()