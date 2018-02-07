
if(ANDROID) 
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)
elseif(WIN32)
    set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
    set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL)
endif()

add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)