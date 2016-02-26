//-----------------------------------------------------------------------------
// Class:	CManagedLoader
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
// Copyright: All rights reserved
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ManagedLoader.h"
using namespace ParaEngine;


CManagedLoader::CManagedLoader(void)
{
	m_bHasAttached = false;
}

CManagedLoader::~CManagedLoader(void)
{
	Cleanup();
}

void CManagedLoader::Cleanup()
{
	if(!m_bHasAttached)
	{
		CSceneObject* pScene = CGlobals::GetScene();

		for (auto pChild : m_children)
		{
			pChild->Release();
		}
	}
}

void CManagedLoader::AddChild(CBaseObject * pObject)
{
	/// only add a new object if the loader has not been attached to scene
	if(!m_bHasAttached && pObject)
		m_children.push_back(pObject);
}

void CManagedLoader::AttachObjectsToScene()
{
	if(!m_bHasAttached)
	{
		CSceneObject* pScene = CGlobals::GetScene();

		for( auto pChild : m_children)
		{
			pChild->SetCtorPercentage(1.0f);
			pScene->AttachObject(pChild);
		}

		m_bHasAttached = true;
	}
}

void CManagedLoader::DetachObjectsFromScene()
{
	if(m_bHasAttached)
	{
		CSceneObject* pScene = CGlobals::GetScene();

		for (auto pChild : m_children)
		{
			pScene->DetachObject(pChild);
		}

		m_bHasAttached = false;
	}
}

void CManagedLoader::PreLoad()
{
	/// currently, this function does nothing.
	//TODO:
}

void CManagedLoader::CompressObject(CompressOption option)
{
	if(m_bHasAttached)
	{
		CSceneObject* pScene = CGlobals::GetScene();

		for (auto pChild : m_children)
		{
			pChild->CompressObject();
		}
	}
}
