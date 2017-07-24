
INCLUDE_DIRECTORIES(
	${BULLET_PHYSICS_SOURCE_DIR}/src
)

SET(PhysicsBT_SRCS
    ParaPhysicsWorld.cpp
    PhysicsBT.cpp
    PhysicsDebugDraw.cpp
)

SET(PhysicsBT_HDRS
	ParaPhysicsWorld.h
	PhysicsDebugDraw.h
)
set(ParaEngineClient_SOURCE_DIR ${PARAENGINE_ROOT}/Mobile/trunk/ParaCraftMobile/frameworks/runtime-src/Classes CACHE PATH "Path to ParaEngine Src")
include_directories("${ParaEngineClient_SOURCE_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/Core")
include_directories("${ParaEngineClient_SOURCE_DIR}/NPL")

ADD_LIBRARY(PhysicsBT ${PhysicsBT_SRCS} ${PhysicsBT_HDRS})
if (BUILD_SHARED_LIBS)
  target_link_libraries(PhysicsBT Bullet3Dynamics)
endif ()

