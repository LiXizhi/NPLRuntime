//-----------------------------------------------------------------------------
// Class: ParaTerrain	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.25
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "KidsDBProvider.h"
#include "NpcDatabase.h"
#endif
#include "DataProviderManager.h"
#include "AttributeProvider.h"
#include "SelectionManager.h"
#include "2dengine/GUIRoot.h"
#include "WorldInfo.h"
#include "ic/ICDBManager.h"
#include "terrain/WorldNameFactory.h"
#include "terrain/GlobalTerrain.h"

#include "ParaScriptingWorld.h"

extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace ParaScripting;
using namespace ParaEngine;
using namespace luabind;

#ifndef MAX_LINE
#define MAX_LINE	500
#endif

#include "memdebug.h"

#ifdef USE_DIRECTX_RENDERER
//////////////////////////////////////////////////////////////////////////
//
// ParaKidsDataProvider
//
//////////////////////////////////////////////////////////////////////////

ParaKidsDataProvider::~ParaKidsDataProvider()
{
}

bool ParaKidsDataProvider::InsertPuzzleRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertPuzzleRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeletePuzzleRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeletePuzzleRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertItemRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertItemRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::UpdateItemRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->UpdateItemRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteItemRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteItemRecordByID(ID);
	else
		return false;
}

const char * ParaKidsDataProvider::SelectItemRecordToString(__int64 ID) const
{
	if(m_pObj != NULL)
		return m_pObj->SelectItemRecordToString(ID);
	else
		return "";
}

bool ParaKidsDataProvider::InsertCharacterRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertCharacterRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteCharacterRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteCharacterRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertChestRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertChestRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteChestRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteChestRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertFruitRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertFruitRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteFruitRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteFruitRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertPetAIRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertPetAIRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeletePetAIRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeletePetAIRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertPetRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertPetRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeletePetRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeletePetRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertQuestRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertQuestRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteQuestRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteQuestRecordByID(ID);
	else
		return false;
}

bool ParaKidsDataProvider::InsertTitleRecordFromString(const char* strRecord)
{
	if(m_pObj != NULL)
		return m_pObj->InsertTitleRecordFromString(strRecord);
	else
		return false;
}
bool ParaKidsDataProvider::DeleteTitleRecordByID(int ID)
{
	if(m_pObj != NULL)
		return m_pObj->DeleteTitleRecordByID(ID);
	else
		return false;
}
#endif
//////////////////////////////////////////////////////////////////////////
//
// ParaDataProvider
//
//////////////////////////////////////////////////////////////////////////

bool ParaDataProvider::InsertPuzzleRecordFromString(const char* strRecord)
{
#ifdef USE_DIRECTX_RENDERER
	if (IsValid())
	{
		return m_pDbKidsProvider->InsertPuzzleRecordFromString(strRecord);
	}
	else
#endif
		return false;
}

bool ParaDataProvider::DeletePuzzleRecordByID(int ID)
{
#ifdef USE_DIRECTX_RENDERER
	if (IsValid())
	{
		return m_pDbKidsProvider->DeletePuzzleRecordByID(ID);
	}
	else
#endif
		return false;
}
#ifdef USE_DIRECTX_RENDERER
void ParaDataProvider::GetKidsDBProvider(ParaKidsDataProvider* pOut)
{
	pOut->m_pObj = CGlobals::GetDataProviderManager()->GetKidsDBProvider();
}
#endif

int ParaDataProvider::GetNPCIDByName(const char* name)
{
#ifdef USE_DIRECTX_RENDERER
	if (IsValid())
	{
		return m_pDbNPC->GetNPCIDByName(name);
	}
	else
#endif
		return -1;
}


int ParaDataProvider::GetNPCCount()
{
#ifdef USE_DIRECTX_RENDERER
	if (IsValid())
	{
		return m_pDbNPC->GetNPCCount();
	}
	else
#endif
		return -1;
}

bool ParaDataProvider::DoesAttributeExists(const char*  sName)
{
	if (IsValid())
		return m_pAttributeProvider->DoesAttributeExists(sName);
	else
		return false;
}

object ParaDataProvider::GetAttribute(const char* sName, const object& sOut)
{
	if (!IsValid())
		return object(sOut);
	
	int nType = type(sOut);
	if( nType== LUA_TNUMBER )
	{
		string value;
		if(m_pAttributeProvider->GetAttribute(sName, value))
		{
			float number=0;
			if (sscanf(value.c_str(), "%f", &number)>0)
			{
				return object(sOut.interpreter(),(double)number);
			}
		}
	}
	else if (nType == LUA_TSTRING)
	{
		string value;
		if(m_pAttributeProvider->GetAttribute(sName, value))
		{
			return object(sOut.interpreter(), value);
		}
	}
	else if(nType == LUA_TBOOLEAN)
	{
		string value;
		if(m_pAttributeProvider->GetAttribute(sName, value))
		{
			return object(sOut.interpreter(),(bool)(value != "false" && value !="nil"));
		}
	}
	return object(sOut);
}

bool ParaDataProvider::UpdateAttribute(const char* sName, const object& sIn)
{
	if (!IsValid())
		return false;
	bool result = false;

	int nType = type(sIn);
	if( nType== LUA_TNUMBER )
	{
		float value = object_cast<float>(sIn);
		char tmp[101];
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, 100, "%f", value);
		result = m_pAttributeProvider->UpdateAttribute(sName, tmp);
	}
	else if (nType == LUA_TSTRING)
	{
		result = m_pAttributeProvider->UpdateAttribute(sName, object_cast<const char*>(sIn));
	}
	else if(nType == LUA_TBOOLEAN)
	{
		bool value = object_cast<bool>(sIn);
		result = m_pAttributeProvider->UpdateAttribute(sName, value?"true":"false");
	}

	return result;
}

bool ParaDataProvider::InsertAttribute(const char* sName, const object& sIn)
{
	if (!IsValid())
		return false;
	bool result = false;

	int nType = type(sIn);
	if( nType== LUA_TNUMBER )
	{
		float value = object_cast<float>(sIn);
		char tmp[101];
		memset(tmp, 0, sizeof(tmp));
		snprintf(tmp, 100, "%f", value);
		result = m_pAttributeProvider->InsertAttribute(sName, tmp);
	}
	else if (nType == LUA_TSTRING)
	{
		result = m_pAttributeProvider->InsertAttribute(sName, object_cast<const char*>(sIn));
	}
	else if(nType == LUA_TBOOLEAN)
	{
		bool value = object_cast<bool>(sIn);
		result = m_pAttributeProvider->InsertAttribute(sName, value?"true":"false");
	}

	return result;
}

bool ParaDataProvider::DeleteAttribute(const char* sName)
{
	if (IsValid())
		return m_pAttributeProvider->DeleteAttribute(sName);
	else
		return false;
}

void ParaDataProvider::SetTableName(const char* sName)
{
	if (IsValid())
		m_pAttributeProvider->SetTableName(sName);
}

const char* ParaDataProvider::GetTableName()
{
	if (IsValid())
		return m_pAttributeProvider->GetTableName().c_str();
	else
		return CGlobals::GetString(G_STR_EMPTY).c_str();
}


bool ParaScripting::ParaDataProvider::ExecSQL( const char* sCmd )
{
	if (IsValid() && sCmd!=NULL)
	{
		if(m_pAttributeProvider->GetDBEntity())
		{
			m_pAttributeProvider->GetDBEntity()->ExecuteSqlScript(sCmd);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
// ParaWorld
//
//////////////////////////////////////////////////////////////////////////

ParaWorld::ParaWorld(void)
{
}

ParaWorld::~ParaWorld(void)
{
}

string ParaWorld::NewWorld(const char* sWorldName, const char* sBaseWorldName)
{
	CParaFile templateFile;
	templateFile.OpenAssetFile(sBaseWorldName);
	if(templateFile.isEof())
		return "";
	ParaTerrain::CWorldNameFactory worldname(sWorldName);
	ParaTerrain::CWorldNameFactory baseWorldname(sBaseWorldName);
	CParaFile file;
	// copy the world config file.
	string configFileName =worldname.GetDefaultWorldConfigName();
	if(file.CreateNewFile(configFileName.c_str(), true)==false)
		return "";
	file.write(templateFile.getBuffer(), (int)templateFile.getSize());

	// copy the default terrain tile file, called %WORLD%/flat.txt in the root directory of the base world.
	string dest = worldname.GetDefaultTileConfigName();
	string src = baseWorldname.GetDefaultTileConfigName();
	if( ! CParaFile::CopyFile(src.c_str(), dest.c_str(), false))
	{
		OUTPUT_LOG("warning: unable to copy default terrain tile %s file in ParaWorld.NewWorld()\n", src.c_str());
	}
	return configFileName;
}

const char* ParaWorld::NewWorld_(const char* sWorldName, const char* sBaseWorldName)
{
	static string g_str;
	g_str = NewWorld(sWorldName, sBaseWorldName);
	return g_str.c_str();
}

void ParaWorld::DeleteWorld(const char* sWorldName)
{

}

string ParaWorld::NewEmptyWorld(const char* sWorldName, float fTileSize, int nTileDimension )
{
	string configFileName;
	char line[MAX_LINE+1];
	memset(line, 0, sizeof(line));
	ParaTerrain::CWorldNameFactory worldname((sWorldName==NULL) ? "_emptyworld":sWorldName);
	string sTerrainConfig = worldname.GetWorldDirectory()+"flat.txt"; // worldname.GetTerrainConfigFile(0,0);
	string sTerrainElev = worldname.GetWorldDirectory()+"flat.raw"; //worldname.GetTerrainElevationFile(0,0);

	{ // write config file
		CParaFile cFile;
		configFileName = worldname.GetDefaultWorldConfigName();
		if(cFile.CreateNewFile(configFileName.c_str(), true)==false)
			return "";
		cFile.WriteString("-- Auto generated by ParaEngine \n");
		cFile.WriteString("type = lattice\n");
		snprintf(line, MAX_LINE, "TileSize = %f\n", fTileSize);
		cFile.WriteString(line);

		for (int x=0;x<nTileDimension; ++x)
		{
			for (int y=0;y<nTileDimension;++y)
			{
				snprintf(line, MAX_LINE, "(%d,%d) = %s\n", x,y, sTerrainConfig.c_str());
				cFile.WriteString(line);
			}
		}
	}
	{ // write terrain config file for 0 0
	
		CParaFile cFile;
		if(cFile.CreateNewFile(sTerrainConfig.c_str(), true)==false)
			return "";
		const char sFileFmt[] = "\
-- auto gen by ParaEngine\n\
Heightmapfile = %s\n\
MainTextureFile = terrain/data/MainTexture.dds\n\
CommonTextureFile = terrain/data/CommonTexture.dds\n\
Size = %f\n\
ElevScale = 1.0\n\
Swapvertical = 1\n\
HighResRadius = 30\n\
DetailThreshold = 50.000000\n\
MaxBlockSize = 64\n\
DetailTextureMatrixSize = 64\n\
NumOfDetailTextures = 0\n\
";
		char buf[1024+1];
		int nSize = snprintf(buf, 1024, sFileFmt, sTerrainElev.c_str(), fTileSize);
		cFile.write(buf, nSize);
	}
	{ // flat elevation file

		CParaFile cFile;
		if(cFile.CreateNewFile(sTerrainElev.c_str(), true)==false)
			return "";
		int nSize = 129*129;
		float* elev = new float[nSize];
		memset(elev, 0 , nSize*4);
		cFile.write(elev, nSize*4);
		delete [] elev;
	}
	return configFileName;
}

const char* ParaWorld::NewEmptyWorld_(const char* sWorldName, float fTileSize, int nTileDimension)
{
	static string g_str;
	g_str = NewEmptyWorld(sWorldName, fTileSize, nTileDimension);
	return g_str.c_str();
}

ParaDataProvider ParaWorld::GetNpcDB()
{
#ifdef USE_DIRECTX_RENDERER
	return ParaDataProvider(CGlobals::GetDataProviderManager()->GetNpcDB());
#else
	return ParaDataProvider();
#endif
}

void ParaWorld::GetNpcDB_(ParaDataProvider* out)
{
	*out = GetNpcDB();
}
void ParaWorld::SetNpcDB(const char* sConnectionstring)
{
	CGlobals::GetDataProviderManager()->SetNpcDB(sConnectionstring);
}

ParaDataProvider ParaWorld::GetAttributeProvider()
{
	return ParaDataProvider(CGlobals::GetDataProviderManager()->GetAttributeProvider());
}

void ParaWorld::GetAttributeProvider_(ParaDataProvider * out)
{
	*out = GetAttributeProvider();
}

void ParaWorld::SetAttributeProvider(const char* sConnectionstring)
{
	CGlobals::GetDataProviderManager()->SetAttributeProvider(sConnectionstring);
}

void ParaWorld::SetWorldDB(const char* sConnectionstring)
{
	CGlobals::GetDataProviderManager()->SetWorldDB(sConnectionstring);
}

string ParaWorld::GetStringbyID(int ID)
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetDataProviderManager()->GetKidsDBProvider()->GetStringbyID(ID);
#else
	return CGlobals::GetString(0);
#endif
}

int ParaWorld::InsertString(const char* strEN, const char * strCN)
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetDataProviderManager()->GetKidsDBProvider()->InsertString(strEN, strCN);
#else
	return 0;
#endif
}

string ParaWorld::GetWorldName()
{
	return CGlobals::GetWorldInfo()->GetWorldName();
}

string ParaWorld::GetWorldDirectory()
{
	return CGlobals::GetWorldInfo()->GetWorldDirectory();
}


void ParaWorld::SetServerState(int nState)
{
	//CEmuCSParaEditGame::GetInstance().SetState((CEmuCSParaEditGame::CurrentState)nState);
}

int ParaWorld::GetServerState()
{
	//return (int)CEmuCSParaEditGame::GetInstance().GetState();
	return 0;
}

bool ParaWorld::SendTerrainUpdate( const char* sDestination, float center_x,float center_y,float center_z, float fRadius, float fResolution )
{
	//return CEmuCSParaEditGame::GetInstance().SendTerrainUpdate(sDestination, Vector3(center_x, center_y, center_z), fRadius, fResolution);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// ParaSelection
//
//////////////////////////////////////////////////////////////////////////

void ParaSelection::RemoveObject(const ParaObject& obj)
{
	if(obj.IsValid())
		CGlobals::GetSelectionManager()->RemoveObject((CBaseObject*) obj.m_pObj);
}

void ParaSelection::AddObject(const ParaObject& obj, int nGroupID)
{
	if(obj.IsValid())
		CGlobals::GetSelectionManager()->AddObject((CBaseObject*)obj.m_pObj, nGroupID);
}

ParaObject ParaSelection::GetObject(int nGroupID, int nItemIndex)
{
	ParaObject obj;
	SelectedItem item;
	if(CGlobals::GetSelectionManager()->GetObject(nGroupID, nItemIndex, &item))
	{
		obj.m_pObj = item.GetAs3DObject();
	}
	return obj;
}
void ParaSelection::GetObject_(ParaObject* obj, int nGroupID, int nItemIndex)
{
	if(obj!=0)
	{
		SelectedItem item;
		if(CGlobals::GetSelectionManager()->GetObject(nGroupID, nItemIndex, &item))
		{
			obj->m_pObj = item.GetAs3DObject();
		}
	}
}

int ParaSelection::GetItemNumInGroup(int nGroupID)
{
	return CGlobals::GetSelectionManager()->GetItemNumInGroup(nGroupID);
}

void ParaSelection::SelectGroup(int nGroupID, bool bSelect)
{
	CGlobals::GetSelectionManager()->SelectGroup(nGroupID, bSelect);
}

void ParaSelection::ClearGroup(int nGroupID)
{
	CGlobals::GetSelectionManager()->ClearGroup(nGroupID);
}

void ParaSelection::SetMaxItemNumberInGroup(int nGroupID, int nMaxItemsNumber)
{
	CGlobals::GetSelectionManager()->SetMaxItemNumberInGroup(nGroupID, nMaxItemsNumber);
}

int ParaSelection::GetMaxItemNumberInGroup(int nGroupID)
{
	return CGlobals::GetSelectionManager()->GetMaxItemNumberInGroup(nGroupID);
}

bool ParaSelection::RemoveUIObject(const char* sName)
{
	CGUIBase* pObject = CGlobals::GetGUI()->GetUIObject(sName);
	if(pObject!=NULL)
	{
		CGlobals::GetSelectionManager()->RemoveObject(pObject);
		return true;
	}
	return false;
}

bool ParaSelection::AddUIObject(const char* sName, int nGroupID)
{
	CGUIBase* pObject = CGlobals::GetGUI()->GetUIObject(sName);
	if(pObject!=NULL)
	{
		CGlobals::GetSelectionManager()->AddObject(pObject, nGroupID);
		return true;
	}
	return false;
}

void ParaScripting::ParaWorld::SetScriptSandBox( const object& sSandboxNeuronFile )
{
	int nType = type(sSandboxNeuronFile);
	const char* sValue = NULL;
	if (nType == LUA_TSTRING)
	{
		sValue = object_cast<const char*>(sSandboxNeuronFile);
	}

	CGlobals::GetWorldInfo()->SetScriptSandBox(sValue);
}

const char* ParaScripting::ParaWorld::GetScriptSandBox()
{
	return CGlobals::GetWorldInfo()->GetScriptSandBox();
}

void ParaScripting::ParaWorld::SetEnablePathEncoding( bool bEnable )
{
	CGlobals::GetGlobalTerrain()->SetEnablePathEncoding(bEnable);
}

bool ParaScripting::ParaWorld::GetEnablePathEncoding()
{
	return CGlobals::GetGlobalTerrain()->GetEnablePathEncoding();
}
