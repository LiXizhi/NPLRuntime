# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.5.27
# Note: download luabind and boost src, and then use cmake to compile
# Note1: During release build, edit luabind/config.hpp and add #define LUABIND_NO_ERROR_CHECKING
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
PROJECT(luabind)

# source files for luabind
SET(luabind_SRCS )

file (GLOB luabind_src_SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)
SOURCE_GROUP("src" FILES ${luabind_src_SRCS})
list(APPEND luabind_SRCS ${luabind_src_SRCS})

file (GLOB luabind_bind_SRCS "${PROJECT_SOURCE_DIR}/luabind/*.hpp" "${PROJECT_SOURCE_DIR}/luabind/detail/*.hpp")
SOURCE_GROUP("luabind" FILES ${luabind_bind_SRCS})
list(APPEND luabind_SRCS ${luabind_bind_SRCS})

IF(MSVC)

ELSE()
	# suppress warnings
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")
ENDIF()

ADD_LIBRARY(luabind STATIC ${luabind_SRCS})
target_link_libraries(luabind liblua)
target_include_directories(luabind
PUBLIC 	${PROJECT_SOURCE_DIR}
PUBLIC 	${PROJECT_SOURCE_DIR}/luabind
PRIVATE ${PROJECT_SOURCE_DIR}/src
PUBLIC 	${Boost_INCLUDE_DIR}
)

IF(WIN32)
	if(NOT NPLRUNTIME_STATIC_LIB)
		target_compile_definitions(luabind PUBLIC LUA_BUILD_AS_DLL)
	endif()
ENDIF(WIN32)