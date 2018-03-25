include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
ucm_add_dirs(src TO OSX_SRC ADDITIONAL_EXT mm m)

add_executable(Paracraft ${OSX_SRC})

find_library(COCOA_LIBRARY Cocoa required)
FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
target_link_libraries(Paracraft PRIVATE
RenderSystemOpenGL
ParaEngine
${COCOA_LIBRARY}
${OPENGL_LIBRARY}
)

set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")


