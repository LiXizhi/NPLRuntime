include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)
ucm_add_dirs(src TO app_src RECURSIVE)
ucm_add_dirs(java TO app_java RECURSIVE ADDITIONAL_EXT java)


add_executable(Paracraft
    ${app_src}
    ${android_res}
	${app_java}
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
    cAudioEngine
)
target_compile_definitions(Paracraft PUBLIC 
    PARAENGINE_MOBILE
    PLATFORM_ANDROID
    STATIC_PLUGIN_CAUDIOENGINE
    AUTOUPDATE_PARAENGINE
	OPENAL_STATIC
)


set_target_properties(Paracraft PROPERTIES 
    ANDROID_GUI TRUE
    ## ANDROID_NATIVE_LIB_DEPENDENCIES "ParaSqlite.so"
    ANDROID_ASSETS_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/assets
	ANDROID_JAVA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/java
	ANDROID_NATIVE_LIB_DIRECTORIES ${OUTPUT_BIN_DIR}
)


