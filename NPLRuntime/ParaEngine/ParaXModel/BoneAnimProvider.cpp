//-----------------------------------------------------------------------------
// Class:	CBoneAnimProvider
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.8.27
// Revised: 
// Note: Supplying external animation loaded from ParaX or BVH files. Such animation can be shared by any animation instances. 
// Providers can also be loaded dynamically; some special animation provider can be composed by user at runtime; some may be 
// automatically generated from the physics engine. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <algorithm>
#include "ParaWorldAsset.h"
#include "ParaXAnimInstance.h"
#include "ParaXBone.h"
#include "ParaXModel.h"
#include "DynamicAttributeField.h"
#include "BoneAnimProvider.h"
#include "util/StringHelper.h"

#include "memdebug.h"
/** auto gen anim ID*/
#define FIRST_AUTO_INC_ANIM_ID	10000

using namespace ParaEngine;
using namespace std;

namespace ParaEngine
{
	map<int, CBoneAnimProvider*> CBoneAnimProvider::g_mapProviderIDs;
	map<string, CBoneAnimProvider*> CBoneAnimProvider::g_mapProviderNames;

	class CAutoRelease
	{
	public:
		~CAutoRelease()
		{
			CBoneAnimProvider::CleanupAllProviders();
		}
	};

	CAutoRelease g_dummy;
}

CBoneAnimProvider::CBoneAnimProvider(int nAnimID, const char* name, const char* filename)
:m_bLoaded(false)
{
	m_nAnimID = nAnimID;
	m_nSubAnimID = 0;
	if(name)
		m_sName = name;
	if(filename)
		m_sFileName = filename;

	for (int i=0; i<MAX_KNOWN_BONE_NODE;++i)
		m_boneLookup[i] = -1;
	
	m_MergeMode = MergeMode_ReplaceExisting;

	std::string  sExt = CParaFile::GetFileExtension(m_sFileName);
	StringHelper::make_lower(sExt);
	if (sExt == "x" || sExt == "fbx" || sExt == "xml")
		LoadFromParaXFile(m_sFileName.c_str());
	else
		LoadFromBVHFile(m_sFileName.c_str());
}

CBoneAnimProvider::~CBoneAnimProvider(void)
{
	m_bones.clear();
	m_anims.clear();
}

CBoneAnimProvider* ParaEngine::CBoneAnimProvider::GetProviderByID( int id )
{
	map<int, CBoneAnimProvider*>::iterator it = g_mapProviderIDs.find(id);
	if(it!=g_mapProviderIDs.end())
	{
		it->second->LoadAsset();
		return it->second;
	}
	return NULL;
}

int ParaEngine::CBoneAnimProvider::GetProviderIDByName( const char* sName )
{
	if(sName)
	{
		map<string, CBoneAnimProvider*>::iterator it = g_mapProviderNames.find(sName);
		if(it!=g_mapProviderNames.end())
		{
			if((it->second)!=NULL)
			{
				(it->second)->LoadAsset();
				return (it->second)->GetAnimID();	
			}
		}
	}
	return -1;
}

CBoneAnimProvider* ParaEngine::CBoneAnimProvider::CreateProvider( int nAnimID, const char* name, const char* filename, bool bOverwrite )
{
	if(!bOverwrite)
	{
		int nID = GetProviderIDByName(name);
		if(nID<0)
		{
			nID = nAnimID;
		}

		CBoneAnimProvider* pProvider = GetProviderByID(nID);
		if(pProvider)
			return pProvider;
	}
	if(nAnimID<0 )
	{
		static int AutoIncID = FIRST_AUTO_INC_ANIM_ID;
		nAnimID = AutoIncID;
		AutoIncID++;
	}
	CBoneAnimProvider* pProvider = new CBoneAnimProvider(nAnimID, name, filename);
	if(pProvider)
	{
		DeleteProvider(nAnimID);
		g_mapProviderIDs[nAnimID] = pProvider;
		if(name)
			g_mapProviderNames[name] = pProvider;
	}
	return pProvider;
}


bool ParaEngine::CBoneAnimProvider::CleanupAllProviders()
{
	map<int, CBoneAnimProvider*>::iterator itCur, itEnd =  g_mapProviderIDs.end();
	for (itCur = g_mapProviderIDs.begin(); itCur!=itEnd; ++itCur)
	{
		delete (itCur->second);
	}
	g_mapProviderIDs.clear();
	g_mapProviderNames.clear();
	return true;
}

bool ParaEngine::CBoneAnimProvider::DeleteProvider( int nAnimID )
{
	CBoneAnimProvider* pProvider = GetProviderByID(nAnimID);
	if(pProvider)
	{
		{
			map<int, CBoneAnimProvider*>::iterator it = g_mapProviderIDs.find(nAnimID);
			g_mapProviderIDs.erase(it);
		}

		{
			map<string, CBoneAnimProvider*>::iterator it = g_mapProviderNames.find(pProvider->GetAnimName());
			if(it!=g_mapProviderNames.end())
			{
				if((it->second)==pProvider)
				{
					g_mapProviderNames.erase(it);
				}
			}
		}
		delete pProvider;
	}
	return false;
}

AnimIndex CBoneAnimProvider::GetAnimIndex(int nAnimID)
{
	if(!LoadAsset())
	{
		// this is tricky, if an animation is still being loaded, we shall play an empty undetermined animation for 1 seconds
		// so, if the animation is loaded within 1 seconds, the ParaXAnimInstance will reload AnimIndex structure.
		return AnimIndex(m_nAnimID, 1, 0, 1000, 2 /* make undetermined */);
	}
	int nAnim =(int)m_anims.size();
	for (int i=0; i<nAnim ; i++) {
		if (m_anims[i].animID == nAnimID) 
		{
			return AnimIndex(m_nAnimID, 1, m_anims[i].timeStart, m_anims[i].timeEnd, (unsigned char)m_anims[i].loopType);
		}
	}
	
	return AnimIndex(-1);
}

void CBoneAnimProvider::GetAnimMoveSpeed(float* pSpeed, int nSubAnimID)
{
	int nAnim =(int)m_anims.size();
	for (int i=0; i<nAnim ; i++) {
		if (m_anims[i].animID == nSubAnimID) 
		{
			*pSpeed = m_anims[i].moveSpeed;
			break;
		}
	}
}

bool CBoneAnimProvider::LoadAsset()
{
	if(m_bLoaded)
		return true;
	ParaXEntity* pAsset = m_asset.get();
	if(pAsset && pAsset->GetPrimaryTechniqueHandle() > 0)
	{
		CParaXModel* pModel = pAsset->GetModel();
		if(pModel)
		{
			m_asset.reset();
			m_bLoaded = true;

			// copy bone look up table
			memcpy(m_boneLookup, pModel->m_boneLookup, sizeof(m_boneLookup));

			// copy bones
			int nBones = pModel->GetObjectNum().nBones;
			m_bones.resize(nBones);
			for (int i=0;i<nBones;++i)
			{
				m_bones[i] = pModel->bones[i];
			}

			// copy animation sequence info
			int nAnimations = pModel->GetObjectNum().nAnimations;
			m_anims.resize(nAnimations);
			for (int i=0;i<nAnimations ;++i)
			{
				m_anims[i] = pModel->anims[i];
			}
			return true;
		}
	}

	return m_bLoaded;
}


bool ParaEngine::CBoneAnimProvider::LoadFromBVHFile( const char* filename )
{
	// TODO: please refer to BVH format and my old BVH serialization code. 
	return false;
}

bool ParaEngine::CBoneAnimProvider::LoadFromParaXFile( const char* filename )
{
	// just copy data from the ParaXModel. 
	// TODO: Shall we release the ParaX model after get its data? Sometimes, the ParaX model can be displayed for testing bone animations.

	ParaXEntity* pAsset = m_asset.get();
	if(pAsset == NULL)
	{
		pAsset = CGlobals::GetAssetManager()->LoadParaX("", filename);
		pAsset->LoadAsset();
		m_asset = pAsset;
	}
	return LoadAsset();
}

Bone* ParaEngine::CBoneAnimProvider::GetBoneByIndex(int nIndex)
{
	return (nIndex >= 0 && nIndex < (int)(m_bones.size())) ? &(m_bones[nIndex]) : NULL;
}

Bone* ParaEngine::CBoneAnimProvider::GetBone(KNOWN_BONE_NODES KnownBoneID)
{
	int nIndex = m_boneLookup[KnownBoneID];
	return (nIndex >= 0) ? &(m_bones[nIndex]) : NULL;
}

bool ParaEngine::CBoneAnimProvider::HasBone(KNOWN_BONE_NODES KnownBoneID)
{
	return m_boneLookup[KnownBoneID] >= 0;
}

const string& ParaEngine::CBoneAnimProvider::GetAnimFileName()
{
	return m_sFileName;
}

const string& ParaEngine::CBoneAnimProvider::GetAnimName()
{
	return m_sName;
}

int ParaEngine::CBoneAnimProvider::GetAnimID()
{
	return m_nAnimID;
}

void ParaEngine::CBoneAnimProvider::SetSubAnimID(int nID)
{
	m_nSubAnimID = nID;
}

int ParaEngine::CBoneAnimProvider::GetSubAnimID()
{
	return m_nSubAnimID;
}

ParaEngine::CBoneAnimProvider::MergeMode ParaEngine::CBoneAnimProvider::GetMergeMode()
{
	return m_MergeMode;
}
