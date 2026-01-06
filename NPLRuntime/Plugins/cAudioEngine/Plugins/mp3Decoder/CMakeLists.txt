#-------------------------------------------------------------------
# This file is part of the CMake build system for cAudio
#-------------------------------------------------------------------

############################################################
# cAp_mp3Decoder library
############################################################

PROJECT(cAp_mp3Decoder)

option(NPLRUNTIME_USE_MINI_MP3 "Use minimp3 or mpaudec to decode" ON)

if(WIN32)
	if(${CAUDIO_STATIC})
		ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB)
	endif()
endif() 

include_directories (include ${CAUDIO_INCLUDE_DIR} )
include(mp3Decoder_src_files.cmake)
add_library(cAp_mp3Decoder STATIC ${file_root})

target_link_libraries(cAp_mp3Decoder cAudio)

if(${CAUDIO_STATIC})
	ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB)
endif()

if (NPLRUNTIME_USE_MINI_MP3)
	target_compile_definitions(cAp_mp3Decoder PUBLIC USE_MINI_MP3)
endif()

set_property(TARGET cAp_mp3Decoder PROPERTY VERSION "2.3.0")
set_property(TARGET cAp_mp3Decoder PROPERTY SOVERSION 2 )

install_all_targets(cAp_mp3Decoder)

IF(MSVC)
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
			
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()