include(${PROJECT_SOURCE_DIR}/cmake/FindDirectX.cmake)


add_library(RenderSystemD3D9 
    D3DMapping.cpp
    D3DMapping.h
    RenderDeviceD3D9.cpp
    RenderDeviceD3D9.h
    TextureD3D9.h
    TextureD3D9.cpp
    EffectD3D9.h
    EffectD3D9.cpp
)

#d3dx9 d3dxof dxguid dinput8 dxerr x3daudio
find_library(d3d9 d3d9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(d3dx9 d3dx9 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(d3dxof d3dxof ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(dxguid dxguid ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(dinput8 dinput8 ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(dxerr dxerr ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(x3daudio x3daudio ${DirectX_LIBRARY_PATHS} NO_DEFAULT_PATH)
target_link_libraries(RenderSystemD3D9 PUBLIC
    "${d3d9}"
    "${d3dx9}"
    "${d3dxof}"
    "${dxguid}"
    "${dinput8}"
    "${dxerr}"
    "${x3daudio}"
)

target_include_directories(RenderSystemD3D9 PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngine/
    ${DirectX_INCLUDE_DIRS}
    /
)


target_compile_definitions(RenderSystemD3D9 PUBLIC USE_DIRECTX_RENDERER PARAENGINE_CLIENT)
