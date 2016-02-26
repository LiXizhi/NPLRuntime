//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.3.14
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "AssetManifest.h"
#include "AsyncLoader.h"
#include "ImageEntity.h"
#include "PaintEngine/Painter.h"
#include "CustomCharRenderTarget.h"

using namespace ParaEngine;

//
// CSkinLayers
//

ParaEngine::CSkinLayers::CSkinLayers(const std::string& name, const std::vector <CharTextureComponent>& layers)
{
	SetName(name);
	m_layers = layers;
}

ParaEngine::CSkinLayers::~CSkinLayers()
{

}

void ParaEngine::CSkinLayers::DoPaint(CPaintDevice* pd)
{
	// DO the texture composition here
	CPainter painter(pd);

	int nSize = (int)m_layers.size();
	for (int i = 0; i < nSize; ++i)
	{
		CharTextureComponent &component = m_layers[i];
		const CharRegionCoords &coords = CCharCustomizeSysSetting::regions[component.region];

		// load the component texture
		if (component.name.empty())
			continue;
		string componentfilename = component.name;
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
		if (pEntry)
			componentfilename = pEntry->GetLocalFileName();

		TextureEntity* pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(componentfilename, componentfilename);

		if (pTextureEntity)
		{
			Color color = component.GetColor();
			painter.setPen(color);
			QRect rect(coords.xpos, coords.ypos, coords.xsize, coords.ysize);
			/*if (IsFlipY())
			{
				rect.setTop(CCharCustomizeSysSetting::CharTexSize - coords.ypos - rect.height());
			}*/
			painter.drawTexture(rect, pTextureEntity);
		}
	}
}

bool ParaEngine::CSkinLayers::IsAssetAllLoaded()
{
	bool bAllLoaded = true;
	int nSize = (int)m_layers.size();
	for (int i = 0; i < nSize; ++i)
	{
		CharTextureComponent &component = m_layers[i];
		if (!component.CheckLoad())
			bAllLoaded = false;
	}
	return bAllLoaded;
}

void ParaEngine::CSkinLayers::OnTaskCompleted()
{
	int nSize = (int)m_layers.size();
	for (int i = 0; i < nSize; ++i)
	{
		CharTextureComponent &component = m_layers[i];
		const CharRegionCoords &coords = CCharCustomizeSysSetting::regions[component.region];

		// load the component texture
		if (component.name.empty())
			continue;
		string componentfilename = component.name;
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
		if (pEntry)
			componentfilename = pEntry->GetLocalFileName();

		TextureEntity* pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(componentfilename, componentfilename);

		if (pTextureEntity)
		{
			pTextureEntity->UnloadAsset();
		}
	}
}

//
// CFaceLayers
//
ParaEngine::CFaceLayers::CFaceLayers(const std::string& name, CartoonFace& faceTexture)
{
	SetName(name);
	faceTexture.GetFaceComponents(m_layers);
}

bool ParaEngine::CFaceLayers::IsAssetAllLoaded()
{
	bool bAllLoaded = true;
	for (int i = 0; i < CFS_TOTAL_NUM; ++i)
	{
		FaceTextureComponent& component = m_layers[i];
		// load the component texture
		if (!component.CheckLoad())
			bAllLoaded = false;
	}
	return bAllLoaded;
}

void ParaEngine::CFaceLayers::DoPaint(CPaintDevice* pd)
{
	// DO the texture composition here
	CPainter painter(pd);
	for (int i = 0; i<CFS_TOTAL_NUM; ++i)
	{
		const CharRegionCoords &coords = CCharCustomizeSysSetting::regions[CR_FACE_BASE + i];
		const FaceTextureComponent& component = m_layers[i];

		// load the component texture
		if (component.name.empty())
			continue;
		string componentfilename = component.name;

		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
		if (pEntry)
			componentfilename = pEntry->GetLocalFileName();

		// compute the transform matrix
		QTransform transformMatrix;
		// scale around the center
		float fScale = component.GetScaling();
		if (fabs(fScale)>0.01f)
		{
			transformMatrix.scale(fScale + 1.f, fScale + 1.f);
		}

		// rotate around the center
		float fRotation = component.GetRotation();
		if (fabs(fRotation) > 0.01f)
		{
			transformMatrix.rotateRadians(fRotation);
		}

		// translation
		int x, y;
		component.GetPosition(&x, &y);
		transformMatrix.translate(coords.xpos + (float)x, coords.ypos + (float)y);

		painter.setWorldTransform(transformMatrix, false);

		Color color = component.GetColor();
		painter.setPen(color);
		TextureEntity* pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(componentfilename, componentfilename);

		if (pTextureEntity)
		{
			QRect rect((-coords.xsize / 2), (-coords.ysize / 2), coords.xsize, coords.ysize);
			painter.drawTexture(rect, pTextureEntity);
		}

		// for eye and eye bow, there should be a mirrored image, around the center of the render target
		if (i == CFS_EYE || i == CFS_EYEBROW)
		{
			QTransform reflectMat(-1.f, 0.f, 0.f, 1.f, CCharCustomizeSysSetting::FaceTexSize - (coords.xpos + (float)x) * 2, 0.f);
			transformMatrix = reflectMat * transformMatrix;
			painter.setWorldTransform(transformMatrix, false);
			if (pTextureEntity)
			{
				QRect rect((-coords.xsize / 2), (-coords.ysize / 2), coords.xsize, coords.ysize);
				painter.drawTexture(rect, pTextureEntity);
			}
		}
	}
}

void ParaEngine::CFaceLayers::OnTaskCompleted()
{
	for (int i = 0; i < CFS_TOTAL_NUM; ++i)
	{
		const FaceTextureComponent& component = m_layers[i];

		// load the component texture
		if (component.name.empty())
			continue;

		TextureEntity* pTextureEntity = NULL;
		AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(component.name);
		if (pEntry)
			pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(pEntry->GetLocalFileName(), pEntry->GetLocalFileName());
		else
			pTextureEntity = CGlobals::GetAssetManager()->LoadTexture(component.name, component.name);

		if (pTextureEntity)
		{
			pTextureEntity->UnloadAsset();
		}
	}
}
