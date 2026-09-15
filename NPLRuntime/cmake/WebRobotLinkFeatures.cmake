# Emscripten 3.1.59 supports the linker flags but does not advertise this CMake
# feature. Preserve the SDK's definition whenever it already supplies one.
if(EMSCRIPTEN AND NOT DEFINED CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED
   AND NOT DEFINED CMAKE_CXX_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED)
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE
        "-Wl,--whole-archive" "<LINK_ITEM>" "-Wl,--no-whole-archive")
    set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED TRUE)
endif()
