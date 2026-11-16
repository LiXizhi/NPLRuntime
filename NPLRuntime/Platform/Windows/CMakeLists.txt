include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


set(src_render_context)
set(RenderSystem RenderSystemRHICore RenderSystemNull)

if(NPLRUNTIME_ENABLE_OPENGL OR NPLRUNTIME_RENDERER STREQUAL "OPENGL")
    list(APPEND RenderSystem RenderSystemOpenGL glad_ext_wgl opengl32.lib)
    ucm_add_dirs(src/Render/context/wgl TO src_render_context)
endif()
if(NPLRUNTIME_ENABLE_D3D11 AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/d3d11/CMakeLists.txt")
    list(APPEND RenderSystem RenderSystemD3D11)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/Render/context/d3d11")
        ucm_add_dirs(src/Render/context/d3d11 TO src_render_context)
    endif()
endif()
if(NPLRUNTIME_ENABLE_D3D12 AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/d3d12/CMakeLists.txt")
    list(APPEND RenderSystem RenderSystemD3D12)
endif()
if(NPLRUNTIME_ENABLE_VULKAN AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/vulkan/CMakeLists.txt")
    list(APPEND RenderSystem RenderSystemVulkan)
endif()
if(NPLRUNTIME_ENABLE_WEBGPU AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/webgpu/CMakeLists.txt")
    list(APPEND RenderSystem RenderSystemWebGPU)
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
	add_executable(WindowsApplication ${src_platform} ${src_render_context} ${src_framework} ${PROJECT_SOURCE_DIR}/ParaEngine/ParaEngineRes.rc)
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
    _WIN32_WINNT=0x0A00
    _WINSOCK_DEPRECATED_NO_WARNINGS
    PARA_RHI_MULTI_BACKEND
    $<$<BOOL:${NPLRUNTIME_ENABLE_OPENGL}>:PARA_RHI_HAS_OPENGL>
    $<$<BOOL:${NPLRUNTIME_ENABLE_D3D11}>:PARA_RHI_HAS_D3D11>
    $<$<BOOL:${NPLRUNTIME_ENABLE_D3D12}>:PARA_RHI_HAS_D3D12>
    $<$<BOOL:${NPLRUNTIME_ENABLE_VULKAN}>:PARA_RHI_HAS_VULKAN>
    $<$<BOOL:${NPLRUNTIME_ENABLE_WEBGPU}>:PARA_RHI_HAS_WEBGPU>
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
    # Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()

set_property(GLOBAL PROPERTY WindowsApplicationName WindowsApplication)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_D3D_REDIST_ARCH "x64")
else()
    set(_D3D_REDIST_ARCH "x86")
endif()

find_file(D3DCOMPILER_47_REDIST_DLL
    NAMES d3dcompiler_47.dll
    PATHS
        "$ENV{WindowsSdkDir}/Redist/D3D/${_D3D_REDIST_ARCH}"
        "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/${_D3D_REDIST_ARCH}"
        "C:/Program Files/Windows Kits/10/Redist/D3D/${_D3D_REDIST_ARCH}"
    NO_DEFAULT_PATH
)

# Copy files Link the executable to the libraries. 
ADD_CUSTOM_COMMAND(
   TARGET WindowsApplication
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:WindowsApplication> ${OUTPUT_BIN_DIR}
)

if(D3DCOMPILER_47_REDIST_DLL)
    ADD_CUSTOM_COMMAND(
        TARGET WindowsApplication
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${D3DCOMPILER_47_REDIST_DLL}" "$<TARGET_FILE_DIR:WindowsApplication>/d3dcompiler_47.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${D3DCOMPILER_47_REDIST_DLL}" "${OUTPUT_BIN_DIR}/d3dcompiler_47.dll"
    )
endif()

if(NPL_HAS_DESKTOP_MUJOCO)
    ADD_CUSTOM_COMMAND(
        TARGET WindowsApplication
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:mujoco::mujoco>" "$<TARGET_FILE_DIR:WindowsApplication>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:mujoco::mujoco>" "${OUTPUT_BIN_DIR}"
    )
endif()

if(NPL_HAS_DESKTOP_ONNXRUNTIME)
    ADD_CUSTOM_COMMAND(
        TARGET WindowsApplication
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:onnxruntime::onnxruntime>" "$<TARGET_FILE_DIR:WindowsApplication>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:onnxruntime::onnxruntime>" "${OUTPUT_BIN_DIR}"
    )
endif()

if(WIN32 AND NOT PARAENGINE_COMPILE_LIB)
    add_subdirectory(ParaApplyWaiter)
endif()