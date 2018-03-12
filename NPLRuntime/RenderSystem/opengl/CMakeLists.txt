include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


if(ANDROID)
    set(LIBGL GLESv2 EGL)
    set(PLATFORM_SRC gl_android_spec.c gl_android_spec.h)
elseif(WIN32)
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
    libjpeg
    libpng
    GLSLCodeGen
    fxParser
)
target_compile_definitions(RenderSystemOpenGL PUBLIC USE_OPENGL_RENDERER)


if(MSVC)
    ucm_set_runtime(DYNAMIC)
    # enable multiprocessor build option /MP, this will greatly increase compile speed
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")
endif()