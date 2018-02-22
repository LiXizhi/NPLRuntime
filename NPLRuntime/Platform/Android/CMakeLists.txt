include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)
ucm_add_dirs(src TO app_src RECURSIVE)



add_executable(Paracaft
    ${app_src}
    ${android_res}
    AndroidManifest.xml
)
target_link_libraries(Paracaft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    native_app_glue
    android
    EGL
    log
)

target_compile_definitions(Paracaft PUBLIC 
    PARAENGINE_MOBILE
    PLATFORM_ANDROID
    STATIC_PLUGIN_CAUDIOENGINE
    AUTOUPDATE_PARAENGINE
)

set_target_properties(Paracaft PROPERTIES 
    ANDROID_GUI TRUE
    ANDROID_NATIVE_LIB_DEPENDENCIES "sqlite.so"
    ANDROID_ASSETS_DIRECTORIES ${CMAKE_BINARY_DIR}/assets
)

