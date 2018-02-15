//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CustomCharModelInstance.h"
#include "AssetManifest.h"
#include "ParaWorldAsset.h"
#include "util/HttpUtility.h"
#include "CharacterDB.h"
#include "CustomCharFace.h"

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// CartoonFace
//
//////////////////////////////////////////////////////////////////////////

CartoonFace::CartoonFace()
	: m_bNeedUpdateCache(false)
{
	Reset();
}

CartoonFace::CartoonFace(FaceComponent* _components)
	: m_bNeedUpdateCache(false)
{
	memcpy(components, _components, sizeof(FaceComponent)*CFS_TOTAL_NUM);
}

void CartoonFace::Reset()
{
	for (int i = 0; i < CFS_TOTAL_NUM; ++i)
	{
		FaceComponent component = components[i];
		component.reset();
	}
	m_bNeedUpdate = true;
	m_bNeedUpdateCache = false;
	m_sCachedFileName.clear();
}


std::string CartoonFace::makeComponentTexture(int region, const char *name)
{
	// just return an empty filename
	if (name == 0 || name[0] == '\0')
		return "";

	std::string sName = name;
	std::string fn;
	if (sName.find_last_of('/') == std::string::npos)
	{
		// if it does not contain a parent path, region is used
		std::string fullname = CCharCustomizeSysSetting::GetRegionPath(region);
		fullname += sName;
		if (CParaFile::GetFileExtension(sName) == "")
		{
			fullname += ".dds";
		}
		return fullname;
	}
	else
	{
		if (CParaFile::GetFileExtension(sName) == "")
			sName += ".dds";
		return sName;
	}
}

void CartoonFace::SetModified(bool bModified /*= true*/)
{
	m_bNeedUpdate = bModified;
}

void CartoonFace::compose(asset_ptr<TextureEntity>& tex, CharModelInstance* pCharInst)
{
	m_bNeedUpdate = false;
	m_bNeedUpdateCache = true;

	// if we have composed this texture before, use it. 
	if (CParaFile::DoesFileExist(GetCacheFileName().c_str(), false))
	{
		tex = CGlobals::GetAssetManager()->LoadTexture("", GetCacheFileName().c_str(), TextureEntity::StaticTexture);
	}
	else
	{
		CharModelInstance::AddPendingCartoonFace(pCharInst);
	}
}

const string& CartoonFace::GetCacheFileName()
{
	if (m_bNeedUpdateCache || m_sCachedFileName.empty())
	{
		m_bNeedUpdateCache = false;
		string sParams;
		// put all parameters to sParams, so that we get a unique string for each face configuration. 
		for (int i = 0; i < CFS_TOTAL_NUM; ++i)
		{
			const FaceComponent& component = GetComponent(i);

			int nStyle = component.GetStyle();
			float fScale = component.GetScaling();
			float fRotation = component.GetRotation();
			DWORD color = (DWORD)component.GetColor();
			int x = 0;
			int y = 0;
			component.GetPosition(&x, &y);
			char temp[256];
			sprintf(temp, "%d%d%.2f%.2f%d%d%x", i, nStyle, fScale, fRotation, x, y, color);
			sParams += temp;
		}
		m_sCachedFileName = "temp/composeface/";
		m_sCachedFileName += CHttpUtility::HashStringMD5(sParams.c_str());
		m_sCachedFileName += "Face_dxt1.png"; // used to be ".dds"
	}
	return m_sCachedFileName;
}

void ParaEngine::CartoonFace::GetFaceComponents(std::vector <FaceTextureComponent>& layers)
{
	layers.clear();
	for (int i = 0; i < CFS_TOTAL_NUM; ++i)
	{
		const FaceComponent& component = GetComponent(i);
		FaceTextureComponent dest(component);

		// load the component texture
		int nStyle = component.GetStyle();
		string tex1;
		// We extract from db string from the main thread.
		if (CCharacterDB::GetInstance().GetCartoonFaceComponent(i, nStyle, &tex1, NULL, NULL) == false)
			dest.name = CCharCustomizeSysSetting::DefaultCartoonFaceTexture[i];
		else
			dest.name = makeComponentTexture(CR_FACE_BASE + i, tex1.c_str());

		layers.push_back(dest);
	}
}



//////////////////////////////////////////////////////////////////////////
//
// face components
//
//////////////////////////////////////////////////////////////////////////
FaceComponent::FaceComponent()
{
	reset();
}

void FaceComponent::reset()
{
	SetStyle(0);
	SetColor(0xffffffff);
	SetScaling(0);
	SetRotation(0);
	int x = 0;
	int y = 0;
	SetPosition(&x, &y);
}

float FaceComponent::GetScaling() const
{
	float s = ((float)(((int)SRT[0]) - 0x7f)) / ((float)0x7f);
	return s;
}

void FaceComponent::SetScaling(float scaling)
{
	SRT[0] = ((int)((scaling * 0x7f) + 0x7f)) & 0x000000FF;
}


float FaceComponent::GetRotation() const
{
	float s = ((float)(((int)SRT[1]) - 0x7f))*(3.14f) / 0x7f;
	return s;
}


void FaceComponent::SetRotation(float fRotation)
{
	SRT[1] = ((int)((fRotation / 3.1415926f * 0x7f) + 0x7f)) & 0x000000FF;
}


void FaceComponent::SetPosition(int* x, int* y)
{
	if (x)
		SRT[2] = (*x) + 0x7f;
	if (y)
		SRT[3] = (*y) + 0x7f;
}

void FaceComponent::GetPosition(int* x, int* y) const
{
	if (x != 0)
		(*x) = SRT[2] - 0x7f;
	if (y != 0)
		(*y) = SRT[3] - 0x7f;
}

DWORD FaceComponent::GetColor() const
{
	DWORD c = data[1];
	return (((c & 0x000f) << 4) + (((c >> 4) & 0x000f) << 12) + (((c >> 8) & 0x000f) << 20) + (((c >> 12) & 0x000f) << 28)) | 0x0f0f0f0f;
}

void FaceComponent::SetColor(DWORD color)
{
	data[1] = (WORD)(((color >> 4) & 0x0000000f) + ((color >> 8) & 0x000000f0) + ((color >> 12) & 0x00000f00) + ((color >> 16) & 0x0000f000));
}

int FaceComponent::GetStyle() const
{
	return (int)data[0];
}

void FaceComponent::SetStyle(int style)
{
	data[0] = style;
}

bool ParaEngine::FaceTextureComponent::CheckLoad()
{
	if (!name.empty())
	{
		if (!m_texture)
		{
			AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(name);
			if (pEntry)
				m_texture = CGlobals::GetAssetManager()->LoadTexture(pEntry->GetLocalFileName(), pEntry->GetLocalFileName());
			else
				m_texture = CGlobals::GetAssetManager()->LoadTexture(name, name);
		}
		if (m_texture)
		{
			if (!(m_texture->GetTexture()) || !m_texture->IsLoaded())
			{
				if (!m_texture->IsValid()){
					// skip invalid textures
					m_texture.reset();
					name.clear();
				}
				else
					return false;
			}
		}
	}
	return true;
}
