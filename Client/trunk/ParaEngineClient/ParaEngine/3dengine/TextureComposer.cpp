//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.3.16
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ImageEntity.h"
#include "TextureComposer.h"

using namespace ParaEngine;

//
// CustomCharRenderTarget
//
ParaEngine::TextureComposeRenderTarget::TextureComposeRenderTarget()
	:m_bFlipY(true), m_colorKey(0)
{

}

ParaEngine::TextureComposeRenderTarget::~TextureComposeRenderTarget()
{
}

HRESULT ParaEngine::TextureComposeRenderTarget::Draw(SceneState * sceneState)
{
	if (IsAssetAllLoaded())
	{
		for (auto& layers : m_pending_layers)
		{
			{
				ScopedPaintOnRenderTarget paint(this);
				layers->DoPaint(this);
			}
			layers->UpdateToStaticTexture(this, GetColorKey());
			m_dirty_layers.erase(m_dirty_layers.find(layers->GetName()));
			m_completed_layers.push_back(layers);
		}
		m_pending_layers.clear();

		SetDirty(!m_dirty_layers.empty());

		if (m_dirty_layers.empty())
		{
			// if every thing is finished, we will unload all temporary textures in finished layers. 
			for (auto& layers : m_completed_layers)
			{
				layers->OnTaskCompleted();
			}
			m_completed_layers.clear();
		}
	}
	return E_FAIL;
}

void ParaEngine::TextureComposeRenderTarget::AddTask(TextureComposeTask* pTask)
{
	if (pTask && m_all_layers.find(pTask->GetName()) == m_all_layers.end())
	{
		ref_ptr<TextureComposeTask> layers(pTask);
		m_all_layers[pTask->GetName()] = layers;
		m_dirty_layers[pTask->GetName()] = layers;
		SetDirty(true);
	}
}

bool ParaEngine::TextureComposeRenderTarget::IsAssetAllLoaded()
{
	if (!m_dirty_layers.empty())
	{
		m_pending_layers.clear();
		for (auto& layers : m_dirty_layers)
		{
			if (layers.second)
			{
				if (layers.second->IsAssetAllLoaded())
					m_pending_layers.push_back(layers.second);
			}
		}
	}
	return !(m_pending_layers.empty());
}


//
// TextureComposeTask
//
bool ParaEngine::TextureComposeTask::UpdateToStaticTexture(CRenderTarget* pRenderTarget, Color colorKey)
{
	ImageEntity* image = pRenderTarget->NewImage(true, colorKey);
	bool res = false;
	if (image)
	{
		if (image->IsValid())
		{
			TextureEntity* pTexture = GetTexture();
			if (pTexture)
			{
				if (pTexture->LoadFromImage(image, D3DFMT_DXT3))
					res = true;
				else
				{
					OUTPUT_LOG("warning: failed to save render target to static texture : %s\n", m_name.c_str());
				}
			}
		}
		SAFE_DELETE(image);
	}
	return res;
}

TextureEntity* ParaEngine::TextureComposeTask::GetTexture()
{
	if (!m_composedTexture)
	{
		m_composedTexture = CGlobals::GetAssetManager()->LoadTexture(GetName(), GetName());
		if (!m_composedTexture->IsLoaded())
		{
			// this avoid being loaded from local file. 
			m_composedTexture->SetLocalFileName("");
		}
	}
	return m_composedTexture.get();
}

