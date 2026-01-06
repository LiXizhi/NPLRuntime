IF(MSVC)
# Statically link MSVC runtime for all configurations 
# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDIF()

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

ADD_LIBRARY(pcl_common STATIC ${header_files} ${src_files})