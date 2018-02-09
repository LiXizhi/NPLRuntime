cmake_minimum_required(VERSION 3.10)
project(TegraWithCmake)

add_subdirectory(native_app_glue)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
#set(CMAKE_SHARED_LINKER_FLAGS
 #   "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#add_library(native-activity SHARED jni/main.cpp)
#target_link_libraries(native-activity PUBLIC native_app_glue)



add_executable(testegra jni/main.cpp AndroidManifest.xml res/values/strings.xml)
target_link_libraries(testegra PUBLIC 
    native_app_glue
    android
    EGL
    GLESv2
    log
)

set_target_properties(testegra PROPERTIES 
    ANDROID_GUI TRUE
    ANDROID_API 19
)

