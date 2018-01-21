include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    set(RenderSystem RenderSystemOpenGL)
    list(APPEND RenderSystem  glad_ext_wgl opengl32.lib)
#}
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
#{
    set(RenderSystem RenderSystemD3D9)
#}
endif()

file (GLOB src_render Render/*.cpp Render/*.h)
source_group("Render" FILES ${src_render})

add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    ${src_render})

target_compile_definitions(WindowsApplication PUBLIC 
    PLATFORM_WINDOWS
    PARAENGINE_CLIENT
    USE_WINDOWS_SSPI
    _WIN32_WINNT=0x0501
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

target_link_libraries(WindowsApplication
    ParaEngine
    ${RenderSystem}
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

if(MSVC)
    ucm_set_runtime(STATIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()