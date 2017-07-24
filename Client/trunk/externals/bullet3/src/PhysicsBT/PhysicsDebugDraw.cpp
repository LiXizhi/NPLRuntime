//-----------------------------------------------------------------------------
// Class:	Physics Debug drawing
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2010.2.26
//-----------------------------------------------------------------------------
#include "PluginAPI.h"
#include "ParaPhysicsWorld.h"
#include "IParaDebugDraw.h"

#include "PhysicsDebugDraw.h"

using namespace ParaEngine;


ParaEngine::CPhysicsDebugDraw::CPhysicsDebugDraw()
:m_nDrawMode(0),m_pDebugDraw(NULL)
{

}

void ParaEngine::CPhysicsDebugDraw::SetParaDebugDrawInterface( IParaDebugDraw* pDebugDrawInterface)
{
	m_pDebugDraw = pDebugDrawInterface;
}

IParaDebugDraw* ParaEngine::CPhysicsDebugDraw::GetParaDebugDrawInterface()
{
	return m_pDebugDraw;
}

void ParaEngine::CPhysicsDebugDraw::setDebugMode( int debugMode )
{
	m_nDrawMode = debugMode;
}

int ParaEngine::CPhysicsDebugDraw::getDebugMode() const
{
	return m_nDrawMode;
}

void ParaEngine::CPhysicsDebugDraw::drawLine( const btVector3& from,const btVector3& to,const btVector3& color )
{
	if(m_pDebugDraw)
	{
		PARAVECTOR3 from_(from.getX(), from.getY(), from.getZ());
		PARAVECTOR3 to_(to.getX(), to.getY(), to.getZ());
		PARAVECTOR3 color_(color.getX(), color.getY(), color.getZ());
		m_pDebugDraw->DrawLine(from_, to_, color_);
	}
}

void ParaEngine::CPhysicsDebugDraw::drawContactPoint( const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color )
{
	if(m_pDebugDraw)
	{
		PARAVECTOR3 PointOnB_(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
		PARAVECTOR3 normalOnB_(normalOnB.getX(), normalOnB.getY(), normalOnB.getZ());
		PARAVECTOR3 color_(color.getX(), color.getY(), color.getZ());
		m_pDebugDraw->DrawContactPoint(PointOnB_, normalOnB_, distance, lifeTime, color_);
	}
}

void ParaEngine::CPhysicsDebugDraw::reportErrorWarning( const char* warningString )
{
	if(m_pDebugDraw)
	{
		m_pDebugDraw->ReportErrorWarning(warningString);
	}
}

void ParaEngine::CPhysicsDebugDraw::draw3dText( const btVector3& location,const char* textString )
{
	if(m_pDebugDraw)
	{
		PARAVECTOR3 location_(location.getX(), location.getY(), location.getZ());
		m_pDebugDraw->Draw3dText(location_, textString);
	}
}

