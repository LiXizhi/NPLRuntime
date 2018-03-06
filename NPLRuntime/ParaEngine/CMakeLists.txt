
include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)

# includes
set(NPLRUNTIME_INCLUDES)
# Libraries 
set(NPLRUNTIME_LIBS)

# FreeImage
#SET(FREEIMAGE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/externals/FreeImage/Dist/)
#SET(FREEIMAGE_INCLUDE_LIBS ${PROJECT_SOURCE_DIR}/externals/FreeImage/Dist/FreeImage.lib)

##############################
# FreeImage
if (WIN32)
	option(NPLRUNTIME_FREEIMAGE "include FreeImage" ON)

	if (NPLRUNTIME_FREEIMAGE)
		ADD_DEFINITIONS(-DUSE_FREEIMAGE) # for reading image files
		if(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
			ADD_DEFINITIONS(-DUSE_GDI_COMPOSER) # for css skin, etc
		endif()

		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			SET(FREEIMAGE_DASM_FLAGS "x64")
		else()
			SET(FREEIMAGE_DASM_FLAGS "x32")
		endif()

		#if(WIN32)
		#	if (NPLRUNTIME_RENDERER STREQUAL "OPENGL")
		#		file (GLOB_RECURSE ParaEngineClient_FREEIMIAGE ${ParaEngineClient_SOURCE_DIR}/Engine/FreeImageMemIO.h ${ParaEngineClient_SOURCE_DIR}/Engine/FreeImageMemIO.cpp)
		#		SOURCE_GROUP("Engine" FILES ${ParaEngineClient_FREEIMIAGE})
		#		list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_FREEIMIAGE})
		#	endif()
		#endif()

		
		list(APPEND NPLRUNTIME_INCLUDES ${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS})
		list(APPEND NPLRUNTIME_LIBS ${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}/FreeImage.lib)
		configure_file("${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}/FreeImage.dll" "${OUTPUT_BIN_DIR}/FreeImage.dll" COPYONLY)
	endif()
endif()

ucm_add_dirs(
    Framework
    dirmonitor
    Core
    BMaxModel
    renderer
    protocol
    ParaXModel
    IO
    curllua
    debugtools
    InfoCenter
    BlockEngine
    math
    NPL
    ParaScriptBindings
    2dengine
    PaintEngine
    3dengine
    Util
    WebSocket
    terrain
TO sources_commmon RECURSIVE
)

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
#{
    set(RenderSystem RenderSystemOpenGL)
    ucm_add_dirs(
        OpenGLWrapper
    TO sources_rendersystem RECURSIVE
    )
    file(GLOB SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/opengl_spec/*.fx)
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
    include(${PROJECT_SOURCE_DIR}/cmake/FindDirectX.cmake)
    set(RenderSystem RenderSystemD3D9)
    ucm_add_dirs(
        Engine
        VoxelMesh
        CadModel
        d3dcommon
    TO sources_rendersystem RECURSIVE
    )

    # Embedded Resource For Direct3D
    file(GLOB SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/d3d9_spec/*.fx)
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

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/ParaXmodel.templates)
    list(APPEND EmbeddedResource_Files_D3D ${Resource})	

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/cursor.tga)
    list(APPEND EmbeddedResource_Files_D3D ${Resource})

    SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_Files_D3D})

    list(APPEND sources_rendersystem ${EmbeddedResource_Files_D3D})
#}
endif()



###################################################################
## Target

add_library(ParaEngine  ${sources_commmon} ${sources_rendersystem})

target_include_directories(ParaEngine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/3dengine
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/IO
    ${CMAKE_CURRENT_SOURCE_DIR}/InfoCenter
    ${CMAKE_CURRENT_SOURCE_DIR}/Util
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaScriptBindings
    ${CMAKE_CURRENT_SOURCE_DIR}/NPL
    ${CMAKE_CURRENT_SOURCE_DIR}/Framework/Interface
    ${Boost_INCLUDE_DIRS}
    ${NPLRUNTIME_INCLUDES}
)


if(ANDROID)
    set(PLATROM_SPEC_DEFINITIONS 
        PARAENGINE_MOBILE
        PLATFORM_ANDROID
        STATIC_PLUGIN_CAUDIOENGINE
        AUTOUPDATE_PARAENGINE
    )
    set(PLATFORM_SPE_LIBS
        tolua
    )


elseif(WIN32)
    set(PLATROM_SPEC_DEFINITIONS 
        PLATFORM_WINDOWS
        _WIN32_WINNT=0x0501
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
endif()


target_link_libraries(ParaEngine PUBLIC 
    ${RenderSystem}
    ${Boost_LIBRARIES}
    ${NPLRUNTIME_LIBS}
    ${PLATFORM_SPE_LIBS}
	${LUA_LIB_LIST}
    zlib
	libcurl
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
    ${PLATROM_SPEC_DEFINITIONS}
)




IF(MSVC)
    ucm_set_runtime(DYNAMIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
ENDIF()  