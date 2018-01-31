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

target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



