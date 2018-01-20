
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/FindDirectX.cmake)

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
    terrain
    Util

TO sources RECURSIVE
)


file(GLOB SHADER_FILES ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/shaders/*.fx)
foreach(src_file ${SHADER_FILES})
    get_filename_component(src_filename ${src_file} NAME_WE)
    set(out_file "${PROJECT_BINARY_DIR}/ParaEngineClient/shaders/${src_filename}.fxo")

    add_custom_command(
        OUTPUT ${out_file}
        COMMAND "${DirectX_ROOT_DIR}/Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo ${out_file} ${src_file}
        DEPENDS ${src_file}
        COMMENT "fxc compile: ${src_file}"
    )
    embed_resources_abs(Resource ${out_file})
    list(APPEND EmbeddedResource_Files ${Resource})
endforeach(src_file ${SHADER_FILES})

list(APPEND EmbeddedResource_Files ResourceEmbedded.cpp)

embed_resources_abs(Resource ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/res/ParaXmodel.templates)
list(APPEND EmbeddedResource_Files ${Resource})	

embed_resources_abs(Resource ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/res/cursor.tga)
list(APPEND EmbeddedResource_Files ${Resource})

SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_Files})

add_library(ParaEngine ${sources} ${EmbeddedResource_Files})

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
    RenderSystemD3D9
    ${Boost_LIBRARIES}
    ${FREEIMAGE_INCLUDE_LIBS}
    zlib
    libcurl
    liblua
    jsoncpp
    tinyxpath
    luabind
    sqlite 
)

target_compile_definitions(ParaEngine PUBLIC 
    PLATFORM_WINDOWS
    SQLITE_STATICLIB
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