cmake_minimum_required(VERSION 2.6)

project(pcl_common)

file (GLOB header_files 
	include/pcl/exceptions.h
	include/pcl/pcl_exports.h
	include/pcl/pcl_macros.h
	include/pcl/PCLPointField.h
	include/pcl/point_cloud.h
	include/pcl/point_traits.h
	include/pcl/point_types.h
	include/pcl/point_types.hpp
	include/pcl/point_types_conversion.h
	include/pcl/print.h
)

file (GLOB src_files
	src/point_types.cpp
	src/print.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${SERVER_SOURCE_DIR}/Eigen")

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


ADD_LIBRARY(pcl_common STATIC ${header_files} ${src_files})