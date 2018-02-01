include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


ucm_add_dirs(GLWarpper TO sources_warpper)

add_library(RenderSystemOpenGL 
    RenderDeviceOpenGL.cpp
    RenderDeviceOpenGL.h
    ${sources_warpper}
)



target_include_directories(RenderSystemOpenGL PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngineClient/ParaEngine/
    ${PROJECT_SOURCE_DIR}/externals/lpng1634/
    ${PROJECT_SOURCE_DIR}/externals/freetype-2.8.1/include/
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderSystemOpenGL PUBLIC 
    glad_core
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