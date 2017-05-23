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

namespace ParaEngine {


	CParaPhysicsImp::CParaPhysicsImp()
		:m_pDebugDrawer(nullptr)
	{

	}

	void CParaPhysicsImp::Release()
	{
		delete this;
	}

	IParaPhysicsWorld* CParaPhysicsImp::InitPhysics(ParaPhysicsWorldType ppwt)
	{
		return nullptr;
	}

	IParaPhysicsTriangleMeshShape* CParaPhysicsImp::CreateTriangleMeshShape(const ParaPhysicsTriangleMeshDesc& meshDesc)
	{
		return nullptr;
	}

	IParaPhysicsShape * CParaPhysicsImp::CreateBoxShape(const PARAVECTOR3 & boxHalfExtents)
	{
		return nullptr;
	}

	IParaPhysicsShape * CParaPhysicsImp::CreateCylinderShape(const PARAVECTOR3 & boxHalfExtents)
	{
		return nullptr;
	}

	IParaPhysicsShape* CParaPhysicsImp::CreateSphereShape(float radius)
	{
		return nullptr;
	}

	IParaPhysicsShape* CParaPhysicsImp::CreateCapsuleShapeY(float radius, float height)
	{
		return nullptr;
	}

	IParaPhysicsShape* CParaPhysicsImp::CreateCapsuleShapeX(float radius, float height)
	{
		return nullptr;
	}

	IParaPhysicsShape* CParaPhysicsImp::CreateCapsuleShapeZ(float radius, float height)
	{
		return nullptr;
	}

	void CParaPhysicsImp::SetDebugDrawer(IParaDebugDraw* debugDrawer)
	{
		m_pDebugDrawer = debugDrawer;
	}

	IParaDebugDraw* CParaPhysicsImp::GetDebugDrawer()
	{
		return m_pDebugDrawer;
	}

	void CParaPhysicsImp::DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color)
	{
	}


	void CParaPhysicsImp::SetDebugDrawMode(int debugMode)
	{
		m_nDebugMode = debugMode;
	}

	int CParaPhysicsImp::GetDebugDrawMode()
	{
		return m_nDebugMode;
	}

}