#-------------------------------------------------------------------
# This file is part of the CMake build system for CAUDIO
#-------------------------------------------------------------------

############################################################
# Tutorial5_AudioEffects Player
############################################################

PROJECT(Tutorial5_AudioEffects)

set (SOURCE_FILES
	src/main.cpp
)
 
 if(CAUDIO_IOS_BUILD)
	# TODO add ios appdelegate
 endif()
 

include_directories (include ${CAUDIO_INCLUDE_DIR} )
add_executable(Tutorial5_AudioEffects ${SOURCE_FILES})

target_link_libraries(Tutorial5_AudioEffects cAudio)
add_dependencies(Tutorial5_AudioEffects cAudio)

if(CAUDIO_IOS_BUILD)
	set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

if(${CAUDIO_STATIC})
	ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB=1)
endif()

if (WIN32)
	# append _d for debug builds
	set_property(TARGET Tutorial5_AudioEffects APPEND PROPERTY DEBUG_POSTFIX "_d")
endif()

if (APPLE)
	# On OS X, create .app bundle
	set_property(TARGET Tutorial5_AudioEffects PROPERTY MACOSX_BUNDLE TRUE)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.YOUR_COMPANY.\${PRODUCT_NAME:rfc1034identifier}")
	set_property(TARGET Tutorial5_AudioEffects PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist)
	
	if(CAUDIO_IOS_BUILD)
		set_target_properties(Tutorial5_AudioEffects PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
		set_target_properties(Tutorial5_AudioEffects PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
		set_target_properties(Tutorial5_AudioEffects PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")	
	endif()
		
endif ()

install_all_targets(Tutorial5_AudioEffects)

