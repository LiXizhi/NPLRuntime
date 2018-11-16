include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

project(FreeImage)

option(FREEIMAGE_USE_PNG "supported png" ON)
option(FREEIMAGE_USE_JPEG "supported jpg" ON)
option(FREEIMAGE_USE_JXR "supported jxr" OFF)
option(FREEIMAGE_USE_OPENJPEG "supported OpengJPEG" OFF)
option(FREEIMAGE_USE_RAW "supported raw" OFF)
option(FREEIMAGE_USE_TIFF4 "supported tiff4" OFF)
option(FREEIMAGE_USE_WEBP "supported WebP" OFF)
option(FREEIMAGE_USE_OPENGEXR "supported OpenEXR" OFF)

ucm_add_dirs(Source/FreeImage TO FreeImage_Files RECURSIVE)
ucm_add_dirs(Source/FreeImageToolkit TO FreeImageToolkit_Files RECURSIVE)
ucm_add_dirs(Source/Metadata TO Metadata_Files RECURSIVE)


add_library(FreeImage STATIC ${FreeImage_Files} ${FreeImageToolkit_Files} ${Metadata_Files})

target_include_directories(FreeImage PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Source)


set(FREEIMAGE_DEF_LIST)
set(FREEIMAGE_LINK_LIST)


if (FREEIMAGE_USE_PNG)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_PNG)
	list(APPEND FREEIMAGE_LINK_LIST libpng)
endif()

if (FREEIMAGE_USE_JPEG)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_JPEG)
	list(APPEND FREEIMAGE_LINK_LIST libjpeg)
endif()

if (FREEIMAGE_USE_JXR)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_JXR)
endif()

if (FREEIMAGE_USE_OPENJPEG)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_OPENJPEG)
endif()

if (FREEIMAGE_USE_RAW)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_RAW)
endif()

if (FREEIMAGE_USE_TIFF4)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_TIFF4)
endif()

if (FREEIMAGE_USE_WEBP)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_WEBP)
endif()

if (FREEIMAGE_USE_OPENGEXR)
	list(APPEND FREEIMAGE_DEF_LIST FREEIMAGE_USE_OPENGEXR)
endif()

target_compile_definitions(FreeImage PUBLIC 
    FREEIMAGE_LIB
	${FREEIMAGE_DEF_LIST}
)

target_link_libraries(FreeImage PUBLIC 
	${FREEIMAGE_LINK_LIST}
)


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
ELSE()
	# suppress warnings
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")
ENDIF()