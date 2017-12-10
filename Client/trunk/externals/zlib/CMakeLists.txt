# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5

# Note: set to true if build as dll
set(BUILD_SHARED_DLL false)

PROJECT(ZLIB)
# source files for zlib
SET(ZLIB_SRCS
adler32.c   gzio.c      
inftrees.c  uncompr.c
compress.c  infback.c
zutil.c  crc32.c
deflate.c   inffast.c  
inflate.c   trees.c

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
ENDIF()

IF(BUILD_SHARED_DLL)
	ADD_DEFINITIONS(-DZLIB_DLL)
	ADD_LIBRARY(zlib SHARED ${ZLIB_SRCS})
ELSE(BUILD_SHARED_DLL)	
	ADD_LIBRARY(zlib ${ZLIB_SRCS})
ENDIF(BUILD_SHARED_DLL)

target_include_directories(zlib PUBLIC ${PROJECT_SOURCE_DIR})



