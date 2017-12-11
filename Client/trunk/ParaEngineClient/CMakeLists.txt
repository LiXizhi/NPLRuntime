
include(${PROJECT_SOURCE_DIR}/cmake/FindBoost.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/FindDirectX.cmake)

# Source
aux_source_directory(util src_util)
#source_group("util" FILES ${src_util})

aux_source_directory(math src_math)
aux_source_directory(core src_core)
aux_source_directory(renderer src_renderer)
aux_source_directory(2dengine src_2dengine)
aux_source_directory(3dengine src_3dengine)
aux_source_directory(io src_io)
aux_source_directory(npl src_npl)
aux_source_directory(parascriptbindings src_parascriptbindings)
aux_source_directory(sqlite3 src_sqlite3)
aux_source_directory(dir_monitor src_dir_monitor)
aux_source_directory(bmaxmodel src_bmaxmodel)
aux_source_directory(protocol src_protocol)

aux_source_directory(paraxmodel src_paraxmodel)
aux_source_directory(curllua src_curllua)
aux_source_directory(debugtools src_debugtools)
aux_source_directory(ic src_ic)
aux_source_directory(terrain src_terrain)
aux_source_directory(blockengine src_blockengine)
aux_source_directory(websocket src_websocket)
aux_source_directory(PaintEngine src_paintengine)
aux_source_directory(CadModel src_carmodel)

# For direct3d renderer
aux_source_directory(engine src_engine)

aux_source_directory(voxelmesh src_voxelmesh)
aux_source_directory(common src_common) #DirextX



add_executable(ParaEngineCore
    ${src_util}
    ${src_math}
    ${src_core}
    ${src_renderer}
    ${src_2dengine}
    ${src_3dengine}
    ${src_io}
    ${src_npl}
    ${src_parascriptbindings}
    ${src_sqlite3}
    ${src_dir_monitor}
    ${src_bmaxmodel}
    ${src_protocol}
    ${src_paraxmodel}
    ${src_curllua}
    ${src_debugtools}
    ${src_ic}
    ${src_terrain}
    ${src_blockengine}
    ${src_websocket}
    ${src_engine}
    ${src_voxelmesh}
    ${src_common}
    ${src_paintengine}
    ${src_carmodel}
    
)

target_include_directories(ParaEngineCore PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/Framework/Interface/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/math/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/core/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/util/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/2dengine/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/3dengine/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/renderer/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/io/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/npl/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/sqlite3/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/parascriptbindings/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/dir_monitor/
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/bmaxmodel
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/protocol
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/paraxmodel
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/curllua
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/debugtools
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/ic
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/terrain
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/websocket
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/blockengine
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/engine
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/voxelmesh
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/common
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/PaintEngine
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/CadModel

    ${Boost_INCLUDE_DIR}/
    ${DirectX_INCLUDE_DIRS}
)

link_directories(ParaEngineCore ${Boost_LIBRARY_DIRS} ${DirectX_LIBRARY_PATHS})
target_link_libraries(ParaEngineCore libcurl tinyxml tinyxpath liblua luabind zlib jsoncpp  ${Boost_LIBRARIES} ${DirectX_LIBRARIES})
target_compile_definitions(ParaEngineCore 
PUBLIC PE_CORE_EXPORTING
PUBLIC NPLRUNTIME
PUBLIC SQLITE_STATICLIB
PUBLIC USE_DIRECTX_RENDERER
PUBLIC _WIN32_WINNT=0x0501
PUBLIC PARAENGINE_CLIENT
PUBLIC BOOST_SIGNALS_NO_DEPRECATION_WARNING
PUBLIC PLATFORM_WINDOWS
PUBLIC USE_WINDOWS_SSPI
)


IF(MSVC)
# statically link MSVC to reduce dependancies
foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
    if(${flag_var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MDd")
endforeach(flag_var)

# /GR- Remove RTTI to miminize the executable size
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")

# enable multiprocessor build option /MP, this will greatly increase compile speed
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
ENDIF()