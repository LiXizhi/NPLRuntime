include(${PROJECT_SOURCE_DIR}/cmake/ConfigDirectX.cmake)
add_library(RenderSystemD3D9 
    D3D9Tools.cpp
    D3D9Tools.h
    RenderDeviceD3D9.cpp
    RenderDeviceD3D9.h
    TextureD3D9.h
    TextureD3D9.cpp
)
target_include_directories(RenderSystemD3D9 PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/
    ${DirectX_INCLUDE_DIRS}
    /
)

target_link_libraries(RenderSystemD3D9 d3d9.lib)