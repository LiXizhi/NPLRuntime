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

ADD_LIBRARY(jsoncpp STATIC ${jsoncpp_SRCS})
target_include_directories(jsoncpp PUBLIC ${PROJECT_SOURCE_DIR}/include/)