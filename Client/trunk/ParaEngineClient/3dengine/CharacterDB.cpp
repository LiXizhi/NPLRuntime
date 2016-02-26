//-----------------------------------------------------------------------------
// Class:	CCharacterDB
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"

#include "CharacterDB.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;


CCharacterDB::CCharacterDB()
	: m_bIsValid(true)
{
	// 2009.5.8 LXZ removed: since we only perform read only access, let us begin it to preserve cache.
	// m_pDataBase->ExecuteSqlScript("BEGIN");
}

bool ParaEngine::CCharacterDB::CheckLoad()
{
	if (!m_pDataBase && m_bIsValid)
	{
		m_pDataBase = CDBManager::OpenDBEntity("Database/characters.db");
		m_bIsValid = m_pDataBase->IsValid();
	}
	return m_bIsValid;
}

CCharacterDB::~CCharacterDB()
{
}

void CCharacterDB::CloseDB()
{
	m_pDataBase.reset();
}

/** convert .mdx name to .m2*/
void CCharacterDB::ConvertModelName(string& sName)
{
	int nLen= (int)sName.size();
	if (nLen>=4 && sName[nLen-4]=='.' && sName[nLen-1] != '2') {
		sName[nLen-3] = 'm';
		sName[nLen-2] = '2';
		sName.resize(nLen-1);
	}
}

CCharacterDB& CCharacterDB::GetInstance()
{
	static CCharacterDB g_singletonCharDB;
	return g_singletonCharDB;
}

bool CCharacterDB::GetRaceIDbyName(const string& racename, int& id)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CharRacesDB.RaceID\
													 FROM CharRacesDB\
													 WHERE CharRacesDB.Name='%s'", racename.c_str());

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		id = (*rs)[0];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetModelIDfromModelFile(const string& sModelFile, int& modelid, int& modeltype)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CreatureModelDB.ModelID, CreatureModelDB.Type\
													 FROM CreatureModelDB\
													 WHERE CreatureModelDB.Filename='%s'",
													 sModelFile.c_str());

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		modelid = (*rs)[0];
		modeltype = (*rs)[1];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetReplaceTexturesByModelIDAndSkinID(int modelid, int nSkinIndex, string& sReplaceTexture0, string& sReplaceTexture1, string& sReplaceTexture2, bool& bFound)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT DISTINCT CreatureSkinDB.Skin, CreatureSkinDB.Skin2, CreatureSkinDB.Skin3\
													 FROM CreatureSkinDB\
													 WHERE CreatureSkinDB.ModelID=%d\
													 ORDER BY CreatureSkinDB.SkinID",
													 modelid);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{

		int i=0;

		sReplaceTexture0 = (const char*)(*rs)[0];
		sReplaceTexture1 = (const char*)(*rs)[1];
		sReplaceTexture2 = (const char*)(*rs)[2];
		i++;

		if(nSkinIndex >= 1)
		{
			bFound = true;
			for (; i<nSkinIndex;i++)
			{
				if(rs->NextRow()!=0)
				{
					bFound = false;
					break;
				}
			}
			if(bFound)
			{
				if( rs->NextRow()== 0)
				{
					sReplaceTexture0 = (const char*)(*rs)[0];
					sReplaceTexture1 = (const char*)(*rs)[1];
					sReplaceTexture2 = (const char*)(*rs)[2];
					bFound = true;
				}
				else
					bFound = false;
			}
		}
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetCharacterSkins(int race, int gender, int nSectionType, int nSection, int skinColor, string& sSkinTexture0, string& sSkinTexture1, string& sSkinTexture2)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CharSectionsDB.Tex1, CharSectionsDB.Tex2, CharSectionsDB.Tex3\
													 FROM CharSectionsDB\
													 WHERE CharSectionsDB.Race=%d And CharSectionsDB.Gender=%d And CharSectionsDB.Type=\
													 %d And CharSectionsDB.Section=%d And CharSectionsDB.Color=%d",
													 race, gender, nSectionType, nSection, skinColor);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		sSkinTexture0 = (const char*)(*rs)[0];
		sSkinTexture1 = (const char*)(*rs)[1];
		sSkinTexture2 = (const char*)(*rs)[2];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetFacialHairGeosets(int race, int gender, int facialHair, int& geoset1, int& geoset2,int& geoset3)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CharFacialHairDB.Geoset100, CharFacialHairDB.Geoset200,\
													 CharFacialHairDB.Geoset300\
													 FROM CharFacialHairDB\
													 WHERE CharFacialHairDB.Race=%d And CharFacialHairDB.Gender=%d And\
													 CharFacialHairDB.Style=%d",
													 race, gender, facialHair);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		const char* t0 = (const char*)(*rs)[0];
		const char* t1 = (const char*)(*rs)[1];
		const char* t2 = (const char*)(*rs)[2];
		try	{
			geoset1 = atoi(t0);
			geoset2 = atoi(t1);
			geoset3 = atoi(t2);
		}
		catch(...){
			geoset1 = 0;
			geoset2 = 0;
			geoset3 = 0;
		}
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetHairGeosets(int race, int gender, int hairStyle, int& geosetID)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CharHairGeosetsDB.Geoset\
													 FROM CharHairGeosetsDB\
													 WHERE CharHairGeosetsDB.Race=%d And CharHairGeosetsDB.Gender=%d And\
													 CharHairGeosetsDB.Section=%d",
													 race, gender, hairStyle);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		const char * t0 = (const char*)(*rs)[0];
		try	{
			geosetID = atoi(t0);
		}
		catch(...){geosetID = 2;}
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetModelIDfromItemID(int itemid, int& nItemType, int&nItemModelID)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT ItemDatabase.type, ItemDatabase.model\
													 FROM ItemDatabase\
													 WHERE ItemDatabase.id=%d",
													 itemid);
	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		const char* t0 = (const char*)(*rs)[0];
		const char* t1 = (const char*)(*rs)[1];
		try	{
			nItemType = atoi(t0);
			nItemModelID = atoi(t1);
		}
		catch(...){
			nItemType = -1;
			nItemModelID = -1;
		}
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetModelDisplayInfo(int nItemModelID, int& GeosetA, int& GeosetB, int& GeosetC, int& GeosetD, int& GeosetE, 
									   int& flags, int& GeosetVisID1, int& GeosetVisID2, 
									   string& skin, string& skin2, string& TexArmUpper, string& TexArmLower, string& TexHands, string& TexChestUpper, 
									   string& TexChestLower, string& TexLegUpper, string& TexLegLower,string& TexFeet)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT ItemDisplayDB.Skin, ItemDisplayDB.Skin2, \
													 ItemDisplayDB.GeosetA, ItemDisplayDB.GeosetB, ItemDisplayDB.GeosetC, \
													 ItemDisplayDB.GeosetD, ItemDisplayDB.GeosetE, ItemDisplayDB.flags, \
													 ItemDisplayDB.GeosetVisID1, ItemDisplayDB.GeosetVisID2, \
													 ItemDisplayDB.TexArmUpper, ItemDisplayDB.TexArmLower,\
													 ItemDisplayDB.TexHands, ItemDisplayDB.TexChestUpper,\
													 ItemDisplayDB.TexChestLower, ItemDisplayDB.TexLegUpper,\
													 ItemDisplayDB.TexLegLower, ItemDisplayDB.TexFeet\
													 FROM ItemDisplayDB\
													 WHERE ItemDisplayDB.ItemDisplayID=%d",
													 nItemModelID);
	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		skin = (const char*)(*rs)[0];
		skin2 = (const char*)(*rs)[1];
		const char* t0 = (const char*)(*rs)[2];
		const char* t1 = (const char*)(*rs)[3];
		const char* t2 = (const char*)(*rs)[4];
		const char* t3 = (const char*)(*rs)[5];
		const char* t4 = (const char*)(*rs)[6];
		const char* t5 = (const char*)(*rs)[7];
		const char* t6 = (const char*)(*rs)[8];
		const char* t7 = (const char*)(*rs)[9];
		try	{
			GeosetA = atoi(t0);
			GeosetB = atoi(t1);
			GeosetC = atoi(t2);
			GeosetD = atoi(t3);
			GeosetE = atoi(t4);
			flags = atoi(t5);
			GeosetVisID1 = atoi(t6);
			GeosetVisID2 = atoi(t7);
		}
		catch(...){
			GeosetA = 0;
			GeosetB = 0;
			GeosetC = 0;
			GeosetD = 0;
			GeosetE = 0;
			flags = 0;
			GeosetVisID1 = 0;
			GeosetVisID2 = 0;
		}
		TexArmUpper = (const char*)(*rs)[10];
		TexArmLower = (const char*)(*rs)[11];
		TexHands = (const char*)(*rs)[12];
		TexChestUpper = (const char*)(*rs)[13];
		TexChestLower = (const char*)(*rs)[14];
		TexLegUpper = (const char*)(*rs)[15];
		TexLegLower = (const char*)(*rs)[16];
		TexFeet = (const char*)(*rs)[17];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetModelInfoFromModelID(int nItemModelID, string& Model, string& Model2, string& Skin, string& Skin2,
										   int& visualid,int& GeosetVisID1, int& GeosetVisID2)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT ItemDisplayDB.Model, ItemDisplayDB.Model2, ItemDisplayDB.Skin,\
													 ItemDisplayDB.Skin2, ItemDisplayDB.Visuals, ItemDisplayDB.GeosetVisID1, ItemDisplayDB.GeosetVisID2 \
													 FROM ItemDisplayDB\
													 WHERE ItemDisplayDB.ItemDisplayID=%d",
													 nItemModelID);
	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		const char* t0 = (const char*)(*rs)[4];
		const char* t1 = (const char*)(*rs)[5];
		const char* t2 = (const char*)(*rs)[6];
		try	{
			visualid = atoi(t0);
			GeosetVisID1= atoi(t1);
			GeosetVisID2= atoi(t2); // TODO: this is used to be t1? am I miss spelling it?
		}
		catch(...){
			visualid = -1;
			GeosetVisID1=-1;
			GeosetVisID2=-1;
		}
		Model = (const char*)(*rs)[0];
		//ConvertModelName(Model);
		Model2 = (const char*)(*rs)[1];
		//ConvertModelName(Model2);
		Skin = (const char*)(*rs)[2];
		Skin2 = (const char*)(*rs)[3];
	}
	rs->Release();
	return bHasRecord;

}

bool CCharacterDB::GetRaceShortNamebyRaceID(int race, string& racename)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CharRacesDB.ShortName\
													 FROM CharRacesDB\
													 WHERE CharRacesDB.RaceID=%d",
													 race);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		racename = (const char*)(*rs)[0];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetItemSetsBySetID(int setid, void* itemsets, int nSize)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT ItemSetDB.ItemIDBase\
													 FROM ItemSetDB\
													 WHERE ItemSetDB.SetID=%d",
													 setid);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		const void * binarydata=(const void *)(*rs)[0];
		memcpy(itemsets, binarydata, nSize);
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetItemTypeByItemID(int itemid, int& itemtype)
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT ItemDatabase.type\
													 FROM ItemDatabase\
													 WHERE ItemDatabase.id=%d",
													 itemid);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		itemtype = (*rs)[0];
	}
	rs->Release();
	return bHasRecord;
}

bool CCharacterDB::GetCartoonFaceComponent( int nType, int nStyle, string* sTex1,string* sTex2, string* sIconPath )
{
	if (!CheckLoad())
		return false;
	CICRecordSet* rs =  m_pDataBase->CreateRecordSet("SELECT CartoonFaceDB.Tex1, CartoonFaceDB.Tex2, CartoonFaceDB.Icon  \
													 FROM CartoonFaceDB\
													 WHERE CartoonFaceDB.Type=%d AND CartoonFaceDB.Style=%d",
													 nType, nStyle);

	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		if(sTex1)
			*sTex1 = (const char*)(*rs)[0];
		if(sTex2)
			*sTex2 = (const char*)(*rs)[1];
		if(sIconPath)
			*sIconPath = (const char*)(*rs)[2];
	}
	rs->Release();
	return bHasRecord;
}