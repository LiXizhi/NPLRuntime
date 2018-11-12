cmake_minimum_required(VERSION 2.6)

project(Pinocchio)

file (GLOB header_files 
	include/attachment.h
	include/debugging.h
	include/deriv.h
	include/dtree.h
	include/graphutils.h
	include/hashutils.h
	include/indexer.h
	include/intersector.h
	include/lsqSolver.h
	include/mathutils.h
	include/matrix.h
	include/mesh.h
	include/Pinocchio.h
	include/pinocchioApi.h
	include/pointprojector.h
	include/quaddisttree.h
	include/prect.h
	include/skeleton.h
	include/transform.h
	include/utils.h
	include/pvector.h
	include/vecutils.h
)

file (GLOB src_files
	src/attachment.cpp
	src/discretization.cpp
	src/embedding.cpp
	src/graphutils.cpp
	src/indexer.cpp
	src/intersector.cpp
	src/lsqSolver.cpp
	src/matrix.cpp
	src/mesh.cpp
	src/pinocchioApi.cpp
	src/refinement.cpp
	src/skeleton.cpp
)

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

ADD_LIBRARY(Pinocchio STATIC ${header_files} ${src_files})