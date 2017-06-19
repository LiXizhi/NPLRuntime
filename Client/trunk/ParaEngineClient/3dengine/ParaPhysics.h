#pragma once
#include "IParaPhysics.h"

namespace ParaEngine
{
	class CParaPhysicsImp : public IParaPhysics
	{
	public:
		CParaPhysicsImp();

	public:
		virtual bool InitPhysics();

		virtual bool StepSimulation(float fDeltaTime);

		virtual bool ExitPhysics();

		virtual void Release();

		virtual IParaPhysicsShape* CreateTriangleMeshShap(const ParaPhysicsTriangleMeshDesc& meshDesc);

		virtual void ReleaseShape(IParaPhysicsShape *pShape);

		virtual IParaPhysicsActor* CreateActor(const ParaPhysicsActorDesc& meshDesc);

		virtual void ReleaseActor(IParaPhysicsActor* pActor);

		virtual IParaPhysicsActor* RaycastClosestShape(const PARAVECTOR3& vOrigin, const PARAVECTOR3& vDirection, DWORD dwType, RayCastHitResult& hit, short dwGroupMask, float fSensorRange);

		virtual void SetDebugDrawer(IParaDebugDraw* debugDrawer);

		virtual IParaDebugDraw* GetDebugDrawer();

		virtual void DebugDrawObject(const PARAVECTOR3& vOrigin, const PARAMATRIX3x3& vRotation, const IParaPhysicsShape* pShape, const PARAVECTOR3& color);

		virtual void DebugDrawWorld();

		virtual void SetDebugDrawMode(int debugMode);

		virtual int GetDebugDrawMode();
	protected:
		IParaDebugDraw* m_pDebugDrawer;
		int m_nDebugMode;
	};
}
	

