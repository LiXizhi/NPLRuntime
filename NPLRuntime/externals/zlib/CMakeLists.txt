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
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



