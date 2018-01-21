
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
file (GLOB src_render Render/*.cpp Render/*.h)
source_group("Render" FILES ${src_render})

add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    ${src_render}
    ${src_render_d3d9}
)

target_compile_definitions(WindowsApplication PUBLIC 
    PLATFORM_WINDOWS
    PARAENGINE_CLIENT
    USE_WINDOWS_SSPI
    _WIN32_WINNT=0x0501
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

target_link_libraries(WindowsApplication
    ParaEngine
    RenderSystemD3D9
    opengl32.lib
    glad_core
    glad_ext_wgl
    gdiplus.lib
    comctl32.lib
    Dbghelp.lib
    wsock32.lib
    VFW32.lib
    SHLWAPI.lib
    gdiplus.lib
    Psapi.lib
    Ws2_32 winmm wsock32 Dnsapi Secur32 Crypt32 Wldap32
)
set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)

IF(MSVC)
    ucm_set_runtime(STATIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
ENDIF()