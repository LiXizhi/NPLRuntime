add_library(RenderSystemD3D12
    RenderContextD3D12.h
    RenderContextD3D12.cpp
    RenderDeviceD3D12.h
    RenderDeviceD3D12.cpp
    BlockSkyD3D12.cpp
)

# Build-time DXIL embedding keeps the game independent of the installed DXC version.
find_program(BLOCK_SKY_DXC dxc HINTS
    "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64"
    "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64")
if(BLOCK_SKY_DXC)
    set(sky_dxil "${CMAKE_CURRENT_BINARY_DIR}/BlockSkyDXIL.h")
    add_custom_command(OUTPUT "${sky_dxil}"
        COMMAND "${BLOCK_SKY_DXC}" -T cs_6_5 -E main -O3 -Fh "${sky_dxil}" -Vn g_blockSkyDXIL "${CMAKE_CURRENT_SOURCE_DIR}/BlockSky.hlsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/BlockSky.hlsl" VERBATIM)
    set(reflection_dxil "${CMAKE_CURRENT_BINARY_DIR}/BlockReflectionDXIL.h")
    add_custom_command(OUTPUT "${reflection_dxil}"
        COMMAND "${BLOCK_SKY_DXC}" -T cs_6_5 -E main -O3 -Fh "${reflection_dxil}" -Vn g_blockReflectionDXIL "${CMAKE_CURRENT_SOURCE_DIR}/BlockReflection.hlsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/BlockReflection.hlsl" VERBATIM)
    target_sources(RenderSystemD3D12 PRIVATE "${sky_dxil}" "${reflection_dxil}")
    target_compile_definitions(RenderSystemD3D12 PRIVATE PE_BLOCK_SKY_DXR=1)
    target_include_directories(RenderSystemD3D12 PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endif()

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
