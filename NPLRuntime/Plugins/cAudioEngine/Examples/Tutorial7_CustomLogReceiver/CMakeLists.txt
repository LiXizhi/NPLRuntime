#-------------------------------------------------------------------
# This file is part of the CMake build system for CAUDIO
#-------------------------------------------------------------------

############################################################
# Tutorial7_CustomLogReceiver Player
############################################################

PROJECT(Tutorial7_CustomLogReceiver)

set (SOURCE_FILES
	include/cTestLogReceiver.h
	src/cTestLogReceiver.cpp
	src/main.cpp
)
 
 if(CAUDIO_IOS_BUILD)
	# TODO add ios appdelegate
 endif()
 

include_directories (include ${CAUDIO_INCLUDE_DIR} )
add_executable(Tutorial7_CustomLogReceiver ${SOURCE_FILES})

target_link_libraries(Tutorial7_CustomLogReceiver cAudio)
add_dependencies(Tutorial7_CustomLogReceiver cAudio)

if(CAUDIO_IOS_BUILD)
	set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()

if(${CAUDIO_STATIC})
	ADD_DEFINITIONS(-DCAUDIO_STATIC_LIB=1)
endif()

if (WIN32)
	# append _d for debug builds
	set_property(TARGET Tutorial7_CustomLogReceiver APPEND PROPERTY DEBUG_POSTFIX "_d")
endif()

if (APPLE)
	# On OS X, create .app bundle
	set_property(TARGET Tutorial7_CustomLogReceiver PROPERTY MACOSX_BUNDLE TRUE)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.YOUR_COMPANY.\${PRODUCT_NAME:rfc1034identifier}")
	set_property(TARGET Tutorial7_CustomLogReceiver PROPERTY MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/../Info.plist)
	
	if(CAUDIO_IOS_BUILD)
		set_target_properties(Tutorial7_CustomLogReceiver PROPERTIES XCODE_ATTRIBUTE_GCC_THUMB_SUPPORT "NO")
		set_target_properties(Tutorial7_CustomLogReceiver PROPERTIES XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
		set_target_properties(Tutorial7_CustomLogReceiver PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")	
	endif()
		
endif ()

install_all_targets(Tutorial7_CustomLogReceiver)

