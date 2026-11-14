add_library(RenderSystemD3D11
    RenderContextD3D11.h
    RenderContextD3D11.cpp
    RenderDeviceD3D11.h
    RenderDeviceD3D11.cpp
)

target_include_directories(RenderSystemD3D11 PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderSystemD3D11
    PUBLIC RenderSystemRHICore ParaEngine
    PRIVATE d3d11 dxgi d3dcompiler dxguid
)

target_compile_definitions(RenderSystemD3D11 PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _HAS_STD_BYTE=0
)
