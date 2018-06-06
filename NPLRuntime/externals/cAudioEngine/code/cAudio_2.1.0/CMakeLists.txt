# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.6.26
# Desc: ParaEngine plugin wrapper for the cAudio engine(which in turn is a wrapper of OpenAL, Vorbis Ogg, MP3 and Wav)
# Dependencies: dependencies\Libs\dlls\OpenAL32.dll(108KB) wrap_oal.dll(404KB)
# CMake Note: all source locations are same as the cAudio official release, except that I added the folder ./ParaEngineWrapper, and I also included the MP3 and Presets plugins in the main project.
# Plus, I statically linked with msvc, hence the only external dependencies are the OpenAL files, which can be put under the working directory (if not under SYSTEM dir).
# The main reason that I switched to cAudio is that the orignal Xact Engine does not allow us to distribute dll side by side, making it impossible to distribute via web browser plugin (xcopy install). 
#	- MP3 and Preset plugins are compiled to core dll instead of dynamic dll. 
#	- cAudioDefines.h removed CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT, CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER, CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER

cmake_minimum_required(VERSION 2.6)

project (cAudioEngine)

SET(cAudioEngine_SRCS
	changes.txt
	ReadMe.txt
	License.txt
)

file (GLOB cAudioEngine_Source_SRCS ${PROJECT_SOURCE_DIR}/Source/*.cpp)
SOURCE_GROUP("Source" FILES ${cAudioEngine_Source_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_Source_SRCS})

file (GLOB cAudioEngine_include_SRCS ${PROJECT_SOURCE_DIR}/include/*.h)
SOURCE_GROUP("include" FILES ${cAudioEngine_include_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_include_SRCS})

file (GLOB cAudioEngine_headers_SRCS ${PROJECT_SOURCE_DIR}/headers/*.h)
SOURCE_GROUP("Headers" FILES ${cAudioEngine_headers_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_headers_SRCS})

file (GLOB cAudioEngine_PEWrapper_SRCS ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.cpp ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.h ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.def)
SOURCE_GROUP("ParaEngine Wrapper" FILES ${cAudioEngine_PEWrapper_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_PEWrapper_SRCS})

file (GLOB cAudioEngine_PluginMP3_SRCS ${PROJECT_SOURCE_DIR}/plugins/mp3Decoder/cMP3*.* ${PROJECT_SOURCE_DIR}/plugins/mp3Decoder/mpaudec/*.c ${PROJECT_SOURCE_DIR}/plugins/mp3Decoder/mpaudec/*.h)
SOURCE_GROUP("Plugin MP3" FILES ${cAudioEngine_PluginMP3_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_PluginMP3_SRCS})

file (GLOB cAudioEngine_PluginPresets_SRCS ${PROJECT_SOURCE_DIR}/plugins/EAXLegacyPreset/cEAXLegacyPresetPlugin.h)
SOURCE_GROUP("Plugin Presets" FILES ${cAudioEngine_PluginPresets_SRCS})
list(APPEND cAudioEngine_SRCS ${cAudioEngine_PluginPresets_SRCS})


set(NPLRUNTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../ParaEngine)


# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/Headers")
include_directories("${PROJECT_SOURCE_DIR}/dependencies/Headers")
include_directories("${PROJECT_SOURCE_DIR}/dependencies/Headers/al")
include_directories("${NPLRUNTIME_ROOT}")
include_directories("${NPLRUNTIME_ROOT}/Core")
include_directories("${NPLRUNTIME_ROOT}/NPL")
include_directories("${NPLRUNTIME_ROOT}/util")
include_directories("${NPLRUNTIME_ROOT}/Framework/Interface")

if(WIN32)
	link_directories("${PROJECT_SOURCE_DIR}/dependencies/Libs/windows")
elseif(IOS)
	link_directories("${PROJECT_SOURCE_DIR}/dependencies/Libs/Osx-Iphone")
elseif(APPLE)
	link_directories("${PROJECT_SOURCE_DIR}/dependencies/Libs/Osx")
else
	link_directories("${PROJECT_SOURCE_DIR}/dependencies/Libs/linux")
endif()

if(WIN32)
	SET(CMAKE_DEBUG_POSTFIX "_d")
endif

link_libraries(cAudioEngine vorbis_static.lib vorbisfile_static.lib ogg_static.lib OpenAL32.lib vorbisenc_static.lib)

# add the executable
add_library(cAudioEngine SHARED ${cAudioEngine_SRCS})

IF(MSVC)
	# statically link MSVC to reduce dependancies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	 	if(${flag_var} MATCHES "/MDd")
	 		string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MDd")
	endforeach(flag_var)
			
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

if(MSVC)
	get_target_property(DEBUG_EXE_PATH cAudioEngine DEBUG_LOCATION)
	STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" DEBUG_EXE_NAME "${DEBUG_EXE_PATH}" )

	get_target_property(RELEASE_EXE_PATH cAudioEngine RELEASE_LOCATION)
	STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" RELEASE_EXE_NAME "${RELEASE_EXE_PATH}" )
endif(MSVC)

ADD_CUSTOM_COMMAND(
	TARGET cAudioEngine
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cAudioEngine> ${OUTPUT_BIN_DIR}
)