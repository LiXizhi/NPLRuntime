include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)
add_subdirectory(native_app_glue)


ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml png)

add_executable(Paracaft
    main.cpp
    ParacraftApp.cpp
    ParacraftApp.h
    AndroidApplication.h
    AndroidApplication.cpp
    AndroidManifest.xml
    ${android_res}
)
target_link_libraries(Paracaft PUBLIC 
    ParaEngine
    native_app_glue
    android
    EGL
    GLESv2
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
)

