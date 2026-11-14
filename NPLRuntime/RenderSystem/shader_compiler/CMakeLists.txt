add_library(RenderSystemShaderCompiler STATIC
    FxShaderTypes.h
    FxParserV2.h
    FxParserV2.cpp
    AstTextCodeGenCommon.h
    SpirvCodeGen.h
    SpirvCodeGen.cpp
    MslCodeGen.h
    MslCodeGen.cpp
    WgslCodeGen.h
    WgslCodeGen.cpp
    FxCompiler.h
    FxCompiler.cpp
)

target_include_directories(RenderSystemShaderCompiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/RenderSystem
    ${PROJECT_SOURCE_DIR}/externals/hlsl2glslfork/include
    ${PROJECT_SOURCE_DIR}/externals/hlsl2glslfork/hlslang/Include
    ${PROJECT_SOURCE_DIR}/externals/hlsl2glslfork/hlslang/MachineIndependent
    ${PROJECT_SOURCE_DIR}/externals/hlsl2glslfork/hlslang/GLSLCodeGen
)

target_link_libraries(RenderSystemShaderCompiler PUBLIC
    RenderSystemRHICore
    GLSLCodeGen
)

if (WIN32)
    target_link_libraries(RenderSystemShaderCompiler PUBLIC dxguid)
endif()

if (NOT ANDROID AND NOT IOS AND NOT HARMONY_OS AND NOT EMSCRIPTEN AND NOT PARAENGINE_CLIENT_DLL)
    add_executable(parafxc
        parafxc_main.cpp
    )
    target_link_libraries(parafxc PRIVATE RenderSystemShaderCompiler)
endif()
