set (file_root_source_files
	src/main.cpp
)
set (file_root_header_files
	include/cEAXLegacyPresetPlugin.h
)

source_group("Source Files" FILES ${file_root_source_files})
source_group("Header Files" FILES ${file_root_header_files})


set (file_root
	${file_root_source_files}
	${file_root_header_files}
)

