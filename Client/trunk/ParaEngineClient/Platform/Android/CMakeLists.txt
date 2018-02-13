include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)


set(APP_SRC 
    main.cpp
    ParacraftApp.cpp
    ParacraftApp.h
    AppDelegate.h
    AppDelegate.cpp
    AndroidManifest.xml
    ${android_res}
)
source_group(src FILES ${APP_SRC})


add_executable(Paracaft
    ${APP_SRC}
)
target_link_libraries(Paracaft PUBLIC
    ParaEngine
    RenderSystemOpenGL
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
)

