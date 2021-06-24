PROJECT(cAudioCSharpWrapper)
set(CMAKE_DEBUG_POSTFIX)

include_directories (include ${CAUDIO_INCLUDE_DIR})

include (cAudioCSharpWrapper_src_files.cmake)

add_library(cAudioCSharpWrapper SHARED ${file_root})
target_link_libraries(cAudioCSharpWrapper cAudio)
add_dependencies(cAudioCSharpWrapper cAudio)

install_all_targets(cAudioCSharpWrapper)
