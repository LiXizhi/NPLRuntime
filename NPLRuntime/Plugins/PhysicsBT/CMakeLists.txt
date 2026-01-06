# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.2.24
# Desc: ParaEngine plugin for the bullet physics engine. It implement the physics interface in ParaEngine. 
# Note: specify the Bullet_Root directory in case it is changed. 

project (PhysicsBT)

# NOTE: modify the root directory path if it is installed at a different location. 
#set (Bullet_Root ${PROJECT_SOURCE_DIR}/../bullet-2.75)
#set (Bullet_INCLUDE_DIR ${Bullet_Root}/src)

SET(PhysicsBT_SRCS
	stdafx.cpp
	stdafx.h
	ParaPhysicsWorld.cpp
	ParaPhysicsWorld.h
	PhysicsDebugDraw.cpp
	PhysicsDebugDraw.h
	PhysicsBT.cpp
	PhysicsBT.def
	changes.txt
	readme.txt
)

# Note: include the ParaEngine/NPLRuntime include directory
set(NPLRUNTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaEngine)

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${NPLRUNTIME_ROOT}")
include_directories("${NPLRUNTIME_ROOT}/Core")
include_directories("${NPLRUNTIME_ROOT}/util")
include_directories("${NPLRUNTIME_ROOT}/Framework/Interface")
include_directories("${BULLET_PHYSICS_SOURCE_DIR}/src")

IF(MSVC)
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()


IF(MSVC)
	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF(MSVC)

IF(WIN32)
    SET(CMAKE_DEBUG_POSTFIX "_d")
ENDIF()

if(PHYSICS_STATICLIB)
	SET(PHYSICS_LIB_MODE "STATIC")
	ADD_DEFINITIONS(-DPHYSICS_STATICLIB)
else()
	SET(PHYSICS_LIB_MODE "SHARED")
endif()

# add the executable
add_library(PhysicsBT ${PHYSICS_LIB_MODE} ${PhysicsBT_SRCS})


target_link_libraries(PhysicsBT BulletDynamics BulletCollision LinearMath BulletSoftBody)

if(MSVC)
	set(DEBUG_EXE_NAME "$<TARGET_FILE_NAME:PhysicsBT>")
	set(RELEASE_EXE_NAME "$<TARGET_FILE_NAME:PhysicsBT>")
endif(MSVC)

if(PHYSICS_STATICLIB)
else()
	ADD_CUSTOM_COMMAND(
		   TARGET PhysicsBT
		   POST_BUILD

		   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
		   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PhysicsBT> ${OUTPUT_BIN_DIR}
		)
endif()
