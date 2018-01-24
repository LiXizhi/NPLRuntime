include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


set(src_render_context)

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    set(RenderSystem RenderSystemOpenGL)
    list(APPEND RenderSystem  glad_ext_wgl opengl32.lib)
    ucm_add_dirs(Render/context/wgl TO src_render_context)
#}
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
#{
    set(RenderSystem RenderSystemD3D9)
    ucm_add_dirs(Render/context/d3d9 TO src_render_context)
#}
endif()



add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    OSWindows.cpp
    OSWindows.h
    guicon.cpp
    guicon.h
    render/WindowsRenderWindow.cpp
    render/windowsRenderWindow.h
    ApplicationWindow.h
    ApplicationWindow.cpp
    ${src_render_context})

target_compile_definitions(WindowsApplication PUBLIC 
    PLATFORM_WINDOWS
    USE_WINDOWS_SSPI
    _WIN32_WINNT=0x0501
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

target_link_libraries(WindowsApplication PUBLIC
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

target_include_directories(WindowsApplication PRIVATE /)

set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)

if(MSVC)
    ucm_set_runtime(STATIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()