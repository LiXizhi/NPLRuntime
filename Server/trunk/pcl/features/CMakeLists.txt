cmake_minimum_required(VERSION 2.6)

project(pcl_features)

file (GLOB header_files 
	include/pcl/features/feature.h
	include/pcl/features/feature.hpp
	include/pcl/features/fpfh.h
	include/pcl/features/fpfh.hpp
	include/pcl/features/fpfh_omp.h
	include/pcl/features/fpfh_omp.hpp
	include/pcl/features/pfh.h
	include/pcl/features/pfh_tools.hpp
)

file (GLOB src_files
	src/fpfh.cpp
	src/pfh.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${CLIENT_SOURCE_DIR}/trunk/externals/pcl/common/include)
include_directories("$ENV{BOOST_ROOT}")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/Eigen")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/flann-1.8.4-src/src/cpp/")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/pcl/search/include")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/pcl/octree/include")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/pcl/surface/include")
include_directories("${CLIENT_SOURCE_DIR}/trunk/externals/pcl/kdtree/include")

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

ADD_LIBRARY(pcl_features STATIC ${header_files} ${src_files})