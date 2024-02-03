//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.9
// Desc: partially cross platformed
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AssetEntity.h"
#include "ParameterBlock.h"
#include "ParaScriptingCommon.h"
#include "ZipArchive.h"
#include "ic/ICDBManager.h"
#include "TextureEntity.h"
#include "FileManager.h"
#include "NPL/NPLHelper.h"
#include "ParaWorldAsset.h"
#include "BufferPicking.h"
#include "ParaXModel/BoneAnimProvider.h"

#ifdef PARAENGINE_CLIENT
#include "DirectXEngine.h"
#endif

extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

using namespace ParaEngine;
using namespace ParaScripting;
using namespace luabind;

//////////////////////////////////////////////////////////////////////////
//
//ParaAssetObject
//
//////////////////////////////////////////////////////////////////////////
#pragma region ParaAssetObject
bool ParaScripting::ParaAssetObject::IsValid()
{
	return m_pAsset!=NULL && m_pAsset->IsValid();
}

bool ParaScripting::ParaAssetObject::IsLoaded()
{
	// either loaded or not valid will be considered, Loaded. 
	return m_pAsset!=NULL && (m_pAsset->IsLoaded() || ! m_pAsset->IsValid());
}

int ParaScripting::ParaAssetObject::GetType()
{
	if(m_pAsset)
	{
		return (int)(m_pAsset->GetType());
	}
	return  -1;
}


void ParaScripting::ParaAssetObject::LoadAsset()
{
	if(IsValid())
	{
		m_pAsset->LoadAsset();
	}
}

void ParaScripting::ParaAssetObject::Release()
{
	if(m_pAsset)
	{
		m_pAsset->Release();
		m_pAsset = NULL;
	}
}

int ParaScripting::ParaAssetObject::GetRefCount()
{
	if(m_pAsset)
	{
		return m_pAsset->GetRefCount();
	}
	return 0;
}

void ParaScripting::ParaAssetObject::GarbageCollectMe()
{
	if(IsValid())
	{
		return m_pAsset->GarbageCollectMe();
	}
}

bool ParaScripting::ParaAssetObject::equals(const ParaAssetObject obj) const
{
	return m_pAsset == obj.m_pAsset;
}

void ParaScripting::ParaAssetObject::Refresh()
{
	Refresh1(NULL);
}

void ParaScripting::ParaAssetObject::Refresh1(const char* sFilename)
{
	if (m_pAsset)
		m_pAsset->Refresh(sFilename, true);
}

bool ParaScripting::ParaAssetObject::Reload()
{
	if(IsValid())
	{
		if(m_pAsset->GetType()==AssetEntity::mesh)
		{
			m_pAsset->UnloadAsset();
			m_pAsset->LoadAsset();
			return true;
		}
	}
	return false;
}
string ParaScripting::ParaAssetObject::GetKeyName()
{
	if(IsValid())
	{
		return m_pAsset->GetKey();
	}
	return "";
}
const char* ParaScripting::ParaAssetObject::GetKeyName_()
{
	static string g_str;
	g_str = GetKeyName();
	return g_str.c_str();
}

string ParaScripting::ParaAssetObject::GetFileName()
{
	if(IsValid())
	{
		if(m_pAsset->GetType()==AssetEntity::texture)
		{
			return ((TextureEntity*)m_pAsset)->GetLocalFileName();
		}
		else if(m_pAsset->GetType()==AssetEntity::mesh)
		{
			return ((MeshEntity*)m_pAsset)->GetFileName();
		}
		else if(m_pAsset->GetType()==AssetEntity::parax)
		{
			return ((ParaXEntity*)m_pAsset)->GetFileName();
		}
	}

	return "";
}
const char* ParaScripting::ParaAssetObject::GetFileName_()
{
	static string g_str;
	g_str = GetFileName();
	return g_str.c_str();
}

void ParaAssetObject::SetTextureFPS(float FPS)
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		((TextureEntity*)m_pAsset)->SetTextureFPS(FPS);
	}
}

void ParaAssetObject::SaveTextureToPNG(const char * sFileName,int width,int height)
{
	if (IsValid() && m_pAsset->GetType() == AssetEntity::texture)
	{
		((TextureEntity*)m_pAsset)->SaveToFile(sFileName, D3DFMT_A8R8G8B8,width,height);
	}
}

void ParaAssetObject::EnableTextureAutoAnimation(bool bEnable)
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		((TextureEntity*)m_pAsset)->EnableTextureAutoAnimation(bEnable);
	}
}

void ParaAssetObject::SetCurrentFrameNumber(int nFrame)
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		((TextureEntity*)m_pAsset)->SetCurrentFrameNumber(nFrame);
	}
}

int ParaAssetObject::GetCurrentFrameNumber()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		return ((TextureEntity*)m_pAsset)->GetCurrentFrameNumber();
	}
	return 0;
}

int ParaAssetObject::GetFrameCount()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		return ((TextureEntity*)m_pAsset)->GetFrameCount();
	}
	return 0;
}

int ParaScripting::ParaAssetObject::GetWidth()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		const TextureEntity::TextureInfo* pInfo = ((TextureEntity*)m_pAsset)->GetTextureInfo();
		if(pInfo)
		{
			return pInfo->m_width;
		}
	}
	return 0;
}

int ParaScripting::ParaAssetObject::GetHeight()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		const TextureEntity::TextureInfo* pInfo = ((TextureEntity*)m_pAsset)->GetTextureInfo();
		if(pInfo)
		{
			return pInfo->m_height;
		}
	}
	return 0;
}

void ParaScripting::ParaAssetObject::SetSize( int nWidth, int nHeight )
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::texture)
	{
		((TextureEntity*)m_pAsset)->SetTextureInfo(TextureEntity::TextureInfo(nWidth, nHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
	}
}

int ParaScripting::ParaAssetObject::SetHandle( int nHandle )
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CGlobals::GetEffectManager()->MapHandleToEffect(nHandle, (CEffectFile*)m_pAsset);
		return nHandle;
	}
	return -1;
}

int ParaScripting::ParaAssetObject::GetHandle()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		return CGlobals::GetEffectManager()->GetEffectHandle((CEffectFile*)m_pAsset);
	}
	return -1;
}


ParaScripting::ParaParamBlock ParaScripting::ParaAssetObject::GetParamBlock()
{
	if(IsValid())
	{
		return ParaParamBlock(m_pAsset->GetParamBlock(true));
	}
	return ParaParamBlock(NULL);
}

object ParaScripting::ParaAssetObject::GetBoundingBox( const object& box )
{
	if(IsValid())
	{
		Vector3 vMin(-1,0,-1);
		Vector3 vMax(1,1,1);
		
		if (m_pAsset->GetBoundingBox(&vMin, &vMax))
		{
			box["min_x"] = vMin.x;
			box["min_y"] = vMin.y;
			box["min_z"] = vMin.z;

			box["max_x"] = vMax.x;
			box["max_y"] = vMax.y;
			box["max_z"] = vMax.z;
		}
	}
	return object(box);
}

bool ParaScripting::ParaAssetObject::Begin()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		// fixed: end any previous effect. 
		CGlobals::GetEffectManager()->EndEffect();

		CEffectFile* pEffect = (CEffectFile*) m_pAsset;
		pEffect->LoadAsset();
		if(pEffect->IsValid())
		{
			return pEffect->begin(true, 0, true);
		}
	}
	return false;
}

bool ParaScripting::ParaAssetObject::BeginPass( int nPass )
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CEffectFile* pEffect = (CEffectFile*) m_pAsset;
		return pEffect->BeginPass(nPass, true);
	}
	return false;
}

bool ParaScripting::ParaAssetObject::EndPass()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CEffectFile* pEffect = (CEffectFile*) m_pAsset;
		pEffect->EndPass(true);
		return true;
	}
	return false;
}

bool ParaScripting::ParaAssetObject::End()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CEffectFile* pEffect = (CEffectFile*) m_pAsset;
		pEffect->end(true);
		return true;
	}
	return false;
}

bool ParaScripting::ParaAssetObject::CommitChanges()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CEffectFile* pEffect = (CEffectFile*) m_pAsset;

		if(pEffect->GetParamBlock())
		{
			// apply per effect parameters. 
			pEffect->GetParamBlock()->ApplyToEffect(pEffect);
		}
		pEffect->CommitChanges();
		return true;
	}
	return false;
}

ParaScripting::ParaAttributeObject ParaScripting::ParaAssetObject::GetAttributeObject()
{
	return ParaAttributeObject(m_pAsset ? m_pAsset->GetAttributeObject() : NULL);
}

void ParaScripting::ParaAssetObject::UnloadAsset()
{
	if(m_pAsset)
	{
		m_pAsset->UnloadAsset();
		m_pAsset->Cleanup();
		
		// 2008.10.9: we will delete texture info for static texture as well. this corrects a bug when the dimension of the texture info changes.
		if(m_pAsset->GetType()==AssetEntity::texture)
		{
			TextureEntity* pTexture = ((TextureEntity*)m_pAsset);
			if(pTexture->SurfaceType == TextureEntity::StaticTexture)
			{
				SAFE_DELETE(pTexture->m_pTextureInfo);
			}
			// make valid again, because we will reload it
			if (!m_pAsset->IsValid() && m_pAsset->GetState() == AssetEntity::ASSET_STATE_FAILED_TO_LOAD)
			{
				m_pAsset->SetState(AssetEntity::ASSET_STATE_NORMAL);
				m_pAsset->m_bIsValid = true;
			}
		}
	}
}

int ParaScripting::ParaAssetObject::GetNumReplaceableTextures()
{
	if(IsValid() && m_pAsset->GetType()==AssetEntity::mesh)
	{
		return ((MeshEntity*)m_pAsset)->GetMesh()->GetNumReplaceableTextures();
	}
	return 0;
}

ParaAssetObject ParaScripting::ParaAssetObject::GetDefaultReplaceableTexture( int ReplaceableTextureID )
{
	ParaAssetObject texture;
	if(IsValid() && m_pAsset->GetType()==AssetEntity::mesh)
	{
		texture.m_pAsset = ((MeshEntity*)m_pAsset)->GetMesh()->GetDefaultReplaceableTexture(ReplaceableTextureID);
	}
	return texture;
}

void ParaScripting::ParaAssetObject::SetTexture( int nIndex, const char* filename )
{
#ifdef USE_DIRECTX_RENDERER
	if(IsValid() && m_pAsset->GetType()==AssetEntity::effectfile)
	{
		((CEffectFile*)m_pAsset)->UnLockTexture(nIndex);

		if(filename && filename[0]=='\0')
			CGlobals::GetRenderDevice()->SetTexture(nIndex, NULL);
		else
		{
			TextureEntity* pEntity = CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture);
			((CEffectFile*)m_pAsset)->setTexture(nIndex, pEntity);
		}
	}
#endif
}

TextureEntity * ParaScripting::ParaAssetObject::GetTextureEntity()
{
	if (IsValid() && m_pAsset->GetType() == AssetEntity::texture)
		return (TextureEntity *)m_pAsset;
	else
		return NULL;
}

AssetEntity * ParaScripting::ParaAssetObject::GetAssetEntity()
{
	return m_pAsset;
}

TextureEntity * ParaScripting::ParaAssetObject::GetTexture(const object& texture)
{
	using namespace luabind;
	TextureEntity* pTextureEntity = NULL;
	int nType = luabind::type(texture);
	if (nType == LUA_TSTRING){
		pTextureEntity = CGlobals::GetAssetManager()->LoadTexture("", NPL::NPLHelper::LuaObjectToString(texture));
	}
	else if (nType == LUA_TUSERDATA){
		try
		{
			ParaAssetObject textureObj = object_cast<ParaAssetObject>(texture);
			pTextureEntity = textureObj.GetTextureEntity();
		}
		catch (...)
		{
		}
	}
	return pTextureEntity;
}

void ParaScripting::ParaAssetObject::SetCallback(int callbackType,const char * func)
{
	if(IsValid()&&m_pAsset->GetType()==AssetEntity::effectfile)
	{
		CEffectFile* pEffect=(CEffectFile*)m_pAsset;
		pEffect->AddScriptCallback(callbackType,func);
	}
}

#pragma endregion ParaAssetObject
#pragma region ParaParamsBlock
/////////////////////////////////////////////////////////////
//
// ParaParamBlock
//
/////////////////////////////////////////////////////////////

bool ParaScripting::ParaParamBlock::IsValid()
{
	return (m_pParamBlock!=0);
}

void ParaScripting::ParaParamBlock::Clear()
{
	if(IsValid())
	{
		m_pParamBlock->Clear();
	}
}

void ParaScripting::ParaParamBlock::SetInt( const char* sParamName, int value )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, value);
	}
}

void ParaScripting::ParaParamBlock::SetBoolean( const char* sParamName, bool value )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, value);
	}
}

void ParaScripting::ParaParamBlock::SetFloat( const char* sParamName, float value )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, value);
	}
}

void ParaScripting::ParaParamBlock::SetVector2( const char* sParamName, float x, float y )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, Vector2(x,y));
	}
}

void ParaScripting::ParaParamBlock::SetVector3( const char* sParamName, float x, float y, float z )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, Vector3(x,y,z));
	}
}

void ParaScripting::ParaParamBlock::SetVector4( const char* sParamName, float x, float y, float z, float w )
{
	if(IsValid())
	{
		m_pParamBlock->SetParameter(sParamName, Vector4(x,y,z,w));
	}
}

void ParaScripting::ParaParamBlock::SetMatrix43( const char* sParamName, const char* matrix )
{
	if(IsValid())
	{
		Matrix4 mat; 
		if(sscanf(matrix, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
		{
			mat._14 = 0;
			mat._24 = 0;
			mat._34 = 0;
			mat._44 = 1;
		}
		else 
		{
			mat = Matrix4::IDENTITY;
		}
		mat = mat.transpose();
		m_pParamBlock->SetParameter(sParamName, mat);
	}
}

void ParaScripting::ParaParamBlock::SetParam( const char* sParamName, const char* sValue_ )
{
	if(IsValid())
	{
		m_pParamBlock->SetParamByStringValue(sParamName, sValue_);
	}
}

void ParaScripting::ParaParamBlock::SetTexture( int nTextureIndex, const char* sFilePath )
{
	if(IsValid())
	{
		if(nTextureIndex>=0 && nTextureIndex<=9 && sFilePath!=0)
		{
			string sParamName = "0";
			sParamName[0] = (char)('0'+nTextureIndex);
			TextureEntity* pEntity = CGlobals::GetAssetManager()->LoadTexture("", sFilePath, TextureEntity::StaticTexture);
			m_pParamBlock->SetParameter(sParamName, pEntity);
		}
		else
		{
			OUTPUT_LOG("warning: ParaParamBlock::SetTexture(%d, NULL texture path)\n", nTextureIndex);
		}
	}
}

void ParaScripting::ParaParamBlock::SetTexture_(const char* sParamName, const char* sFilePath)
{
	if(IsValid() && sParamName!=0 && sFilePath!=0)
	{
		TextureEntity* pEntity = CGlobals::GetAssetManager()->LoadTexture("", sFilePath, TextureEntity::StaticTexture);
		m_pParamBlock->SetParameter(sParamName, pEntity);
	}
}

void ParaScripting::ParaParamBlock::SetTextureObj( int nTextureIndex, const ParaAssetObject& assetObject )
{
	if(IsValid())
	{
		if(nTextureIndex>=0 && nTextureIndex<=9)
		{
			string sParamName = "0";
			sParamName[0] = (char)('0'+nTextureIndex);
			TextureEntity* pEntity = (TextureEntity*)(assetObject.m_pAsset);
			m_pParamBlock->SetParameter(sParamName, pEntity);
		}
	}
}


#pragma endregion ParaParamsBlock
#pragma region ParaAssets
//--------------------------------------------------------------
// for ParaAsset namespace table.
//--------------------------------------------------------------

bool ParaAsset::OpenArchive(const char* strFileName)
{
	if(strFileName!=NULL)
		return CGlobals::GetFileManager()->OpenArchive(strFileName);
	else
		return false;
}

bool ParaAsset::GeneratePkgFile_(const char* srcZip, const char* destPkg, int nVersion)
{
	bool bRes = false;
	CZipArchive* pArchive = new CZipArchive(); // TODO
	if (pArchive != NULL)
	{
		if (pArchive->Open(srcZip, 0))
		{
			string pkgFile = CParaFile::ChangeFileExtension(srcZip, "pkg");
			if (destPkg == 0)
			{
				destPkg = pkgFile.c_str();
			}

			// close the archive, 
			// TODO: shall we reopen it after done with generation. 
			CGlobals::GetFileManager()->CloseArchive(destPkg);
			CGlobals::GetFileManager()->CloseArchive(CParaFile::ChangeFileExtension(destPkg, "zip"));

			if (nVersion == 1)
				bRes = pArchive->GeneratePkgFile(destPkg);
			else if (nVersion == 2)
				bRes = pArchive->GeneratePkgFile2(destPkg);

			if (!bRes)
			{
				OUTPUT_LOG("warning: failed generating pkg file from %s. Make sure the pkg file is not in use.\n", srcZip);
			}
		}
		SAFE_DELETE(pArchive);
	}
	if (!bRes)
	{
		OUTPUT_LOG("warning: zip file %s is not found\n", srcZip);
	}
	return bRes;
}

bool ParaAsset::GeneratePkgFile( const char* srcZip, const char* destPkg )
{
	// default to version 2
	return GeneratePkgFile_(srcZip, destPkg, 2);
}

bool ParaAsset::OpenArchive2(const char* strFileName, bool bUseRelativePath)
{
	if(strFileName!=NULL)
		return CGlobals::GetFileManager()->OpenArchive(strFileName, bUseRelativePath);
	else
		return false;
}

bool ParaAsset::OpenArchiveEx( const char* strFileName, const char* sRootDir )
{
	if(strFileName!=NULL && sRootDir!=NULL)
		return CGlobals::GetFileManager()->OpenArchiveEx(strFileName, sRootDir);
	else
		return false;
}
void ParaAsset::CloseArchive(const string&  path)
{
	CGlobals::GetFileManager()->CloseArchive(path);
}

void ParaAsset::Init()
{
	/** you can call "init" as many times as you like. Asset Manager guaranteed
	that if a object is valid, it will not be reinitialied */
	CGlobals::GetAssetManager()->InitDeviceObjects();
	/// we needs to call RestoreDeviceObjects() to initialize device dependence
	/// object like the local mesh in the Mesh entity.
	CGlobals::GetAssetManager()->RestoreDeviceObjects();
}
void ParaAsset::Unload(const char* assettype)
{
	if(assettype[0] == '*')
		UnloadAll();
	else
	{
		//TODO:
	}
}

void ParaAsset::UnloadDatabase()
{
	ParaInfoCenter::CICDBManager::Finalize();
}

void ParaAsset::UnloadAll()
{
	CGlobals::GetAssetManager()->UnloadAsset();
}
void ParaAsset::GarbageCollect()
{
	CGlobals::GetAssetManager()->GarbageCollectAll();
}

ParaAssetObject ParaAsset::LoadParaX(const char* strAssetName, const char* strFilePath)
{
	if(strAssetName!=0 && strFilePath!=0)
	{
		AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadParaX(strAssetName, strFilePath);
		return  ParaAssetObject(pAssetEntity);
	}
	else
	{
		return ParaAssetObject();
	}
}

ParaAssetObject ParaAsset::LoadStaticMesh(const char* strAssetName, const char* strFilePath)
{
	if(strAssetName!=0 && strFilePath!=0)
	{
		int len = (int)strlen(strFilePath);
#ifdef USE_DIRECTX_RENDERER
		/*
		if( (strFilePath[len-4] == 'i' && strFilePath[len-3] == 'g'
			&& strFilePath[len-2] == 'e' && strFilePath[len-1] == 's')||
			(strFilePath[len-4] == 's' && strFilePath[len-3] == 't'
			&& strFilePath[len-2] == 'e' && strFilePath[len-1] == 'p'))
			*/
		if(CadModel::IsStepFile(strFilePath) || CadModel::IsIgesFile(strFilePath))
		{
			AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadCadModel(strAssetName, strFilePath);
			return  ParaAssetObject(pAssetEntity);
		}
		else
#endif
		{
			AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadMesh(strAssetName, strFilePath);
			return  ParaAssetObject(pAssetEntity);
		}
	}
	else
	{
		return ParaAssetObject();
	}
}


ParaScripting::ParaAssetObject ParaScripting::ParaAsset::LoadPickingBuffer(const char* strAssetName)
{
	AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadBufferPick(strAssetName);
	return ParaAssetObject(pAssetEntity);
}

ParaAssetObject ParaAsset::LoadTexture(const char* strAssetName, const char* strFilePath, int nSurfaceType)
{
	if(nSurfaceType<0)
		nSurfaceType = (int)(TextureEntity::StaticTexture);

	if(strAssetName!=0 && strFilePath!=0)
	{
		AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadTexture(strAssetName, strFilePath, (TextureEntity::_SurfaceType)nSurfaceType);
		return  ParaAssetObject(pAssetEntity);
	}
	else
	{
		return ParaAssetObject();
	}
}


ParaScripting::ParaAssetObject ParaAsset::LoadRenderTarget( const char* strAssetName, int width, int height )
{
	TextureEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadTexture(strAssetName, strAssetName, TextureEntity::RenderTarget);
	if(pAssetEntity)
	{
		if(pAssetEntity->SurfaceType != TextureEntity::RenderTarget)
		{
			CGlobals::GetAssetManager()->GetTextureManager().DeleteEntity(pAssetEntity);
			pAssetEntity = CGlobals::GetAssetManager()->LoadTexture(strAssetName, strAssetName, TextureEntity::RenderTarget);
		}
		if(pAssetEntity)
		{
			pAssetEntity->SetTextureInfo(TextureEntity::TextureInfo(width, height, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
		}
	}
	return  ParaAssetObject(pAssetEntity);
}

ParaAssetObject ParaAsset::LoadSpriteFrame(const char* strAssetName, int nFrames, int nRow, int nCol)
{
#ifdef PARAENGINE_CLIENT
	AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadD3DXSprite(strAssetName, nFrames, nRow, nCol);
	return  ParaAssetObject(pAssetEntity);
#else
	return ParaAssetObject();
#endif
}
ParaAssetObject ParaAsset::LoadFont(const char* strAssetName, const char* FontName, int nFontSize)
{
	AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadGDIFont(strAssetName, FontName, nFontSize);
	return  ParaAssetObject(pAssetEntity);
}
ParaAssetObject ParaAsset::LoadImageFont(const char* strAssetName, const char* TextureName, int nSize, int nTxtColumns)
{
	return ParaAssetObject(NULL);
}
ParaAssetObject ParaAsset::LoadSound(const char* strAssetName, const char* strFilePath, bool bInit)
{
	//AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadSound(strAssetName, strFilePath, bInit);
	//return  ParaAssetObject(pAssetEntity);
	return NULL;
}

ParaScripting::ParaAssetObject ParaAsset::LoadDatabase( const char* strAssetName, const char* strFilePath )
{
	AssetEntity* pAssetEntity = CGlobals::GetAssetManager()->LoadDatabase(strAssetName, strFilePath);
	return  ParaAssetObject(pAssetEntity);
}

int ParaAsset::GetBoneAnimProviderIDByName( const char* sName )
{
	return CBoneAnimProvider::GetProviderIDByName(sName);
}

const char* ParaAsset::GetBoneAnimProviderFileNameByID(int nAnimID)
{
	CBoneAnimProvider* pProvider = CBoneAnimProvider::GetProviderByID(nAnimID);
	if(pProvider!=NULL)
	{
		return pProvider->GetAnimFileName().c_str();
	}
	return NULL;
}

int ParaAsset::CreateBoneAnimProvider( int nAnimID, const char* name, const char* filename, bool bOverwrite )
{
	CBoneAnimProvider* pProvider = CBoneAnimProvider::CreateProvider(nAnimID, name, filename, bOverwrite);
	return (pProvider!=NULL) ? pProvider->GetAnimID() : -1;
}

bool ParaAsset::DeleteBoneAnimProvider( int nAnimID )
{
	return CBoneAnimProvider::DeleteProvider(nAnimID);
}

ParaScripting::ParaAssetObject ParaAsset::LoadEffectFile( const char* strAssetName, const char* strFilePath )
{
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetByName(strAssetName);
	if(pEffect == 0)
		pEffect = CGlobals::GetAssetManager()->LoadEffectFile(strAssetName, strFilePath);//"shaders/simple_mesh_normal_low.fx"

	return ParaAssetObject(pEffect);
}

ParaScripting::ParaAssetObject ParaAsset::GetEffectFile( const char* strAssetName )
{
	CEffectFile* pEffect = CGlobals::GetEffectManager()->GetByName(strAssetName);
	return ParaAssetObject(pEffect);
}

ParaScripting::ParaAssetObject ParaAsset::GetBlockMaterial(int32_t materialId)
{
	return ParaAssetObject(CGlobals::GetBlockMaterialManager()->GetEntity(std::to_string(materialId)));
}

ParaScripting::ParaAssetObject ParaAsset::CreateBlockMaterial()
{
	return ParaAssetObject(CGlobals::GetBlockMaterialManager()->CreateBlockMaterial());
}

ParaScripting::ParaAssetObject ParaAsset::CreateGetBlockMaterial(int32_t materialId)
{
	return ParaAssetObject(CGlobals::GetBlockMaterialManager()->CreateGetBlockMaterialByID(materialId));
}

void ParaAsset::DeleteBlockMaterial(ParaScripting::ParaAssetObject asset)
{
	auto mgr = CGlobals::GetBlockMaterialManager();
	auto material = mgr->GetEntity(asset.GetKeyName());
	if (material) mgr->DeleteEntity(material);
}

int ParaAsset::PrintToFile( const char* strFileName, DWORD dwSelection )
{
#ifdef PARAENGINE_CLIENT
	string sFileName = (strFileName == NULL || strFileName[0] == '\0') ?  "temp/assets.txt" : strFileName;
	ParaEngine::CParaFile file;
	int nCount = 0;
	if(file.CreateNewFile(sFileName.c_str()))
	{
		nCount = CGlobals::GetAssetManager()->PrintToFile(&file, dwSelection);
	}
	return nCount;
#else
	return 0;
#endif
}

void ParaAsset::AddFontName( const string& sLocalName, const string& sTypeFaceName )
{
	ParaEngine::SpriteFontEntity::AddFontName(sLocalName, sTypeFaceName);
}

const char* ParaAsset::GetAssetServerUrl()
{
	return AssetEntity::GetAssetServerUrl().c_str();
}

void ParaAsset::SetAssetServerUrl( const char* path )
{
	AssetEntity::SetAssetServerUrl(path);
}

bool ParaAsset::Refresh( const char* filename )
{
	return CGlobals::GetAssetManager()->RefreshAsset(filename);
}

#pragma endregion ParaAssets
