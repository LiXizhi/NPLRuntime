include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)
ucm_add_dirs(src TO app_src RECURSIVE)



add_executable(Paracraft
    ${app_src}
    ${android_res}
    AndroidManifest.xml
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    native_app_glue
    android
    EGL
    log
)
target_compile_definitions(Paracraft PUBLIC 
    PARAENGINE_MOBILE
    PLATFORM_ANDROID
    STATIC_PLUGIN_CAUDIOENGINE
    AUTOUPDATE_PARAENGINE
)
set_target_properties(Paracraft PROPERTIES 
    ANDROID_GUI TRUE
    ANDROID_NATIVE_LIB_DEPENDENCIES "sqlite.so"
    ANDROID_ASSETS_DIRECTORIES ${CMAKE_BINARY_DIR}/assets
)

