# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 2.6)

project (CLIENT)

message(${CLIENT_SOURCE_DIR})

if(WIN32)
	include(cmake/win32.cmake)
elseif(APPLE)
	include(cmake/osx.cmake)
endif()

