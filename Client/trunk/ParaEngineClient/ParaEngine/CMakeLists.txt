
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)


# FreeImage
SET(FREEIMAGE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/externals/FreeImage/Dist/)
SET(FREEIMAGE_INCLUDE_LIBS ${PROJECT_SOURCE_DIR}/externals/FreeImage/Dist/FreeImage.lib)

ucm_add_dirs(
    Framework
    Dirmonitor
    Core
    BMaxModel
    Renderer
    Protocol
    ParaXModel
    IO
    Curllua
    DebugTools
    InfoCenter
    BlockEngine
    Math
    NPL
    ParaScriptBindings
    2DEngine
    PaintEngine
    3Dengine
    Util
    WebSocket
    Engine
    VoxelMesh
    CadModel
    d3dcommon
    Util

TO sources RECURSIVE
)



add_library(ParaEngine ${sources})
target_include_directories(ParaEngine PUBLIC 
    ./
    ./Core
    ./Renderer
    ./3dengine
    ./Engine
    ./IO
    ./InfoCenter
    ./Util
    ./Math
    ./ParaScriptBindings
    ./NPL
    ./2dengine/Video
    ./Framework/Interface
    ${Boost_INCLUDE_DIRS}
    ${FREEIMAGE_INCLUDE_DIRS}
)

target_link_libraries(ParaEngine PUBLIC 
    ${Boost_LIBRARIES}
    ${FREEIMAGE_INCLUDE_LIBS}
    zlib
    libcurl
    liblua
    jsoncpp
    tinyxpath
    luabind
    sqlite
    RenderSystemD3D9
)

target_compile_definitions(ParaEngine PUBLIC 
    PLATFORM_WINDOWS
    SQLITE_STATICLIB
    USE_DIRECTX_RENDERER
    PARAENGINE_CLIENT
    PE_CORE_EXPORTING
    _WIN32_WINNT=0x0501
    TIXML_USE_STL
    #WARNINGS
    BOOST_SIGNALS_NO_DEPRECATION_WARNING
    _WINSOCK_DEPRECATED_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
    NPLRUNTIME
    USE_WINDOWS_SSPI
    SQLITE_STATICLIB
)



IF(MSVC)
    ucm_set_runtime(STATIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
ENDIF()