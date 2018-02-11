add_library(native_app_glue STATIC 
    android_native_app_glue.c
    android_native_app_glue.h
)
target_include_directories(native_app_glue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
