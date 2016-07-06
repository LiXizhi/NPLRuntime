//-----------------------------------------------------------------------------
// Class:	AssetEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2009.8.18: Implementation are moved to TextureEntity.cpp, ParaXEntity.cpp, MeshEntity.cpp
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "AsyncLoader.h"
#include "UrlLoaders.h"

#include "AssetEntity.h"

#ifdef PARAENGINE_CLIENT
	#include "memdebug.h"
#endif

using namespace ParaEngine;

/** the default asset server url. if "", asset server will be disabled. */
#define DEFAULT_ASSET_SERVER_URL		""
//#define DEFAULT_ASSET_SERVER_URL		"http://www.paraengine.com/"
static string g_sUrlAssetServer = DEFAULT_ASSET_SERVER_URL;


AssetEntity::AssetEntity() :m_bIsValid(true), m_bIsInitialized(false), m_bIsLocked(false), m_assetState(ASSET_STATE_NORMAL)
{

}

AssetEntity::AssetEntity(const AssetKey& key) : m_bIsValid(true), m_bIsInitialized(false), m_bIsLocked(false), m_key(key), m_assetState(ASSET_STATE_NORMAL)
{

}

AssetEntity::~AssetEntity()
{

}

bool AssetEntity::IsLoaded()
{
	return IsInitialized();
}

bool AssetEntity::IsInitialized()
{
	return m_bIsInitialized;
}

const string& AssetEntity::GetAssetServerUrl()
{
	return g_sUrlAssetServer;
}

void AssetEntity::SetAssetServerUrl(const char* pStr)
{
	if (pStr)
	{
		g_sUrlAssetServer = pStr;
	}
}

bool AssetEntity::Lock( int nMethod/*=0*/ )
{
	if(!m_bIsLocked)
	{
		m_bIsLocked = true;
		return true;
	}
	return m_bIsLocked;
}

void AssetEntity::UnLock()
{
	m_bIsLocked = false;
}

string AssetEntity::GetRemoteAssetURL()
{
	if (m_assetState != ASSET_STATE_REMOTE)
		return GetAssetServerUrl()+GetKey();
	else
		return GetKey();
}

const std::string g_http_cache_root = "temp/cache/";

std::string GetCachePath( const char* sFileUrl )
{
	if(sFileUrl)
		return g_http_cache_root + StringHelper::md5(sFileUrl);
	else
		return "";
}

namespace ParaEngine
{
	class CAssetEntityUserData : public CUrlProcessorUserData
	{
	public:
		CAssetEntityUserData(AssetEntity* pAssetFileEntry)
			: m_pAssetEntity(pAssetFileEntry){};

		virtual ~CAssetEntityUserData()
		{
		}
	public:
		AssetEntity* m_pAssetEntity;
	};

	DWORD AssetEntity_Request_CallBack(int nResult, CUrlProcessor* pRequest, CUrlProcessorUserData* lpUserData)
	{
		bool bSucceed = false;
		if(lpUserData)
		{
			CAssetEntityUserData * pData = (CAssetEntityUserData*) lpUserData;
			CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
			std::string url = pData->m_pAssetEntity->GetRemoteAssetURL();

			if(nResult == CURLE_OK && pRequest->m_responseCode == 200 && pRequest->GetData().size()>0)
			{
				
				CParaFile file;
				string tmpName = pData->m_pAssetEntity->GetLocalFileName();
				// save file to temp/cache directory.
				if(file.CreateNewFile(tmpName.c_str(), false))
				{
					file.write(&(pRequest->GetData()[0]), (int)(pRequest->GetData().size()));
					file.close();
					pAsyncLoader->log(string("AssetFile ASync Completed:")+url+"\n");
					bSucceed = true;
					pData->m_pAssetEntity->SetState(AssetEntity::ASSET_STATE_CACHE);
				}
				else
				{
					pAsyncLoader->log(string("AssetFile ASync Failed cannot save to disk:") + url + "\n");
				}
			}
			else
			{
				pAsyncLoader->log(string("AssetFile ASync Failed http code!=200:") + url + "\n");
			}
		}
		return 0;
	}
}

IAttributeFields* AssetEntity::GetAttributeObject()
{
	return this;
}

const std::string& AssetEntity::GetIdentifier()
{
	return m_key;
}

// this function is absoleted
bool AssetEntity::AutoSync(bool bForceSync, bool bLazyLoading)
{
	bool bHasCacheVersion = false;
	/**
	HTTP texture file name is first hashed (one directional and uniquely) to an encoded file name string. 

	we will first check if the file exist, if so we will use it. 
	if not, we will check if the file is being downloaded (in the downloading map), if so, ignore it. 
	if not, we will begin a file URL request and add the request url to downloading map. 

	Once, download is completed, we will remove the url from the downloading map. 
	if download is succeeded, we will reload the asset.
	*/

	if (m_assetState == ASSET_STATE_NORMAL)
	{
		// this prevent the sync to be called multiple times. 
		m_assetState = ASSET_STATE_LOCAL;
		const string& sKey = GetKey();
		if((sKey[0] == 'h' && sKey[1] == 't' && sKey[2] == 't' && sKey[3] == 'p' && sKey[4] == ':') || 
			(sKey[0] == 'f' && sKey[1] == 't' && sKey[2] == 'p' && sKey[3] == ':') )
		{
			m_assetState = ASSET_STATE_REMOTE;
		}

		if(m_assetState == ASSET_STATE_REMOTE || GetKey().find(':')==string::npos)
		{
			string sCachedFileName = GetCachePath(GetKey().c_str());
			string url = GetRemoteAssetURL();
			SetLocalFileName(sCachedFileName.c_str());
			if(!bForceSync && CParaFile::DoesFileExist(sCachedFileName.c_str(), false))
			{
				m_assetState = ASSET_STATE_CACHE;
				Refresh(sCachedFileName.c_str(), bLazyLoading);
				bHasCacheVersion = true;
			}
			else
			{
				// begin url request. 
				// we need to download from the web server. 
				CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

				if(pAsyncLoader->interruption_requested())
					return false;

				pAsyncLoader->log(string("AssetFile ASync Started:")+url+"\n");

				CUrlLoader* pLoader = new CUrlLoader();
				CUrlProcessor* pProcessor = new CUrlProcessor();

				pLoader->SetUrl(url.c_str());
				pProcessor->SetUrl(url.c_str());
				pProcessor->SetCallBack(AssetEntity_Request_CallBack, new CAssetEntityUserData(this), true);
				m_assetState = ASSET_STATE_SYNC_START;
				if(!pAsyncLoader->AddWorkItem( pLoader, pProcessor, NULL, NULL, ResourceRequestID_Web))
				{
					// error adding work item.
				}
			}
		}
	}
	return bHasCacheVersion;
}

void AssetEntity::Refresh( const char* sFilename/*=NULL*/,bool bLazyLoad /*= false*/ )
{
	if(sFilename != NULL && sFilename[0] != '\0')
		SetLocalFileName(sFilename);
	UnloadAsset();
	if(!bLazyLoad)
		LoadAsset();
}

const string& AssetEntity::GetLocalFileName()
{
	if(m_assetState == ASSET_STATE_NORMAL)
	{
		return (m_localfilename.empty()) ? GetKey() : m_localfilename;
	}
	return m_localfilename;
}

void AssetEntity::SetLocalFileName(const char* sFileName)
{
	if(sFileName)
		m_localfilename = sFileName;
}

int AssetEntity::Release()
{
	if (delref()) {
		int nRefCount = GetRefCount();
		Cleanup();
		delete this;
		return nRefCount;
	}
	return GetRefCount();
}

int AssetEntity::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("LocalFileName", FieldType_String, (void*)SetLocalFileName_s, (void*)GetLocalFileName_s, NULL, NULL, bOverride);
	pClass->AddField("RemoteAssetURL", FieldType_String, (void*)0, (void*)GetRemoteAssetURL_s, NULL, NULL, bOverride);
	pClass->AddField("IsValid", FieldType_Bool, (void*)0, (void*)IsValid_s, NULL, NULL, bOverride);
	pClass->AddField("IsLoaded", FieldType_Bool, (void*)0, (void*)IsLoaded_s, NULL, NULL, bOverride);
	pClass->AddField("IsLocked", FieldType_Bool, (void*)0, (void*)IsLocked_s, NULL, NULL, bOverride);
	pClass->AddField("IsInitialized", FieldType_Bool, (void*)0, (void*)IsInitialized_s, NULL, NULL, bOverride);
	pClass->AddField("State", FieldType_Int, (void*)SetState_s, (void*)GetState_s, NULL, NULL, bOverride);
	return S_OK;
}