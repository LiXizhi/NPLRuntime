# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.5.27
# Note: download luabind and boost src, and then use cmake to compile
# Note1: During release build, edit luabind/config.hpp and add #define LUABIND_NO_ERROR_CHECKING

PROJECT(luabind)

# source files for luabind
SET(luabind_SRCS )

file (GLOB luabind_src_SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)
SOURCE_GROUP("src" FILES ${luabind_src_SRCS})
list(APPEND luabind_SRCS ${luabind_src_SRCS})

file (GLOB luabind_bind_SRCS "${PROJECT_SOURCE_DIR}/luabind/*.hpp" "${PROJECT_SOURCE_DIR}/luabind/detail/*.hpp")
SOURCE_GROUP("luabind" FILES ${luabind_bind_SRCS})
list(APPEND luabind_SRCS ${luabind_bind_SRCS})

include_directories("${luabind_SOURCE_DIR}")
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${Boost_INCLUDE_DIRS}")

IF(WIN32)
	add_definitions(-DLUA_BUILD_AS_DLL)
ENDIF(WIN32)

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

ADD_LIBRARY(luabind ${luabind_SRCS})

