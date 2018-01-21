set(NPLRUNTIME_RENDERER "DIRECTX" CACHE STRING "Render API")
set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL)

add_subdirectory(EmbedResource)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem/d3d9)
add_subdirectory(Platform)