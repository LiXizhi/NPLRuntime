
if(ANDROID)
	add_library(luajit21 INTERFACE)
	target_include_directories(luajit21 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(luajit21 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/${CMAKE_ANDROID_ARCH_ABI}/libluajit.a)
else()
	if(NPLRUNTIME_STATIC_LIB)
		set(LUA_STATICLIB TRUE)
	endif()
	add_subdirectory(code ${CLIENT_BINARY_DIR}/LuaJIT-2.1)
	target_include_directories(luajit21 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
	
endif()



