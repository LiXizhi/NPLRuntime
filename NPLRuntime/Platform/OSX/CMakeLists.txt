include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

ucm_add_dirs(src TO OSX_SRC ADDITIONAL_EXT mm m)

ucm_add_dirs(res TO RESOURCE_FILES ADDITIONAL_EXT *)

set_source_files_properties(${RESOURCE_FILES} PROPERTICES MACOSX_PACKAGE_LOCATION Resources)

add_executable(Paracraft ${OSX_SRC} ${RESOURCE_FILES})
set_target_properties(Paracraft PROPERTIES 
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE icon.icns
    RESOURCE "${RESOURCE_FILES}"
    )

find_library(COCOA_LIBRARY Cocoa required)
FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
target_link_libraries(Paracraft PRIVATE
    RenderSystemOpenGL
    ParaEngine
    ${COCOA_LIBRARY}
    ${OPENGL_LIBRARY}
)

set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")


#add_custom_command(TARGET Paracraft
#    POST_BUILD
#    COMMAND "echo fuck!"
#)