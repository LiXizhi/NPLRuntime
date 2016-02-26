#pragma once
#include "BaseObject.h"

namespace ParaEngine
{
struct MeshEntity;
/**
* dynamic actor based scene object
*/
class CDynamicObject :
	public CBaseObject
{
public:
	CDynamicObject(void);
	virtual ~CDynamicObject(void);
public:
	HRESULT InitObject(MeshEntity* ppMesh, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bRenderMesh);
};
}