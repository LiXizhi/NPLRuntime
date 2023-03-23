include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

if(ANDROID)
    set(LIBGL GLESv2 EGL)
    set(PLATFORM_SRC gl_android_spec.c gl_android_spec.h)
elseif(IOS)
    find_library(OPENGL_LIBRARY OpenGLES)
    set(LIBGL ${OpenGL})
    set(PLATFORM_SRC gl_ios_spec.c gl_ios_spec.h)
elseif(EMSCRIPTEN)
    set(PLATFORM_SRC gl_glad_spec.c)
elseif(WIN32 OR LINUX)
    set(LIBGL glad_core)
    set(PLATFORM_SRC gl_glad_spec.c)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    set(LIBGL ${OpenGL})
    set(PLATFORM_SRC gl_osx_spec.c gl_osx_spec.h)
endif()

add_library(RenderSystemOpenGL 
    RenderDeviceOpenGL.cpp
    RenderDeviceOpenGL.h
    OpenGL.h
    ${PLATFORM_SRC}
)


target_include_directories(RenderSystemOpenGL PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/
    ${PROJECT_SOURCE_DIR}/externals/freetype-2.8.1/include/
    ${CMAKE_CURRENT_SOURCE_DIR}
)


target_link_libraries(RenderSystemOpenGL PUBLIC 
    ${LIBGL}
    ParaEngine
    freetype
    GLSLCodeGen
    fxParser
)
target_compile_definitions(RenderSystemOpenGL PUBLIC USE_OPENGL_RENDERER)
