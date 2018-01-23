add_library(libpng INTERFACE)
target_include_directories(libpng INTERFACE include/win32)
target_link_libraries(libpng INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/prebuild/win32/libpng.lib)