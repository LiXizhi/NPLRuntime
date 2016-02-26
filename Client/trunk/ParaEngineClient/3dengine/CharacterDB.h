#pragma once
#include "ic/ParaDatabase.h"

namespace ParaEngine
{
	/** it contains all functions to access the character database. This is a singleton class. 
	* call GetInstance() to use this class. */
	class CCharacterDB
	{
	public:		
		/** get the singleton class.*/
		static CCharacterDB& GetInstance();

		/** clean up.*/
		void CloseDB();

		/** check if database is valid or not.*/
		bool CheckLoad();

		/** TODO: recreate a blank database. */
		void ResetDataBase();
		
	public:
		/** Get race ID from name
		return true if the record is found in database. */
		bool GetRaceIDbyName(const string& racename, int& id);

		/** Get race short name from race id
		return true if the record is found in database. */
		bool GetRaceShortNamebyRaceID(int id, string& racename);

		/** get the model ID from model asset file name
		return true if the record is found in database. */
		bool GetModelIDfromModelFile(const string& sModelFile, int& modelid, int& modeltype);

		/** get the replaceable texture group for the specified index.
		return true if the model is found.but it does not mean that the skin ID is found. 
		@param bFound: whether found */
		bool GetReplaceTexturesByModelIDAndSkinID(int modelid, int skinIndex, string& sReplaceTexture0, string& sReplaceTexture1, string& sReplaceTexture2, bool& bFound);

		/** get the character's skin textures with a specified section type and section number.
		return true if the record is found in database. */
		bool GetCharacterSkins(int race, int gender, int nSectionType, int nSection, int skinColor, string& sSkinTexture0, string& sSkinTexture1, string& sSkinTexture2);

		/** get the facial hair's geosets ids.
		return true if the record is found in database. */
		bool GetFacialHairGeosets(int race, int gender, int facialHair, int& geoset1, int& geoset2,int& geoset3);

		/** get the hair style geoset.
		return true if the record is found in database. */
		bool GetHairGeosets(int race, int gender, int hairStyle, int& geoset);

		/**  get item's model type and model id by its item id. 
		return true if the record is found in database. */
		bool GetModelIDfromItemID(int itemid, int& nItemType, int&nItemModelID);

		/**  get item display information by its model item id. The display information will describe how the item (equipment) is 
		displayed on the body of a character. 
		return true if the record is found in database. */
		bool GetModelDisplayInfo(int nItemModelID, int& GeosetA, int& GeosetB, int& GeosetC, int& GeosetD, int& GeosetE, 
			int& flags, int& GeosetVisID1, int& GeosetVisID2, 
			string& skin, string& skin2, string& TexArmUpper, string& TexArmLower, string& TexHands, string& TexChestUpper, 
			string& TexChestLower, string& TexLegUpper, string& TexLegLower,string& TexFeet);

		/** get the model information from the model display database by the model id
		return true if the record is found in database. */
		bool GetModelInfoFromModelID(int nItemModelID, string& Model, string& Model2, string& Skin, string& Skin2,
			int& visualid,int& GeosetVisID1, int& GeosetVisID2);

		/** get the item set.
		return true if the record is found in database. */
		bool GetItemSetsBySetID(int setid, void* itemsets, int nSize);
		/** get the item's type by its item id
		return true if the record is found in database. */
		bool GetItemTypeByItemID(int itemid, int& itemtype);

		/**
		* 
		* @param nType: type of CartoonFaceSlots
				enum CartoonFaceSlots
				{
				CFS_FACE = 0,
				CFS_WRINKLE = 1,
				CFS_EYE = 2,
				CFS_EYEBROW = 3,
				CFS_MOUTH = 4,
				CFS_NOSE = 5,
				CFS_MARKS = 6,
				};
		* @param nStyle: starting from 0
		* @param sTex1: out texture path
		* @param sTex2: out texture path
		* @param sIconPath: out texture icon path
		* @return :true if succeed.
		*/
		bool GetCartoonFaceComponent(int nType, int nStyle, string* sTex1,string* sTex2, string* sIconPath );

	protected:
	private:
		CCharacterDB();
		~CCharacterDB();

		static void ConvertModelName(string& sName);
		/** base database interface */
		asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;
		bool m_bIsValid;
	};
}
