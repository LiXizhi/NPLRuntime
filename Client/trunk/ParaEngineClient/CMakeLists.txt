if(WIN32)
    if(ANDROID) 
        set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
        set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)
    else()
        set(NPLRUNTIME_RENDERER "DIRECTX" CACHE STRING "Render API")
        set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS DIRECTX OPENGL)
    endif()
endif(WIN32)

set(NPLRUNTIME_RENDERER "OPENGL" CACHE STRING "Render API")
set_property(CACHE NPLRUNTIME_RENDERER PROPERTY STRINGS OPENGL)



add_subdirectory(EmbedResource)
add_subdirectory(ParaEngine)
add_subdirectory(RenderSystem)
add_subdirectory(Platform)