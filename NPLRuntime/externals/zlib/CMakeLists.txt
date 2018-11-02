# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5

# Note: set to true if build as dll
set(BUILD_SHARED_DLL false)

PROJECT(ZLIB)
# source files for zlib
SET(ZLIB_SRCS
adler32.c
compress.c
crc32.c
deflate.c
gzclose.c
gzlib.c
gzread.c
gzwrite.c
inflate.c
infback.c
inftrees.c
inffast.c
trees.c
uncompr.c
zutil.c
)


IF(BUILD_SHARED_DLL)
	ADD_DEFINITIONS(-DZLIB_DLL)
	ADD_LIBRARY(zlib SHARED ${ZLIB_SRCS})
ELSE(BUILD_SHARED_DLL)	
	ADD_LIBRARY(zlib ${ZLIB_SRCS})
ENDIF(BUILD_SHARED_DLL)

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

target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



