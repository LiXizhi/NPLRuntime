add_library(Freetype2 INTERFACE)

target_include_directories(Freetype2 INTERFACE include/win32)

target_link_libraries(Freetype2 INTERFACE prebuild/win32/freetype250.lib)