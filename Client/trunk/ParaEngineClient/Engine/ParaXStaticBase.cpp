#include "ParaEngine.h"
#include "ParaXStaticBase.h"
#include "ParaWorldAsset.h"

#include "SortedFaceGroups.h"
#include "util/StringHelper.h"

namespace ParaEngine
{
	CParaXStaticBase::CParaXStaticBase(const char* strName)
		: m_pFileData(nullptr)
		, m_pFileDataSize(0)
		, m_bUseMaterials(true)
		, m_dwNumMaterials(0)
	{
		if (strName)
			m_strName = strName;
	}

	CParaXStaticBase::~CParaXStaticBase()
	{
		Destroy();
	}


	int CParaXStaticBase::GetNumReplaceableTextures()
	{
		return (int)m_ReplaceableTextures.size();
	}


	TextureEntity* CParaXStaticBase::GetReplaceableTexture(int ReplaceableTextureID)
	{
		if (ReplaceableTextureID >= 0 && (int)m_ReplaceableTextures.size()>ReplaceableTextureID)
			return	m_ReplaceableTextures[ReplaceableTextureID].get();
		else
			return nullptr;
	}

	bool CParaXStaticBase::SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity)
	{
		if ((int)m_ReplaceableTextures.size()>ReplaceableTextureID && ReplaceableTextureID >= 0)
		{
			m_ReplaceableTextures[ReplaceableTextureID] = pTextureEntity;
			return true;
		}
		else
		{
			return false;
		}
	}

	int CParaXStaticBase::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		// install parent fields if there are any. Please replace __super with your parent class name.
		IAttributeFields::InstallFields(pClass, bOverride);

		assert(pClass != nullptr);
		pClass->AddField("TextureUsage", FieldType_String, nullptr, (void*)DumpTextureUsage_s, nullptr, nullptr, bOverride);
		pClass->AddField("PolyCount", FieldType_Int, nullptr, (void*)GetPolyCount_s, nullptr, nullptr, bOverride);
		pClass->AddField("PhysicsCount", FieldType_Int, nullptr, (void*)GetPhysicsCount_s, nullptr, nullptr, bOverride);
		return S_OK;
	}

	void CParaXStaticBase::SetMeshFileData(void * pData, int nSize)
	{
		DeleteMeshFileData();
		m_pFileData = (char*)pData;
		m_pFileDataSize = nSize;
	}
	void CParaXStaticBase::DeleteMeshFileData()
	{
		SAFE_DELETE_ARRAY(m_pFileData);
		m_pFileDataSize = 0;
	}

	const ParaXRefObject* CParaXStaticBase::GetXRefObjectsByIndex(int nIndex)
	{
		return (nIndex >= 0 && nIndex< GetXRefObjectsCount()) ? &(m_XRefObjects[nIndex]) : nullptr;
	}

	void CParaXStaticBase::ClearFaceGroups()
	{
		//////////////////////////////////////////////////////////////////////////
		// delete all face group. 
		for (int i = 0; i<(int)m_faceGroups.size(); ++i)
		{
			SAFE_DELETE(m_faceGroups[i]);
		}
		m_faceGroups.clear();
	}

	TextureEntity*	CParaXStaticBase::LoadTextureByPath(const string& sFilePath, DWORD TextureType)
	{
		TextureEntity* pTex = NULL;
		// Initialize textures from texture file
		if (!CParaFile::IsFileName(sFilePath))
			pTex = CGlobals::GetAssetManager()->LoadTexture("", sFilePath, (TextureEntity::_SurfaceType)TextureType);
		else
		{
			string sNewTexPath = CParaFile::GetAbsolutePath(sFilePath, CParaFile::GetParentDirectoryFromPath(m_strName));
			pTex = CGlobals::GetAssetManager()->LoadTexture("", sNewTexPath, (TextureEntity::_SurfaceType)TextureType);
		}
		return pTex;
	}

	HRESULT CParaXStaticBase::Create(RenderDevicePtr pd3dDevice, const char* strFilename, bool bCheckSecondUV)
	{
		// Initialize textures from texture file
		if (strFilename != NULL)
			m_strName = strFilename;

		if (m_pFileData)
		{
			return Create(pd3dDevice, (char*)m_pFileData, m_pFileDataSize, bCheckSecondUV);
		}
		else
		{
			CParaFile myFile(m_strName.c_str());
			char* buffer = myFile.getBuffer();
			DWORD nFileSize = (DWORD)myFile.getSize();
			if (buffer == 0)
				return E_FAIL;
			return Create(pd3dDevice, buffer, nFileSize, bCheckSecondUV);
		}
	}

	HRESULT CParaXStaticBase::Destroy()
	{
		m_ReplaceableTextures.clear();

		ClearFaceGroups();

		DeleteMeshFileData();

		m_dwNumMaterials = 0L;

		DeleteDeviceObjects();

		return S_OK;
	}

	XModelFileType CParaXStaticBase::GetFileTypeByFilename(const std::string& sFilename)
	{
		XModelFileType nFileType = FileType_ParaX;

		std::string sExt = CParaFile::GetFileExtension(sFilename);
		StringHelper::make_lower(sExt);

		if (sExt == "fbx")
		{
			nFileType = FileType_FBX;
		}

		return nFileType;
	}


} // end namespace