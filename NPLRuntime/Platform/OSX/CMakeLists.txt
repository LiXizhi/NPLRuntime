include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
ucm_add_dirs(src TO OSX_SRC ADDITIONAL_EXT mm m)

add_executable(Paracraft MACOSX_BUNDLE ${OSX_SRC})

find_library(COCOA_LIBRARY Cocoa required)
FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
target_link_libraries(Paracraft PRIVATE
RenderSystemOpenGL
ParaEngine
${COCOA_LIBRARY}
${OPENGL_LIBRARY}
)
