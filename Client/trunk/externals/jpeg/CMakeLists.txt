add_library(libjpeg INTERFACE)

target_include_directories(libjpeg INTERFACE include/win32)

target_link_libraries(libjpeg INTERFACE prebuild/win32/libjpeg.lib)