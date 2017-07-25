# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.5
# Note: download lua src, 
# O edit luaconf.h. and edit luaconf.h as below:
# #define LUA_IDSIZE	120
# #define LUA_COMPAT_GETN
# add npl_compiler.c(h) file to the source
# and then use cmake to compile

PROJECT(lua)

# source files for lua
SET(lua_SRCS
src/lapi.c       src/lauxlib.c    src/lbaselib.c   src/lcode.c      src/ldblib.c     src/ldebug.c
src/ldo.c        src/ldump.c      src/lfunc.c      src/lgc.c        src/linit.c      src/liolib.c
src/llex.c       src/lmathlib.c   src/lmem.c       src/loadlib.c    src/lobject.c    src/lopcodes.c
src/loslib.c     src/lparser.c    src/lstate.c     src/lstring.c    src/lstrlib.c    src/ltable.c
src/ltablib.c    src/ltm.c        src/lundump.c    src/lvm.c
src/lzio.c       src/print.c

# src/luac.c src/lua.c
# Note: added by LiXizhi 2010.3.15, which is exported from src/luac.c
# src/npl_compiler.c src/npl_compiler.h

src/lapi.h       src/lauxlib.h    src/lcode.h      src/ldebug.h     src/ldo.h        src/lfunc.h
src/lgc.h        src/llex.h       src/llimits.h    src/lmem.h       src/lobject.h    src/lopcodes.h
src/lparser.h    src/lstate.h     src/lstring.h    src/ltable.h     src/ltm.h        src/lua.h
src/luaconf.h    src/lualib.h     src/lundump.h    src/lvm.h        src/lzio.h
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
	
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

# the following will build lua as dll, the reason for this, is that we need its pdb files for debugging using decoda (a lua debugger). 
# plus it also makes core game engine update more granular. 
IF(WIN32)
	add_definitions(-DLUA_BUILD_AS_DLL)
	SET(CMAKE_DEBUG_POSTFIX "_d")
	
	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /opt:ref")
	# add a stripped version of symbol files during release build
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /PDBSTRIPPED:Release/lua.stripped.pdb")
	
	ADD_LIBRARY(lua SHARED ${lua_SRCS})

	SET(output_luafilename "lua51.dll")
ELSE(WIN32)
	add_definitions(-DLUA_USE_LINUX)
	ADD_LIBRARY(lua SHARED ${lua_SRCS})

	SET(output_luafilename "liblua51.so")
ENDIF(WIN32)

# SET_TARGET_PROPERTIES(lua PROPERTIES OUTPUT_NAME "lua51")

 ADD_CUSTOM_COMMAND(
 	TARGET lua
 	POST_BUILD
 	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
 	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:lua> ${OUTPUT_BIN_DIR}/${output_luafilename}
)


