# Title: ParaEngine Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2023.09.26

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
if (NOT EMSCRIPTEN)
    include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
endif()

if (EMSCRIPTEN_SINGLE_THREAD)
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_C_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
else()
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_C_STANDARD 11)
endif()

# includes
set(NPLRUNTIME_INCLUDES)

# Libraries 
set(NPLRUNTIME_LIBS)
##############################

ucm_add_dirs(
    Framework
    dirmonitor
    Core
    BMaxModel
    renderer
    protocol
    ParaXModel
    IO
    debugtools
    InfoCenter
    BlockEngine
    math
    NPL
    ParaScriptBindings
    2dengine
    PaintEngine
    3dengine
    util
    WebSocket
    terrain
	AutoRigger
    TO sources_commmon RECURSIVE
)

if (NOT EMSCRIPTEN)
    ucm_add_dirs(curllua TO sources_commmon RECURSIVE)
endif()

if (NPLRUNTIME_RENDERER STREQUAL "OPENGL")
    set(RenderSystem RenderSystemOpenGL)

    ucm_add_dirs(OpenGLWrapper TO sources_rendersystem RECURSIVE)

    file(GLOB SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/opengl_spec/*.fx)

    foreach(src_file ${SHADER_FILES})	
        embed_resources_abs(Resource ${src_file})
        list(APPEND EmbeddedResource_Files_OPENGL ${Resource})
    endforeach(src_file ${SHADER_FILES})

    list(APPEND EmbeddedResource_Files_OPENGL ResourceEmbedded.cpp)
    source_group("ResEmbedded" FILES ${EmbeddedResource_Files_OPENGL})
    list(APPEND sources_rendersystem ${EmbeddedResource_Files_OPENGL})

	source_group("ShaderFiles" FILES ${SHADER_FILES})
	set_source_files_properties(${SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
	list(APPEND sources_rendersystem ${SHADER_FILES})
elseif (NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
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

	embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/paraworld.ico)
    list(APPEND EmbeddedResource_Files_D3D ${Resource})

    source_group("ResEmbedded" FILES ${EmbeddedResource_Files_D3D})

    list(APPEND sources_rendersystem ${EmbeddedResource_Files_D3D})

    # FreeImage
	list(APPEND NPLRUNTIME_LIBS FreeImage)

	if (NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
		add_definitions(-DUSE_GDI_COMPOSER) # for css skin, etc
	endif ()

    # if (WIN32)
    #     option(NPLRUNTIME_FREEIMAGE "include FreeImage" ON)

    #     if (NPLRUNTIME_FREEIMAGE)
    #         add_definitions(-DUSE_FREEIMAGE) # for reading image files
    #         if (NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
    #             add_definitions(-DUSE_GDI_COMPOSER) # for css skin, etc
    #         endif ()
    #         if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    #             SET(FREEIMAGE_DASM_FLAGS "x64")
    #         else ()
    #             SET(FREEIMAGE_DASM_FLAGS "x32")
    #         endif ()

    #         list(APPEND NPLRUNTIME_INCLUDES ${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS})
    #         list(APPEND NPLRUNTIME_LIBS ${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}/FreeImage.lib)
    #         configure_file("${PROJECT_SOURCE_DIR}/externals/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}/FreeImage.dll" "${OUTPUT_BIN_DIR}/FreeImage.dll" COPYONLY)
    #     endif ()
    # endif ()
elseif (NPLRUNTIME_RENDERER STREQUAL "NULL")
    set(RenderSystem RenderSystemNull)
    list(APPEND sources_rendersystem ResourceEmbedded.cpp)
endif ()

list(APPEND NPLRUNTIME_INCLUDES ${PROJECT_SOURCE_DIR}/externals/Pinocchio/include)

# remove some compile time warnings
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

if (SDL)
    list(APPEND NPLRUNTIME_INCLUDES ${SDL_ROOT}/include)
endif()
###################################################################
## Target

add_library(ParaEngine ${sources_commmon} ${sources_rendersystem})

set(include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/3dengine
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    ${CMAKE_CURRENT_SOURCE_DIR}/IO
    ${CMAKE_CURRENT_SOURCE_DIR}/InfoCenter
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaScriptBindings
    ${CMAKE_CURRENT_SOURCE_DIR}/NPL
    ${CMAKE_CURRENT_SOURCE_DIR}/Framework/Interface
    ${CMAKE_CURRENT_SOURCE_DIR}/AutoRigger
    ${Boost_INCLUDE_DIRS}
    ${NPLRUNTIME_INCLUDES}
)

if (APPLE AND NOT IOS)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/OSX/src)
elseif (ANDROID)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/AndroidStudio/app/cpp)
endif()

target_include_directories(ParaEngine PUBLIC ${include_dirs})

if (ANDROID)
    set(PLATROM_SPEC_DEFINITIONS PARAENGINE_MOBILE ANDROID PLATFORM_ANDROID AUTOUPDATE_PARAENGINE)
elseif (WIN32)
    set(PLATROM_SPEC_DEFINITIONS PLATFORM_WINDOWS _WIN32_WINNT=0x0600 _WINSOCK_DEPRECATED_NO_WARNINGS)
elseif (IOS)
    set(PLATROM_SPEC_DEFINITIONS PARAENGINE_MOBILE IOS PLATFORM_IPHONE)
elseif (MAC_SERVER)
	set(PLATROM_SPEC_DEFINITIONS PLATFORM_MAC MAC MAC_SERVER)

    # Readline support
    include("${PROJECT_SOURCE_DIR}/cmake/FindReadline.cmake")
    # find_package(Readline)

    if (READLINE_FOUND)
		list(APPEND PLATROM_SPEC_DEFINITIONS NPL_USE_READLINE)
		list(APPEND PLATFORM_SPE_LIBS ${Readline_LIBRARY})

		message("NPL_USE_READLINE read line is found and used")

		include_directories(${READLINE_INCLUDE_DIR})
	else ()
		message("warning: NPL_USE_READLINE NOT FOUND please apt-get install libreadline6 libreadline6-dev")
	endif ()
elseif (APPLE AND NOT IOS)
    set(PLATROM_SPEC_DEFINITIONS PLATFORM_MAC)
elseif (UNIX AND NOT ANDROID)
    # Readline support
    include("${PROJECT_SOURCE_DIR}/cmake/FindReadline.cmake")

    # find_package(Readline)
    if (READLINE_FOUND)
        set(PLATROM_SPEC_DEFINITIONS NPL_USE_READLINE)
        list(APPEND PLATFORM_SPE_LIBS ${Readline_LIBRARY})

        message("NPL_USE_READLINE read line is found and used")

        include_directories(${READLINE_INCLUDE_DIR})
    else ()
        message("warning: NPL_USE_READLINE NOT FOUND please apt-get install libreadline6 libreadline6-dev")
    endif ()
endif ()

get_property(ICONV_LIB_NAME GLOBAL PROPERTY "ICONV_LIB_NAME") 
get_property(LUA_LIB_LIST GLOBAL PROPERTY "LUA_LIB_LIST") 

##############################
# assimp
if (NPLRUNTIME_SUPPORT_FBX)
	list(APPEND NPLRUNTIME_LIBS assimp)
endif ()

if (NPLRUNTIME_PHYSICS AND PHYSICS_STATICLIB)
	list(APPEND NPLRUNTIME_LIBS PhysicsBT)
endif ()

if (AUDIO_STATICLIB AND NPLRUNTIME_AUDIO)
	list(APPEND NPLRUNTIME_LIBS cAudioEngine)
endif ()

if (NPLRUNTIME_OCE)
	list(APPEND NPLRUNTIME_LIBS nploce)
	add_dependencies(ParaEngine nploce)
endif ()

if (EMSCRIPTEN AND NOT NPLOCE_DEPS_ROOT)
    if (EMSCRIPTEN_SINGLE_THREAD)
        set(NPLOCE_DEPS_ROOT "/mnt/d/workspace/npl/nploce-deps/install")
    else()
        set(NPLOCE_DEPS_ROOT "/mnt/d/workspace/npl/nploce-deps/install_thread")
    endif()
endif()
if (EMSCRIPTEN AND NPLOCE_DEPS_ROOT)
    link_directories(${NPLOCE_DEPS_ROOT}/harfbuzz/lib)
    link_directories(${NPLOCE_DEPS_ROOT}/oce/lib)
	list(APPEND NPLRUNTIME_LIBS  
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKBO.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKBool.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKBRep.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKernel.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKFeat.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKFillet.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKG2d.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKG3d.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKGeomAlgo.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKGeomBase.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKHLR.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKMath.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKMesh.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKOffset.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKPrim.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKShHealing.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKIGES.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKSTEP.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKSTEP209.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKSTEPAttr.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKSTEPBase.a
        # ${NPLOCE_DEPS_ROOT}/oce/lib/TKXSBase.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKTopAlgo.a
        ${NPLOCE_DEPS_ROOT}/oce/lib/libTKXMesh.a

        ${NPLOCE_DEPS_ROOT}/harfbuzz/lib/libharfbuzz.a
        # ${NPLOCE_DEPS_ROOT}/freetype2_8_1/lib/libfreetype.a
        ${NPLOCE_DEPS_ROOT}/nploce/libnploce.a
    )
endif()

if (EMSCRIPTEN)
    if (NOT EMSCRIPTEN_SINGLE_THREAD)
        list(APPEND NPLRUNTIME_LIBS libcurl)
    endif()
    
    target_link_libraries(ParaEngine PUBLIC 
    ${RenderSystem}
    ${NPLRUNTIME_LIBS}
    ${PLATFORM_SPE_LIBS}
    ${LUA_LIB_LIST}
    jsoncpp
    tinyxpath
    luabind
    ParaSqlite 
    Pinocchio
    ParaAntlr4
    ${ICONV_LIB_NAME}
    "${BOOST_ROOT}/stage/lib/libboost_atomic.a"
    "${BOOST_ROOT}/stage/lib/libboost_chrono.a"
    "${BOOST_ROOT}/stage/lib/libboost_date_time.a"
    "${BOOST_ROOT}/stage/lib/libboost_date_time.a"
    "${BOOST_ROOT}/stage/lib/libboost_filesystem.a"
    "${BOOST_ROOT}/stage/lib/libboost_serialization.a"
    "${BOOST_ROOT}/stage/lib/libboost_system.a"
    "${BOOST_ROOT}/stage/lib/libboost_thread.a"
    "${BOOST_ROOT}/stage/lib/libboost_wserialization.a"
    )
else()
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
        ParaSqlite 
        libjpeg
        libpng
        Pinocchio
        ParaAntlr4
        ${ICONV_LIB_NAME}
    )
endif()


set(PARAENGINE_DEF_LIST)
list(APPEND PARAENGINE_DEF_LIST USE_FREEIMAGE USE_ICONV TIXML_USE_STL BOOST_SIGNALS_NO_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS NPLRUNTIME ${PLATROM_SPEC_DEFINITIONS})
if (SDL)
    list(APPEND PARAENGINE_DEF_LIST PLATFORM_SDL2)
endif()

if (WIN32 AND PARAENGINE_CLIENT_DLL)
	list(APPEND PARAENGINE_DEF_LIST NPLRUNTIME_DLL)
endif ()

if (SQLITE_STATICLIB)
	list(APPEND PARAENGINE_DEF_LIST SQLITE_STATICLIB)
endif ()

if (PHYSICS_STATICLIB)
	list(APPEND PARAENGINE_DEF_LIST PHYSICS_STATICLIB)
endif ()

if (AUDIO_STATICLIB AND NPLRUNTIME_AUDIO)
	list(APPEND PARAENGINE_DEF_LIST STATIC_PLUGIN_CAUDIOENGINE)
endif ()

if (NPLRUNTIME_STATIC_LIB)
	list(APPEND PARAENGINE_DEF_LIST CURL_STATICLIB)
	list(APPEND PARAENGINE_DEF_LIST PE_CORE_EXPORTING)
else ()
	#add_definitions(-DPE_CORE_EXPORTING)
	list(APPEND PARAENGINE_DEF_LIST PE_CORE_EXPORTING)
endif ()

if (NPLRUNTIME_SERVER)
	list(APPEND PARAENGINE_DEF_LIST NPLRUNTIME_SERVER)
endif ()

target_compile_definitions(ParaEngine PUBLIC ${PARAENGINE_DEF_LIST})

if (MSVC)
    ucm_set_runtime(STATIC)

    # enable multiprocessor build option /MP, this will greatly increase compile speed
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif ()  
