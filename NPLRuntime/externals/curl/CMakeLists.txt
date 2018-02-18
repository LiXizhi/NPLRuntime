add_library(libcurl INTERFACE)

if(ANDROID)
    target_include_directories(libcurl INTERFACE include/android)
    target_link_libraries(libcurl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/${CMAKE_ANDROID_ARCH_ABI}/libcurl.a para_libssl para_libcrypto)
elseif(WIN32)
    target_include_directories(libcurl INTERFACE include/win32)
    target_link_libraries(libcurl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/win32/libcurl.lib)
endif()
