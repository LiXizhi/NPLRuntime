add_library(RenderSystemD3D12
    RenderContextD3D12.h
    RenderContextD3D12.cpp
    RenderDeviceD3D12.h
    RenderDeviceD3D12.cpp
)

target_include_directories(RenderSystemD3D12 PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderSystemD3D12
    PUBLIC RenderSystemRHICore ParaEngine
    PRIVATE dxguid
)

target_compile_definitions(RenderSystemD3D12 PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _HAS_STD_BYTE=0
)
