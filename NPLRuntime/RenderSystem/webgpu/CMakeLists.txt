add_library(RenderSystemWebGPU
    webgpu_c_api.h
    WebGPULoader.h
    WebGPULoader.cpp
    RenderContextWebGPU.h
    RenderContextWebGPU.cpp
    RenderDeviceWebGPU.h
    RenderDeviceWebGPU.cpp
)

target_include_directories(RenderSystemWebGPU PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${PROJECT_SOURCE_DIR}/RenderSystem/shader_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderSystemWebGPU
    PUBLIC RenderSystemRHICore ParaEngine
)

if(WIN32)
    target_compile_definitions(RenderSystemWebGPU PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _HAS_STD_BYTE=0
    )
elseif(EMSCRIPTEN)
    target_compile_options(RenderSystemWebGPU PUBLIC -sUSE_WEBGPU=1)
    target_link_options(RenderSystemWebGPU PUBLIC -sUSE_WEBGPU=1)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(RenderSystemWebGPU PRIVATE dl)
endif()
