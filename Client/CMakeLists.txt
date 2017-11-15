# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 2.6)

project (CLIENT)


if(WIN32)
	include(cmake/win32.cmake)
endif()

