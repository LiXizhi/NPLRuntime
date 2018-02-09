#include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
#set(CMAKE_SHARED_LINKER_FLAGS
 #   "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#add_subdirectory(native_app_glue)



#ucm_add_dirs(res TO android_res RECURSIVE ADDITIONAL_EXT xml)
#add_executable(ParaCraft AndroidManifest.xml main.cpp AndroidApplication.cpp AndroidApplication.h ${android_res})

#set_target_properties(ParaCraft PROPERTIES 
#    ANDROID_GUI TRUE
#    ANDROID_API 19
#)

# add lib dependencies
#target_link_libraries(ParaCraft PUBLIC
#    android
#    native_app_glue
#    EGL
#    GLESv2
#    log
#)

add_subdirectory(tegra)