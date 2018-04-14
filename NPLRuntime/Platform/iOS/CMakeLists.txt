

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


ucm_add_dirs(src TO app_src RECURSIVE ADDITIONAL_EXT mm m)
ucm_add_dirs(res TO RESOURCE_FILES RECURSIVE ADDITIONAL_EXT *)

set_source_files_properties(${RESOURCE_FILES} PROPERTICES MACOSX_PACKAGE_LOCATION Resources)


find_library(UIKit UIKit)
find_library(OpenGLES OpenGLES)
find_library(QuartzCore QuartzCore)

add_executable(Paracraft MACOSX_BUNDLE ${app_src})
target_link_libraries(Paracraft PRIVATE 
    ${UIKit} ${OpenGLES} ${QuartzCore}
    RenderSystemOpenGL
    ParaEngine
)




set_target_properties(Paracraft PROPERTIES 
MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/res/info.plist
RESOURCE "${RESOURCE_FILES}"
)



