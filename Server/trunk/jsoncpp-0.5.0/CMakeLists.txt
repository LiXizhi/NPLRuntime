# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.10.4
# Note: this is fast json reader/writer. we switched to it from tinyjson(boost spirit)

cmake_minimum_required(VERSION 2.6)

PROJECT(jsoncpp)

# source files for tinyxpath
SET(jsoncpp_SRCS
README.txt
)

file (GLOB jsoncpp_headers ${PROJECT_SOURCE_DIR}/include/json/*.h)
# SOURCE_GROUP("header" FILES ${jsoncpp_headers})
list(APPEND jsoncpp_SRCS ${jsoncpp_headers})

file (GLOB jsoncpp_sources 
	${PROJECT_SOURCE_DIR}/src/lib_json/*.cpp
	${PROJECT_SOURCE_DIR}/src/lib_json/*.h
	${PROJECT_SOURCE_DIR}/src/lib_json/*.inl)
# SOURCE_GROUP("source" FILES ${jsoncpp_sources})
list(APPEND jsoncpp_SRCS ${jsoncpp_sources})

include_directories(${PROJECT_SOURCE_DIR}/include)

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

ADD_LIBRARY(jsoncpp STATIC ${jsoncpp_SRCS})

