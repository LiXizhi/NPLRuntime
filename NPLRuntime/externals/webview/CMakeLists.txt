set(NPLRUNTIME_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

if(NOT Boost_INCLUDE_DIRS)
	set(PROJECT_SOURCE_DIR "${NPLRUNTIME_ROOT_DIR}")
	include("${NPLRUNTIME_ROOT_DIR}/cmake/ConfigBoost.cmake")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(webview_arch x64)
else()
	set(webview_arch x86)
endif()

################################
# standalone executable
################################
add_executable(webview
	${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
set_target_properties(webview PROPERTIES
	WIN32_EXECUTABLE true
	DEBUG_OUTPUT_NAME "webview_d"
)
target_compile_definitions(webview PRIVATE
	_HAS_STD_BYTE=0
	_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)
if(MSVC)
	target_compile_options(webview PRIVATE /MP $<$<CONFIG:Release>:/GR->)
endif()
target_include_directories(webview PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include")
target_link_libraries(webview PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib"
	dcomp.lib
	urlmon.lib
)

ADD_CUSTOM_COMMAND(
	TARGET webview
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:webview> ${OUTPUT_BIN_DIR}
)

################################
# ParaWebView DLL for ParaEngine
################################
set(ParaEngineClient_SOURCE_DIR "${NPLRUNTIME_ROOT_DIR}/ParaEngine")
set(JSONCPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../jsoncpp")
file(GLOB jsoncpp_headers "${JSONCPP_ROOT}/include/json/*.h")
file(GLOB jsoncpp_sources
	"${JSONCPP_ROOT}/src/lib_json/*.cpp"
	"${JSONCPP_ROOT}/src/lib_json/*.h"
	"${JSONCPP_ROOT}/src/lib_json/*.inl"
)

add_library(ParaWebView SHARED
	${jsoncpp_headers}
	${jsoncpp_sources}
	${CMAKE_CURRENT_SOURCE_DIR}/webview.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/paramain.cpp
)
set_target_properties(ParaWebView PROPERTIES DEBUG_POSTFIX "_d")
target_compile_definitions(ParaWebView PRIVATE
	_HAS_STD_BYTE=0
	_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)
if(MSVC)
	target_compile_options(ParaWebView PRIVATE /MP $<$<CONFIG:Release>:/GR->)
endif()
target_include_directories(ParaWebView PRIVATE
	"${ParaEngineClient_SOURCE_DIR}"
	"${ParaEngineClient_SOURCE_DIR}/Core"
	"${ParaEngineClient_SOURCE_DIR}/NPL"
	"${ParaEngineClient_SOURCE_DIR}/math"
	"${ParaEngineClient_SOURCE_DIR}/Framework/Interface"
	"${ParaEngineClient_SOURCE_DIR}/util"
	${Boost_INCLUDE_DIRS}
	"${JSONCPP_ROOT}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/include"
)
target_link_libraries(ParaWebView PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/${webview_arch}/lib/WebView2LoaderStatic.lib"
	dcomp.lib
	urlmon.lib
)

ADD_CUSTOM_COMMAND(
	TARGET ParaWebView
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaWebView> ${OUTPUT_BIN_DIR}
)
