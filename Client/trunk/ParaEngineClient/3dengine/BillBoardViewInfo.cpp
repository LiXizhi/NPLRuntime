//-----------------------------------------------------------------------------
// Class:	billboard view info
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneState.h"
#include "BillBoardViewInfo.h"

using namespace ParaEngine;


void ParaEngine::BillBoardViewInfo::UpdateBillBoardInfo(SceneState* pSceneState)
{
	Vector3 vDir = pSceneState->vLookAt - pSceneState->vEye;
	if (vDir.x > 0.0f)
		m_billboardYAngles = -atanf(vDir.z / vDir.x) + MATH_PI / 2;
	else
		m_billboardYAngles = -atanf(vDir.z / vDir.x) - MATH_PI / 2;
	ParaMatrixRotationY(&matBillboardMatrix, m_billboardYAngles);
	
	float fPitch = 0;
	rotationX = Math::Cos(m_billboardYAngles);
	rotationZ = Math::Sin(m_billboardYAngles);
	rotationYZ = -rotationZ * Math::Sin(fPitch);
	rotationXY = rotationX * Math::Sin(fPitch);
	rotationXZ = Math::Cos(fPitch);

	// spherical billboard mat
	Math::CreateBillboardMatrix(&matBillboardMatrixSphere, NULL, NULL, false);
}

ParaEngine::Vector3 ParaEngine::BillBoardViewInfo::TransformVertex(const Vector3& v)
{
	return v;
}

ParaEngine::Vector3 ParaEngine::BillBoardViewInfo::TransformVertexWithoutY(const Vector3& v)
{
	// this is faster. 
	return Vector3(rotationX*v.x + rotationZ*v.z, v.y, -rotationZ*v.x + rotationX*v.z);
	// return v.TransformNormal(matBillboardMatrix);
}
