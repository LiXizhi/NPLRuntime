#pragma once

#include "btBulletDynamicsCommon.h"
//#include "btIDebugDraw.h"


namespace ParaEngine
{
	class IParaDebugDraw;

	class CPhysicsDebugDraw : public btIDebugDraw
	{
	public:
		CPhysicsDebugDraw();

		void SetParaDebugDrawInterface(IParaDebugDraw* pDebugDrawInterface);
		IParaDebugDraw* GetParaDebugDrawInterface();

		virtual void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color);

		virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);

		virtual void	reportErrorWarning(const char* warningString);

		virtual void	draw3dText(const btVector3& location,const char* textString);

		virtual void	setDebugMode(int debugMode);

		virtual int		getDebugMode() const;

	protected:
		IParaDebugDraw* m_pDebugDraw;
		int m_nDrawMode;
	};

}