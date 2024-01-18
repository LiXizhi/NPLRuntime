PROJECT(cAudioEngine)

option(NPLRUNTIME_USE_MINI_MP3 "Use minimp3 or mpaudec to decode" ON)

if (AUDIO_STATICLIB)
	add_definitions(-DAUDIO_STATICLIB)
endif()
	
if(CAUDIO_STATIC)
	ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB)
endif()
if(BUILD_ON_ANDROID)
    add_definitions(-DANDROID)
    add_definitions(-DSTATIC_LIBRARY)
endif()
# generate cAudioBuildSettings.h 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../CMake/Templates/cAudioBuildSettings.h.in ${CMAKE_BINARY_DIR}/include/cAudioBuildSettings.h @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/include/cAudioBuildSettings.h DESTINATION include/cAudio)
INSTALL(DIRECTORY include/ DESTINATION include/cAudio FILES_MATCHING PATTERN "*.h")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(CAUDIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include PARENT_SCOPE)

include(cAudio_src_files.cmake)		
add_library(cAudioEngine ${CAUDIO_LIB_TYPE} ${file_root})

if (NPLRUNTIME_USE_MINI_MP3)
	target_compile_definitions(cAudioEngine PUBLIC USE_MINI_MP3)
endif()

if(CAUDIO_ENABLE_OGG)
	include_directories (include Headers ${CMAKE_BINARY_DIR}/include ${OPENAL_INCLUDE_DIRS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
	target_link_libraries(cAudioEngine Vorbis Ogg)
	add_dependencies(cAudioEngine Vorbis Ogg)
else()
	include_directories (include Headers ${CMAKE_BINARY_DIR}/include ${OPENAL_INCLUDE_DIRS})
	target_link_libraries(cAudioEngine)
	add_dependencies(cAudioEngine)
endif()

if (APPLE AND CAUDIO_IOS_BUILD)
	find_library(OpenAL OpenAL)
	target_link_libraries(cAudioEngine ${OpenAL})
elseif (EMSCRIPTEN)
	target_link_libraries(cAudioEngine openal)
	ADD_DEFINITIONS(-DEMSCRIPTEN)
	if (EMSCRIPTEN_SINGLE_THREAD)
		ADD_DEFINITIONS(-DEMSCRIPTEN_SINGLE_THREAD)
	endif()
else()
	target_link_libraries(cAudioEngine OpenAL)
	add_dependencies(cAudioEngine OpenAL)
endif()

if (APPLE AND CAUDIO_IOS_BUILD)
	set_target_properties(cAudioEngine PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
	set_target_properties(cAudioEngine PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
	set_target_properties(cAudioEngine PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
endif()

install_all_targets(cAudioEngine)

if(WIN32)
	SET(CMAKE_DEBUG_POSTFIX "_d")
	target_compile_definitions(cAudioEngine PUBLIC  AL_LIBTYPE_STATIC)
endif()

####################################################
# ParaEngine related 
# ParaEngineClient_SOURCE_DIR can be used to specify a single base
# folder where the required dependencies may be found.

set(ParaEngineClient_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../ParaEngine)
include_directories("${ParaEngineClient_SOURCE_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/Core")
include_directories("${ParaEngineClient_SOURCE_DIR}/NPL")
include_directories("${ParaEngineClient_SOURCE_DIR}/Framework/Interface")
include_directories("${PROJECT_SOURCE_DIR}/../Plugins/mp3Decoder/include/")

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

ADD_CUSTOM_COMMAND(
	TARGET cAudioEngine
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cAudioEngine> ${OUTPUT_BIN_DIR}
)