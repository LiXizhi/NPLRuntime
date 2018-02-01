
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
    terrain
    Util
TO sources_commmon RECURSIVE
)

message(STATUS "RenderSytem:"${NPLRUNTIME_RENDERER})

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    set(RenderSystem RenderSystemOpenGL)
    ucm_add_dirs(
        OpenGLWrapper
    TO sources_rendersystem RECURSIVE
    )
    file(GLOB SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fx/*.fx)
    foreach( src_file ${SHADER_FILES} )	
        embed_resources_abs(Resource ${src_file})
        list(APPEND EmbeddedResource_Files_OPENGL ${Resource})
    endforeach( src_file ${SHADER_FILES} )
    list(APPEND EmbeddedResource_Files_OPENGL ResourceEmbedded.cpp)
    SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_Files_OPENGL})
    list(APPEND sources_rendersystem ${EmbeddedResource_Files_OPENGL})
#}
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
#{
    set(RenderSystem RenderSystemD3D9)
    ucm_add_dirs(
        Engine
        VoxelMesh
        CadModel
        d3dcommon
    TO sources_rendersystem RECURSIVE
    )

    # Embedded Resource For Direct3D
    file(GLOB SHADER_FILES ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/shaders/*.fx)
    foreach(src_file ${SHADER_FILES})
        get_filename_component(src_filename ${src_file} NAME_WE)
        set(out_file "${CMAKE_CURRENT_BINARY_DIR}/shaders/${src_filename}.fxo")

        add_custom_command(
            OUTPUT ${out_file}
            COMMAND "${DirectX_ROOT_DIR}/Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo ${out_file} ${src_file}
            DEPENDS ${src_file}
            COMMENT "fxc compile: ${src_file}"
        )
        embed_resources_abs(Resource ${out_file})
        list(APPEND EmbeddedResource_Files_D3D ${Resource})
    endforeach(src_file ${SHADER_FILES})

    list(APPEND EmbeddedResource_Files_D3D ResourceEmbedded.cpp)

    embed_resources_abs(Resource ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/res/ParaXmodel.templates)
    list(APPEND EmbeddedResource_Files_D3D ${Resource})	

    embed_resources_abs(Resource ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/res/cursor.tga)
    list(APPEND EmbeddedResource_Files_D3D ${Resource})

    SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_Files_D3D})

    list(APPEND sources_rendersystem ${EmbeddedResource_Files_D3D})
#}
endif()



###################################################################
## Target

add_library(ParaEngine ${sources_commmon} ${sources_rendersystem})

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
    ${RenderSystem}
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
    SQLITE_STATICLIB
    PE_CORE_EXPORTING
    TIXML_USE_STL
    BOOST_SIGNALS_NO_DEPRECATION_WARNING
    _CRT_SECURE_NO_WARNINGS
    NPLRUNTIME
    SQLITE_STATICLIB
    _WIN32_WINNT=0x0501
    _WINSOCK_DEPRECATED_NO_WARNINGS
    PLATFORM_WINDOWS
)




IF(MSVC)
    ucm_set_runtime(DYNAMIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
ENDIF()