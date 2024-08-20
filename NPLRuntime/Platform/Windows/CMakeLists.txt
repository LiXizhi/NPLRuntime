include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


set(src_render_context)

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    set(RenderSystem RenderSystemOpenGL)
    list(APPEND RenderSystem  glad_ext_wgl opengl32.lib)
    ucm_add_dirs(src/Render/context/wgl TO src_render_context)
#}
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
#{
    set(RenderSystem RenderSystemD3D9)
    ucm_add_dirs(src/Render/context/d3d9 TO src_render_context)
#}
endif()
ucm_add_dirs(src/Framework TO src_framework RECURSIVE)
ucm_add_dirs(src TO src_platform)
ucm_add_dirs(src/WebView TO src_platform)
ucm_add_dirs(src/ParaScriptBindings TO src_platform)
ucm_add_dirs(src/util TO src_platform)
ucm_add_dirs(src/3dengine TO src_platform)


# Note: if true, we will produce dll instead of exe, only under win32
if(WIN32)
	if(PARAENGINE_CLIENT_DLL)
		set(PARAENGINE_COMPILE_LIB    true)
		
	else()
		set(PARAENGINE_COMPILE_LIB    false)
	endif()
endif()

if (PARAENGINE_COMPILE_LIB)
	set(CMAKE_DEBUG_POSTFIX "_d")
	if(NPLRUNTIME_STATIC_LIB)
        set(NPLRUNTIME_LIB_MODE "STATIC")
        ADD_DEFINITIONS(-DUSE_BOOST_FILE_API)
	else()
		set(NPLRUNTIME_LIB_MODE "SHARED")
	endif()

	add_library(WindowsApplication ${NPLRUNTIME_LIB_MODE} ${src_platform} ${src_render_context} ${src_framework})
else()
	add_executable(WindowsApplication ${src_platform} ${src_render_context} ${src_framework})
    set_property(TARGET WindowsApplication PROPERTY VS_DPI_AWARE "PerMonitor")

	if(WIN32)
		set_target_properties(WindowsApplication PROPERTIES DEBUG_OUTPUT_NAME "ParaEngineClient_d")
		set_target_properties(WindowsApplication PROPERTIES RELEASE_OUTPUT_NAME "ParaEngineClient")
	endif()
endif()

target_compile_definitions(WindowsApplication PRIVATE 
    PLATFORM_WINDOWS
    USE_WINDOWS_SSPI
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0501
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

target_link_libraries(WindowsApplication PRIVATE
    ${RenderSystem}
    ParaEngine
    gdiplus.lib
    comctl32.lib
    Dbghelp.lib
    wsock32.lib
    VFW32.lib
    SHLWAPI.lib
    gdiplus.lib
    Psapi.lib
    Ws2_32
    winmm
    wsock32
    Dnsapi
    Secur32
    Crypt32
    Wldap32
)

target_include_directories(WindowsApplication PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)

if(MSVC)
    ucm_set_runtime(STATIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()

set_property(GLOBAL PROPERTY WindowsApplicationName WindowsApplication)

# Copy files Link the executable to the libraries. 
ADD_CUSTOM_COMMAND(
   TARGET WindowsApplication
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:WindowsApplication> ${OUTPUT_BIN_DIR}
)