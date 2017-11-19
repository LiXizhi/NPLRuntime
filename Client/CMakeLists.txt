# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Note: see NPLRuntime/cmakelist.txt

cmake_minimum_required (VERSION 3.3)
project (CLIENT)
set(CMAKE_CXX_STANDARD 11)

if(WIN32)
	include(cmake/win32.cmake)
elseif(APPLE)
	include(cmake/osx.cmake)
endif()

