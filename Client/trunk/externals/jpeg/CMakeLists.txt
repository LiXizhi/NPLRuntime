add_library(libjpeg INTERFACE)

target_include_directories(libjpeg INTERFACE include/win32)

target_link_libraries(libjpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuild/win32/libjpeg.lib)