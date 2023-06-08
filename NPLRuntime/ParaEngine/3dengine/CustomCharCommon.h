#pragma once
#include "TextureEntity.h"

namespace ParaEngine
{
	enum CharRegions {
		// character skin regions
		CR_BASE = 0,
		CR_ARM_UPPER,
		CR_ARM_LOWER,
		CR_HAND,
		CR_FACE_UPPER,
		CR_FACE_LOWER,
		CR_TORSO_UPPER,
		CR_TORSO_LOWER,
		CR_LEG_UPPER,
		CR_LEG_LOWER,
		CR_FOOT,
		CR_WINGS, // newly added by andy -- 2009.5.11, we use the face lower texture region for wing texture

		// character face regions
		CR_FACE_BASE,
		CR_FACE_WRINKLE,
		CR_FACE_EYE_RIGHT,
		//CR_FACE_EYE_LEFT,
		CR_FACE_EYEBROW_RIGHT,
		//CR_FACE_EYEBROW_LEFT,
		CR_FACE_MOUTH,
		CR_FACE_NOSE,
		CR_FACE_MARKS,

		// aries character skin regions
		CR_ARIES_CHAR_SHIRT,
		CR_ARIES_CHAR_SHIRT_OVERLAYER,
		CR_ARIES_CHAR_PANT,
		CR_ARIES_CHAR_HAND,
		CR_ARIES_CHAR_FOOT,
		CR_ARIES_CHAR_GLASS,

		// aries pet skin regions
		CR_ARIES_PET_HEAD,
		CR_ARIES_PET_BODY,
		CR_ARIES_PET_TAIL,
		CR_ARIES_PET_WING,

		CR_ARIES_CHAR_SHIRT_TEEN,

		NUM_REGIONS,

		// character object regions
		CR_CAPE,
		CR_HEAD,
		CR_SHOULDER,
		CR_WEAPON,
		CR_SHIELD,
		CR_SKIRT,
		CR_FACE_ADDON,

		// aries character object regions
		CR_ARIES_CHAR_WING,
		CR_ARIES_CHAR_BACK,
		CR_ARIES_CHAR_GROUND_EFFECT,
		CR_ARIES_CHAR_GEM_BUFF_EFFECT,

		NUM_REGION_PATHS,
	};

	struct CharRegionCoords {
		int xpos, ypos, xsize, ysize;
	};

	enum ItemTypes {
		IT_ALL = 0,
		IT_HEAD = 1,
		IT_NECK,
		IT_SHOULDER,
		IT_SHIRT,
		IT_CHEST,
		IT_BELT,
		IT_PANTS,
		IT_BOOTS,
		IT_BRACERS,
		IT_GLOVES,
		IT_RINGS,
		IT_OFFHAND,
		IT_DAGGER,
		IT_SHIELD,
		IT_BOW,
		IT_CAPE,
		IT_2HANDED,
		IT_QUIVER,
		IT_TABARD,
		IT_ROBE,
		IT_1HANDED,
		IT_CLAW,
		IT_ACCESSORY,
		IT_THROWN,
		IT_GUN,
		IT_MASK, // CS_FACE_ADDON
		IT_WINGS, // CS_WINGS

		IT_ARIES_CHAR_SHIRT,
		IT_ARIES_CHAR_PANT,
		IT_ARIES_CHAR_HAND,
		IT_ARIES_CHAR_FOOT,
		IT_ARIES_CHAR_GLASS,
		IT_ARIES_CHAR_WING,
		IT_ARIES_PET_HEAD,
		IT_ARIES_PET_BODY,
		IT_ARIES_PET_TAIL,
		IT_ARIES_PET_WING,
		IT_ARIES_CHAR_BACK,
		IT_ARIES_CHAR_GROUND_EFFECT,

		IT_ARIES_CHAR_SHIRT_TEEN,

		IT_ARIES_CHAR_GEM_BUFF_EFFECT,

		NUM_ITEM_TYPES
	};

	/** attachment ID for character models */
	enum ATTACHMENT_ID
	{
		ATT_ID_SHIELD = 0,
		ATT_ID_HAND_RIGHT = 1,
		ATT_ID_HAND_LEFT = 2,
		ATT_ID_TEXT = 3,
		ATT_ID_GROUND = 4,
		ATT_ID_SHOULDER_RIGHT = 5,
		ATT_ID_SHOULDER_LEFT = 6,
		ATT_ID_HEAD = 11,
		ATT_ID_FACE_ADDON = 12,
		ATT_ID_EAR_LEFT_ADDON = 13,
		ATT_ID_EAR_RIGHT_ADDON = 14,
		ATT_ID_BACK_ADDON = 15,
		ATT_ID_WAIST = 16,
		ATT_ID_NECK = 17,
		ATT_ID_BOOTS = 18,
		ATT_ID_MOUTH = 19,
		ATT_ID_MOUNT1 = 20,
		ATT_ID_MOUNT2,
		ATT_ID_MOUNT3,
		ATT_ID_MOUNT4,
		ATT_ID_MOUNT5,
		ATT_ID_MOUNT6,
		ATT_ID_MOUNT7,
		ATT_ID_MOUNT8,
		ATT_ID_MOUNT9,
		ATT_ID_MOUNT10,
		ATT_ID_MOUNT11,
		ATT_ID_MOUNT12,
		ATT_ID_MOUNT13,
		ATT_ID_MOUNT14,
		ATT_ID_MOUNT15,
		ATT_ID_MOUNT16,
		ATT_ID_MOUNT17,
		ATT_ID_MOUNT18,
		ATT_ID_MOUNT19,
		ATT_ID_MOUNT20,
		ATT_ID_MOUNT00 = 0xffff, // this is same as ATT_ID_SHIELD or MOUNT0
	};

	/**
	* slots ID on the character
	*/
	enum CharSlots {
		CS_HEAD = 0,
		CS_NECK = 1,
		CS_SHOULDER = 2,
		CS_BOOTS = 3,
		CS_BELT = 4,
		CS_SHIRT = 5,
		CS_PANTS = 6,
		CS_CHEST = 7,
		CS_BRACERS = 8,
		CS_GLOVES = 9,
		CS_HAND_RIGHT = 10,
		CS_HAND_LEFT = 11,
		CS_CAPE = 12,
		CS_TABARD = 13,
		CS_FACE_ADDON = 14,
		CS_WINGS = 15,
		CS_ARIES_CHAR_SHIRT = 16,
		CS_ARIES_CHAR_PANT = 17,
		CS_ARIES_CHAR_HAND = 18,
		CS_ARIES_CHAR_FOOT = 19,
		CS_ARIES_CHAR_GLASS = 20,
		CS_ARIES_CHAR_WING = 21,
		CS_ARIES_PET_HEAD = 22,
		CS_ARIES_PET_BODY = 23,
		CS_ARIES_PET_TAIL = 24,
		CS_ARIES_PET_WING = 25,
		CS_ARIES_CHAR_BACK = 26,
		CS_ARIES_CHAR_GROUND_EFFECT = 27,
		CS_ARIES_CHAR_SHIRT_TEEN = 28,
		CS_ARIES_CHAR_GEM_BUFF_EFFECT = 29,
		CS_ARIES_CHAR_RING_BUFF_EFFECT = 30,

		NUM_CHAR_SLOTS
	};

	/** cartoon face slots on the face of the character*/
	enum CartoonFaceSlots
	{
		CFS_FACE = 0,
		CFS_WRINKLE = 1,
		CFS_EYE = 2,
		CFS_EYEBROW = 3,
		CFS_MOUTH = 4,
		CFS_NOSE = 5,
		CFS_MARKS = 6,

		CFS_TOTAL_NUM,
	};

	/** body parameter type on the character*/
	enum BodyParamsType
	{
		BP_SKINCOLOR = 0,
		BP_FACETYPE = 1,
		BP_HAIRCOLOR = 2,
		BP_HAIRSTYLE = 3,
		BP_FACIALHAIR = 4
	};


	/** display option type on the character*/
	enum DisplayOptionType
	{
		DO_SHOWUNDERWEAR = 0,
		DO_SHOWEARS = 1,
		DO_SHOWHAIR = 2
	};

	struct CharTextureComponent
	{
		std::string name;
		ref_ptr<TextureEntity> m_texture;
		int region;
		int layer;
		DWORD m_dwColor;

		CharTextureComponent();

		const bool operator<(const CharTextureComponent& c) const
		{
			return layer < c.layer;
		}

		/** get color. */
		DWORD GetColor() const;

		/** set color */
		void SetColor(DWORD color);

		/** copy the texture to memory in RAW RGBA(32bits) format.
		* @param pBuffer: unsigned char[width*height*4]
		* @param nWidth: [out] width of the texture in pixels
		* @param nHeight: [out] height of the texture in pixels
		* @return : return true if succeed, false otherwise
		*/
		bool GetPixels(unsigned char* pBuffer, int* nWidth, int* nHeight);

		/** whether the texture component is loaded*/
		bool CheckLoad();
	};

	struct CharTexture
	{
	public:
		CharTexture();
		~CharTexture();

		std::vector<CharTextureComponent> components;
		void addLayer(const char* fn, int region, int layer, DWORD dwColor = 0xffffffff)
		{
			if (fn == 0 || strlen(fn) == 0) return;
			CharTextureComponent ct;
			ct.name = fn;
			ct.region = region;
			ct.layer = layer;
			ct.m_dwColor = dwColor;
			components.push_back(ct);
			m_bNeedUpdateCache = true;
		}
		/** the cached file name is created from the concatenated string of component.names. */
		const string& GetCacheFileName();

		/** reset all */
		void Reset();

		/** build the synthesized texture, and save the result into the tex texture entity
		* @param tex: pointer to the texture */
		void compose(asset_ptr<TextureEntity>& tex, CharModelInstance* pCharInst);

	private:
		string m_sCachedFileName;
		bool m_bNeedUpdateCache;
		bool m_bNeedUpdate;
	};
}

#include "CustomCharSettings.h"