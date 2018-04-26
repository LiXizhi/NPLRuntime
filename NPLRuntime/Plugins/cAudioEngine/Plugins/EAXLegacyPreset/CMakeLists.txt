#-------------------------------------------------------------------
# This file is part of the CMake build system for cAudio
#-------------------------------------------------------------------

############################################################
# EAXLegacyPreset library
############################################################

PROJECT(EAXLegacyPreset)

if(WIN32)
	if(${CAUDIO_STATIC})
		ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB)
	endif()
endif() 

include_directories (include ${CAUDIO_INCLUDE_DIR} )
include(EAXLegacyPreset_src_files.cmake)
add_library(EAXLegacyPreset ${CAUDIO_LIB_TYPE} ${file_root})

target_link_libraries(EAXLegacyPreset cAudio)
add_dependencies(EAXLegacyPreset cAudio)

if(${CAUDIO_STATIC})
	ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB)
endif()

install_all_targets(EAXLegacyPreset)