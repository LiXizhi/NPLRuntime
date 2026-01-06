IF(MSVC)
# Statically link MSVC runtime for all configurations 
# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDIF()

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
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${SERVER_SOURCE_DIR}/Eigen")
include_directories("${SERVER_SOURCE_DIR}/flann-1.8.4-src/src/cpp/")
include_directories("${SERVER_SOURCE_DIR}/pcl/common/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/search/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/octree/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/surface/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/kdtree/include")

ADD_LIBRARY(pcl_features STATIC ${header_files} ${src_files})