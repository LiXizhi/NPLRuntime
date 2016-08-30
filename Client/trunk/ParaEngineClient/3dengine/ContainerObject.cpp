//-----------------------------------------------------------------------------
// Class:	CContainerObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised:	2015.1.19
// Desc: container object
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneState.h"
#include "ContainerObject.h"

using namespace ParaEngine;


CContainerObject::CContainerObject(void)
{
}

CContainerObject::~CContainerObject(void)
{
}

HRESULT CContainerObject::Draw( SceneState * sceneState)
{
	return S_OK;
}

void ParaEngine::CContainerObject::Animate(double dTimeDelta, int nRenderNumber /*= 0*/)
{
	for (auto it = GetChildren().begin(); it != GetChildren().end();)
	{
		CBaseObject* pChild = (*it);
		pChild->Animate(dTimeDelta, nRenderNumber);
		if (!(pChild->IsDead()))
			++it;
		else
		{
			pChild->AddToDeadObjectPool();
			it = GetChildren().erase(it);
		}
	}
}

int ParaEngine::CContainerObject::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	for (auto it = GetChildren().begin(); it != GetChildren().end(); )
	{
		CBaseObject* pChild = (*it);
		pChild->PrepareRender(pCamera, pSceneState);
		if (!(pChild->IsDead()))
			++it;
		else
		{
			pChild->AddToDeadObjectPool();
			it = GetChildren().erase(it);
		}
	}
	return 0;
}

int ParaEngine::CContainerObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);
	return S_OK;
}

