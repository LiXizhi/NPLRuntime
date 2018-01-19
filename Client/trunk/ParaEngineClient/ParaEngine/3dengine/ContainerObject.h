#pragma once
#include "BaseObject.h"

namespace ParaEngine
{
/**
* Desc: present a scene object that is not rendered, but may have a volume.
*/
class CContainerObject : public CBaseObject
{
public:
	CContainerObject(void);
	virtual ~CContainerObject(void);
	virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::ContainerObject;};

	ATTRIBUTE_DEFINE_CLASS(CContainerObject);
	ATTRIBUTE_SUPPORT_CREATE_FACTORY(CContainerObject);

	/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
	virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

public:
	virtual void Animate(double dTimeDelta, int nRenderNumber = 0);
	virtual HRESULT Draw( SceneState * sceneState);
	virtual int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);
};
}