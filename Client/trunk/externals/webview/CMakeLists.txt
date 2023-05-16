cmake_minimum_required(VERSION 3.16.0)

################################
# standalone executable
################################
project(webview)

set(BoostRoot_DIR "$ENV{BOOST_ROOT}")

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

list(APPEND Exe_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp)
list(APPEND Exe_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${PROJECT_NAME} ${Exe_Src_list})
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_OUTPUT_NAME "${PROJECT_NAME}_d")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib")

ADD_CUSTOM_COMMAND(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${OUTPUT_BIN_DIR}
	)


################################
# webview dll for paraengine
################################
project(ParaWebView)

set(BoostRoot_DIR "$ENV{BOOST_ROOT}")
	
####################################################
# ParaEngine related 
# ParaEngineClient_SOURCE_DIR can be used to specify a single base
# folder where the required dependencies may be found.
set(ParaEngineClient_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaEngineClient)
include_directories("${ParaEngineClient_SOURCE_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/Core")
include_directories("${ParaEngineClient_SOURCE_DIR}/NPL")
include_directories("${ParaEngineClient_SOURCE_DIR}/math")
include_directories("${ParaEngineClient_SOURCE_DIR}/util")

set(LibName "ParaWebView")
set(CMAKE_DEBUG_POSTFIX "_d")

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


##############################
# add src files
file (GLOB_RECURSE Main_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
list(APPEND Lib_Src_list ${Main_Src_list})

list(APPEND Lib_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp)
list(APPEND Lib_Src_list ${CMAKE_CURRENT_SOURCE_DIR}/paramain.cpp)

add_library(${LibName} SHARED ${Lib_Src_list})
target_include_directories(${LibName} PUBLIC ${BoostRoot_DIR})
target_include_directories(${LibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include")
target_link_libraries(${LibName} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib")
ADD_CUSTOM_COMMAND(
	TARGET ${LibName}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${LibName}> ${OUTPUT_BIN_DIR}
	)
