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
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDIF()

ADD_LIBRARY(Pinocchio STATIC ${header_files} ${src_files})