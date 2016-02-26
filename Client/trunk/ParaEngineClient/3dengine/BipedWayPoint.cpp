//-----------------------------------------------------------------------------
// Class:	BipedWayPoint
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2005.10.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedWayPoint.h"

using namespace ParaEngine;


BipedWayPoint::BipedWayPoint(const Vector3& p, MyType ptType, float fTime)
	:vPos(p), nType(ptType), fTimeLeft(fTime), fFacing(0), bUseFacing(false), m_nReserved0(0)
{
}
BipedWayPoint::BipedWayPoint(const Vector3& p, float facing, MyType ptType, float fTime)
	: vPos(p), fFacing(facing), nType(ptType), fTimeLeft(fTime), bUseFacing(true), m_nReserved0(0)
{
}

BipedWayPoint::BipedWayPoint(float fBlockTime)
	: fTimeLeft(fBlockTime), nType(BipedWayPoint::BLOCKED), fFacing(0), bUseFacing(false), m_nReserved0(0), vPos(0, 0, 0)
{
}
BipedWayPoint::BipedWayPoint(MyType ptType)
	: nType(ptType), fFacing(0), bUseFacing(false), m_nReserved0(0), vPos(0,0,0)
{
}
BipedWayPoint::BipedWayPoint()
	: nType(BipedWayPoint::COMMAND_POINT), fTimeLeft(-1.f), fFacing(0), bUseFacing(false), m_nReserved0(0), vPos(0, 0, 0)
{
}

bool BipedWayPoint::IsMoving()
{
	return (GetPointType() < BipedWayPoint::BLOCKED);
}
bool BipedWayPoint::IsUseFacing()
{
	return bUseFacing;
}



