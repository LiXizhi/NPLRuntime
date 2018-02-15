//-----------------------------------------------------------------------------
// Class:	CSphereObject
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"

#include "SphereObject.h"

using namespace ParaEngine;

CSphereObject::CSphereObject(void)
	:m_vCenter(0, 0, 0), m_fRadius(0.5f)
{
	
}

CSphereObject::~CSphereObject(void)
{
}

DVector3 CSphereObject::GetObjectCenter()
{
	return m_vCenter;
}

void CSphereObject::SetObjectCenter(const Vector3 & v)
{
	m_vCenter = v;
}

DVector3 CSphereObject::GetPosition()
{
	return DVector3(m_vCenter.x, m_vCenter.y - m_fRadius, m_vCenter.z);
}

void CSphereObject::SetPosition(const DVector3& v)
{
	m_vCenter.x = v.x;
	m_vCenter.y = v.y + m_fRadius;
	m_vCenter.z = v.z;
}

void ParaEngine::CSphereObject::SetRadius(float fRadius)
{
	m_fRadius = fRadius;
}

float ParaEngine::CSphereObject::GetRadius()
{
	return m_fRadius;
}

ParaEngine::ObjectShape ParaEngine::CSphereObject::GetObjectShape()
{
	return _ObjectShape_Sphere;
}

int CSphereObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	CBaseObject::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("position", FieldType_Vector3, (void*)SetPosition_s, (void*)GetPosition_s, NULL, "global position of the character", bOverride);
	return S_OK;
}

