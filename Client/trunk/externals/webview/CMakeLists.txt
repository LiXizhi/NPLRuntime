cmake_minimum_required(VERSION 3.20.0)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(webview)

IF(MSVC)
	# statically link MSVC to reduce dependancies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	 	if(${flag_var} MATCHES "/MDd")
	 		string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MDd")
	endforeach(flag_var)

	# enable multiprocessor build option /MP, this will greatly increase compile speed
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")

	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(webview_arch x64)
else()
	set(webview_arch x86)
endif()

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

list(APPEND Exe_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp)
list(APPEND Exe_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${Exe_Src_list})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib")

if (NPLRUNTIME_WEBVIEW)
	set(BoostRoot_DIR "$ENV{BOOST_ROOT}")
	set(ParaEngineClient_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaEngineClient)
	set(LibName "ParaWebView")
	list(APPEND Lib_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp)
	list(APPEND Lib_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/paramain.cpp)
	add_library(${LibName} SHARED ${Lib_Src_list})
	target_include_directories(${LibName} PUBLIC ${BoostRoot_DIR} ${ParaEngineClient_SOURCE_DIR}/Core ${ParaEngineClient_SOURCE_DIR}/math ${ParaEngineClient_SOURCE_DIR}/util)
	target_include_directories(${LibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include")
	target_link_libraries(${LibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib")
	ADD_CUSTOM_COMMAND(
		TARGET ${LibName}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${LibName}> D:/workspace/npl/NPLRuntime_win32/build/ParaEngineClient/Debug
	 )
endif()
