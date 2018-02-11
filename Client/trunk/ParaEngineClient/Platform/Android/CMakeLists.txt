include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)

add_executable(Paracaft main.cpp AndroidApplication.h AndroidApplication.cpp  AndroidManifest.xml ${android_res})
target_link_libraries(Paracaft PUBLIC 
    native_app_glue
    android
    EGL
    GLESv2
    log
)

set_target_properties(Paracaft PROPERTIES 
    ANDROID_GUI TRUE
    ANDROID_API 19
)

