
cmake_minimum_required(VERSION 3.15)

option(ParaANTLR_BUILD_EXE "Build the Exe of ParaAltler" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(ParaAntlr4)

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

	# enable multiprocessor build option /MP, this will greatly increase compile speed
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")

	# /GR- Remove RTTI to miminize the executable size
	# RTTI must be enabled since alters requires it, by LiXizhi 
	# SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	# SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

set(ANTLR_BUILD_CPP_TESTS OFF)
set(ANTLR_BUILD_SHARED OFF)
set(WITH_STATIC_CRT OFF)

add_subdirectory(runtime)

file(GLOB SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/python_generated/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_generated/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

list(APPEND INCLUDE_DIRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/python_generated
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp_generated
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src/atn
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src/dfa
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src/tree
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/runtime/src/support
)

if (ANTLR_BUILD_SHARED)
	add_executable(ParaAntlrExe ${SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
	target_include_directories(ParaAntlrExe PUBLIC ${INCLUDE_DIRS})
	target_link_libraries(ParaAntlrExe PUBLIC antlr4_static)
endif()

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC antlr4_static)

