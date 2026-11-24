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
    if (ANDROID OR WIN32)
        set(CMAKE_CXX_STANDARD 17)
    else()
        set(CMAKE_CXX_STANDARD 20)
    endif()
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif()

# includes
set(NPLRUNTIME_INCLUDES)

# Libraries 
set(NPLRUNTIME_LIBS)

# Pick up new block renderer sources in existing incremental build trees.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/BlockEngine")

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
    VoxelMesh
    CadModel
    TO sources_commmon RECURSIVE
)

list(APPEND sources_commmon
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/BVHSerializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/DropShadowRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/DynamicRenderable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/EdgeBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/GlowEffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/GuiConsole.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/MirrorSurface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/OcclusionQueryBank.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/SpriteObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/VoxelMesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/WaveEffect.cpp
)

if (NOT EMSCRIPTEN)
    ucm_add_dirs(curllua TO sources_commmon RECURSIVE)
endif()

if (NPLRUNTIME_SERVER)
    set(RenderSystem RenderSystemRHICore RenderSystemNull)
    list(APPEND sources_rendersystem ResourceEmbedded.cpp)
else ()
    set(RenderSystem RenderSystemRHICore RenderSystemNull RenderSystemShaderCompiler)
    ucm_add_dirs(OpenGLWrapper TO sources_rendersystem RECURSIVE)
    list(APPEND NPLRUNTIME_INCLUDES ${PROJECT_SOURCE_DIR}/externals/freetype-2.8.1/include)
    list(APPEND NPLRUNTIME_LIBS freetype)

    if (NPLRUNTIME_ENABLE_OPENGL OR NPLRUNTIME_RENDERER STREQUAL "OPENGL")
        list(APPEND RenderSystem RenderSystemOpenGL)
    endif()
    if (NPLRUNTIME_ENABLE_D3D11 AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/d3d11/CMakeLists.txt")
        list(APPEND RenderSystem RenderSystemD3D11)
    endif()
    if (NPLRUNTIME_ENABLE_D3D12 AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/d3d12/CMakeLists.txt")
        list(APPEND RenderSystem RenderSystemD3D12)
    endif()
    if (NPLRUNTIME_ENABLE_VULKAN AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/vulkan/CMakeLists.txt")
        list(APPEND RenderSystem RenderSystemVulkan)
    endif()
    if (NPLRUNTIME_ENABLE_METAL AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/metal/CMakeLists.txt")
        list(APPEND RenderSystem RenderSystemMetal)
    endif()
    if (NPLRUNTIME_ENABLE_WEBGPU AND EXISTS "${PROJECT_SOURCE_DIR}/RenderSystem/webgpu/CMakeLists.txt")
        list(APPEND RenderSystem RenderSystemWebGPU)
    endif()

    # Unified Embedded Resources for all ParaRHI client backends
    list(APPEND sources_commmon
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/CharacterDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/ChestDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/FruitDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/ItemDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/KidsDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/NpcDatabase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/PetAIDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/PetDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/PuzzleDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/QuestDBProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/StringTable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/TitleDBProvider.cpp
    )

    file(GLOB SHADER_FX_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fx/*.fx
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fx/*.h
    )
    set(EmbeddedResource_Files_RHI)
    foreach(src_file ${SHADER_FX_FILES})
        embed_resources_abs(Resource ${src_file})
        list(APPEND EmbeddedResource_Files_RHI ${Resource})
    endforeach()

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/ParaXmodel.templates)
    list(APPEND EmbeddedResource_Files_RHI ${Resource})

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/cursor.tga)
    list(APPEND EmbeddedResource_Files_RHI ${Resource})

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/paraworld.ico)
    list(APPEND EmbeddedResource_Files_RHI ${Resource})

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/db_create_npc_table.sql)
    list(APPEND EmbeddedResource_Files_RHI ${Resource})

    embed_resources_abs(Resource ${CMAKE_CURRENT_SOURCE_DIR}/res/character_script_template.txt)
    list(APPEND EmbeddedResource_Files_RHI ${Resource})

    list(APPEND EmbeddedResource_Files_RHI ResourceEmbedded.cpp)
    source_group("ResEmbedded" FILES ${EmbeddedResource_Files_RHI})
    list(APPEND sources_rendersystem ${EmbeddedResource_Files_RHI})

    source_group("ShaderFiles" FILES ${SHADER_FX_FILES})
    set_source_files_properties(${SHADER_FX_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
    list(APPEND sources_rendersystem ${SHADER_FX_FILES})

    if (NOT LINUX AND NOT EMSCRIPTEN)
        list(APPEND NPLRUNTIME_LIBS FreeImage)
    endif()
endif ()

list(REMOVE_DUPLICATES RenderSystem)

# remove some compile time warnings
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

if (SDL)
    list(APPEND NPLRUNTIME_INCLUDES ${SDL_ROOT}/include)
endif()

###################################################################
## Target
add_library(ParaEngine ${sources_commmon} ${sources_rendersystem})

# MIDI playback is independent of the host's installed instrument bank.
if (NOT NPLRUNTIME_SERVER AND (NPLRUNTIME_AUDIO OR HARMONY_OS))
    set(midi_dir ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/Midi)
    target_sources(ParaEngine PRIVATE
        ${midi_dir}/MidiSequence.cpp ${midi_dir}/MidiPlayback.cpp ${midi_dir}/MidiOutput.cpp)
    set_property(SOURCE util/MidiMsg.cpp APPEND PROPERTY COMPILE_DEFINITIONS PARAENGINE_MIDI_OUTPUT)
    if (HARMONY_OS)
        target_link_libraries(ParaEngine PUBLIC ohaudio)
    elseif (ANDROID)
        target_link_libraries(ParaEngine PUBLIC OpenSLES)
    elseif (APPLE)
        target_link_libraries(ParaEngine PUBLIC "-framework AudioToolbox")
        if (IOS)
            set_source_files_properties(${midi_dir}/MidiOutput.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
            target_link_libraries(ParaEngine PUBLIC "-framework AVFoundation")
        endif()
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_package(SDL2 REQUIRED CONFIG HINTS ${SDL_ROOT})
        target_link_libraries(ParaEngine PUBLIC SDL2::SDL2)
    endif()
endif()

if(NPLRUNTIME_WEB_ROBOT)
    target_sources(ParaEngine PRIVATE robotics/RobotCoordinateConverter.cpp robotics/MujocoRobotSimulation.cpp)
    target_include_directories(ParaEngine PRIVATE robotics)
    target_compile_definitions(ParaEngine PUBLIC USE_MUJOCO USE_ONNXRUNTIME)
    target_link_options(ParaEngine INTERFACE -msimd128)
    # Preserve constructor-based mesh decoder registration in MuJoCo.
    target_link_libraries(ParaEngine PUBLIC "$<LINK_LIBRARY:WHOLE_ARCHIVE,mujoco>" onnxruntime_web)
elseif(NPL_HAS_DESKTOP_MUJOCO OR NPL_HAS_DESKTOP_ONNXRUNTIME)
    if(NPL_HAS_DESKTOP_MUJOCO)
        target_sources(ParaEngine PRIVATE robotics/RobotCoordinateConverter.cpp robotics/MujocoRobotSimulation.cpp)
        set_source_files_properties(
            robotics/RobotCoordinateConverter.cpp
            robotics/MujocoRobotSimulation.cpp
            ParaScriptBindings/MuJoCoSimulation.cpp
            PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES
        )
        target_include_directories(ParaEngine PRIVATE robotics)
        target_compile_definitions(ParaEngine PUBLIC USE_MUJOCO)
        target_link_libraries(ParaEngine PUBLIC mujoco::mujoco)
    endif()
    if(NPL_HAS_DESKTOP_ONNXRUNTIME)
        target_compile_definitions(ParaEngine PUBLIC USE_ONNXRUNTIME)
        target_link_libraries(ParaEngine PUBLIC onnxruntime::onnxruntime)
    endif()
endif()

set(include_dirs
    ${PROJECT_SOURCE_DIR}
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
    ${NPLRUNTIME_INCLUDES}
)

if (APPLE AND NOT IOS)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/OSX/src)
elseif (ANDROID)
    if (PICO_VR)
        list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/PicoVR/app/cpp)
    endif()
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/AndroidStudio/app/cpp)
elseif (HARMONY_OS)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/HarmonyOS/entry/src/main/cpp)
elseif (EMSCRIPTEN)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/SDL/src)
elseif (WIN32)
    list(APPEND include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../Platform/Windows/src)
endif()

target_include_directories(ParaEngine PUBLIC ${include_dirs})

# boost 头只以系统头方式引入（-isystem）：避免 IDE 的 C++ 引擎把上万个第三方 boost 头
# 当作工程源码做符号索引，导致 "Analyzing project to enable smart features" 长时间卡住。
# 注意：-isystem 目录排在 -I 之后，不改变头文件查找优先级，仅影响编译诊断与 IDE 归类。
if (Boost_INCLUDE_DIRS)
    target_include_directories(ParaEngine SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
endif()

if (ANDROID)
    set(PLATROM_SPEC_DEFINITIONS PARAENGINE_MOBILE ANDROID PLATFORM_ANDROID AUTOUPDATE_PARAENGINE)
    if (PICO_VR)
        list(APPEND PLATROM_SPEC_DEFINITIONS PICO_VR)
    endif()
elseif (HARMONY_OS)
    set(PLATROM_SPEC_DEFINITIONS PARAENGINE_MOBILE HARMONY_OS PLATFORM_HARMONY AUTOUPDATE_PARAENGINE)
elseif (WIN32)
    set(PLATROM_SPEC_DEFINITIONS PLATFORM_WINDOWS _WIN32_WINNT=0x0A00 _WINSOCK_DEPRECATED_NO_WARNINGS)
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

if (NPLRUNTIME_DARKNET)
    list(APPEND NPLRUNTIME_LIBS darknet)
endif()

if (EMSCRIPTEN AND NOT NPLOCE_DEPS_ROOT)
    if (EMSCRIPTEN_SINGLE_THREAD)
        set(NPLOCE_DEPS_ROOT "/mnt/d/workspace/npl/nploce-deps/install")
    else()
        set(NPLOCE_DEPS_ROOT "/mnt/d/workspace/npl/nploce-deps/install_thread")
    endif()
endif()
if (EMSCRIPTEN AND NPLOCE_DEPS_ROOT AND (EXISTS ${NPLOCE_DEPS_ROOT}/oce/lib/libTKBO.a))
    # check if ${NPLOCE_DEPS_ROOT}/oce/lib/libTKBO.a exists    
    add_definitions(-DEMSCRIPTEN_HAS_OCE)
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
    # if (NOT EMSCRIPTEN_SINGLE_THREAD)
    #     list(APPEND NPLRUNTIME_LIBS libcurl)
    # endif()
    
    if(NOT BOOST_LIBRARYDIR)
        set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")
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
    ParaAntlr4
    ${ICONV_LIB_NAME}
    "${BOOST_LIBRARYDIR}/libboost_atomic.a"
    "${BOOST_LIBRARYDIR}/libboost_chrono.a"
    "${BOOST_LIBRARYDIR}/libboost_date_time.a"
    "${BOOST_LIBRARYDIR}/libboost_filesystem.a"
    "${BOOST_LIBRARYDIR}/libboost_serialization.a"
    #"${BOOST_ROOT}/stage/lib/libboost_system.a"
    "${BOOST_LIBRARYDIR}/libboost_thread.a"
    "${BOOST_LIBRARYDIR}/libboost_wserialization.a"
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
        ParaAntlr4
        ${ICONV_LIB_NAME}
    )
endif()

set(PARAENGINE_DEF_LIST)
list(APPEND PARAENGINE_DEF_LIST USE_ICONV TIXML_USE_STL BOOST_SIGNALS_NO_DEPRECATION_WARNING _CRT_SECURE_NO_WARNINGS NPLRUNTIME ${PLATROM_SPEC_DEFINITIONS})
if (NOT NPLRUNTIME_SERVER AND NOT LINUX AND NOT EMSCRIPTEN)
    list(APPEND PARAENGINE_DEF_LIST USE_FREEIMAGE)
endif()
if (SDL OR EMSCRIPTEN)
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
else ()
	if (WIN32)
		list(APPEND PARAENGINE_DEF_LIST PARA_ENABLE_WIN32_IME)
	endif()
	if (NPLRUNTIME_ENABLE_OPENGL OR NPLRUNTIME_RENDERER STREQUAL "OPENGL")
		list(APPEND PARAENGINE_DEF_LIST PARA_ENABLE_OPENGL)
	endif()
endif ()

if (MSVC)
	list(APPEND PARAENGINE_DEF_LIST _HAS_STD_BYTE=0)
endif ()

if (NPLRUNTIME_DARKNET)
    list(APPEND PARAENGINE_DEF_LIST USE_NPL_DARKNET)
endif()

target_compile_definitions(ParaEngine PUBLIC ${PARAENGINE_DEF_LIST})

if (MSVC)
    # Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # enable multiprocessor build option /MP, this will greatly increase compile speed
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override -Wno-missing-template-arg-list-after-template-kw")
endif ()  
