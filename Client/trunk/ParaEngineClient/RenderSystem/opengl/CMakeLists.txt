include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


ucm_add_dirs(GLWarpper TO sources_warpper)

add_library(RenderSystemOpenGL 
    RenderDeviceOpenGL.cpp
    RenderDeviceOpenGL.h
    ${sources_warpper}
)



target_include_directories(RenderSystemOpenGL PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/
    /
)

target_link_libraries(RenderSystemOpenGL PUBLIC 
    glad_core
    iconv
    Freetype2
    libjpeg
    libpng
)
target_compile_definitions(RenderSystemOpenGL PUBLIC USE_OPENGL_RENDERER)