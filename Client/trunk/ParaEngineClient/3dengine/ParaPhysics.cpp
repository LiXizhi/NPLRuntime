//-----------------------------------------------------------------------------
// Class:	ParaPhysics
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.10
// Note: currently just a null implementation for mobile version when Physic Bullet dll is not found. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaPhysics.h"

using namespace ParaEngine;


ParaEngine::CParaPhysicsImp::CParaPhysicsImp()
:m_pDebugDrawer(NULL)
{

}

bool ParaEngine::CParaPhysicsImp::InitPhysics()
{
	return true;
}

bool ParaEngine::CParaPhysicsImp::StepSimulation(float fDeltaTime)
{
	return true;
}

bool ParaEngine::CParaPhysicsImp::ExitPhysics()
{
	return true;
}

void ParaEngine::CParaPhysicsImp::Release()
{
	delete this;
}

IParaPhysicsShape* ParaEngine::CParaPhysicsImp::CreateTriangleMeshShap(const ParaPhysicsTriangleMeshDesc& meshDesc)
{
	return NULL;
}

void ParaEngine::CParaPhysicsImp::ReleaseShape(IParaPhysicsShape *pShape)
{
	
}

IParaPhysicsActor* ParaEngine::CParaPhysicsImp::CreateActor(const ParaPhysicsActorDesc& meshDesc)
{
	return NULL;
}

void ParaEngine::CParaPhysicsImp::ReleaseActor(IParaPhysicsActor* pActor)
{
	
}

IParaPhysicsActor* ParaEngine::CParaPhysicsImp::RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange)
{
	return NULL;
}

void ParaEngine::CParaPhysicsImp::SetDebugDrawer(IParaDebugDraw* debugDrawer)
{
	m_pDebugDrawer = debugDrawer;
}

IParaDebugDraw* ParaEngine::CParaPhysicsImp::GetDebugDrawer()
{
	return m_pDebugDrawer;
}

void ParaEngine::CParaPhysicsImp::DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color)
{
}

void ParaEngine::CParaPhysicsImp::DebugDrawWorld()
{
}

void ParaEngine::CParaPhysicsImp::SetDebugDrawMode(int debugMode)
{
	m_nDebugMode = debugMode;
}

int ParaEngine::CParaPhysicsImp::GetDebugDrawMode()
{
	return m_nDebugMode;
}
