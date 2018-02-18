add_library(libjpeg INTERFACE)

if(ANDROID)
    target_include_directories(libjpeg INTERFACE include/android)
    target_link_libraries(libjpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuild/android/${CMAKE_ANDROID_ARCH_ABI}/libjpeg.a)
elseif(WIN32)
    target_include_directories(libjpeg INTERFACE include/win32)
    target_link_libraries(libjpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuild/win32/libjpeg.lib)
endif()


