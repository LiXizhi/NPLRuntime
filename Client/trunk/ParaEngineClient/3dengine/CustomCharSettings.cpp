//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CustomCharCommon.h"
#include "CustomCharSettings.h"

using namespace ParaEngine;

namespace ParaEngine
{
#ifdef PARAENGINE_MOBILE
	int CCharCustomizeSysSetting::CharTexSize = 128; // the recommended size is 256, however 512 is the original size of component textures, 2007.7.7 by LiXizhi
	int CCharCustomizeSysSetting::FaceTexSize = 128;
#else
	int CCharCustomizeSysSetting::CharTexSize = 256; // the recommended size is 256, however 512 is the original size of component textures, 2007.7.7 by LiXizhi
	int CCharCustomizeSysSetting::FaceTexSize = 256;
#endif
	const float face_pixel_scale = (float)CCharCustomizeSysSetting::FaceTexSize / 256.f;
	CharRegionCoords CCharCustomizeSysSetting::regions[NUM_REGIONS] =
	{
		//
		// character skin regions: left, top, width, height
		//
		{ 0, 0, CharTexSize, CharTexSize },	// base
		{ 0, 0, CharTexSize / 2, CharTexSize / 4 },	// arm upper
		{ 0, CharTexSize / 4, CharTexSize / 2, CharTexSize / 4 },	// arm lower
		{ 0, CharTexSize / 2, CharTexSize / 2, CharTexSize / 8 },	// hand
		{ 0, CharTexSize / 8 * 5, CharTexSize / 2, CharTexSize / 8 },	// face upper
		{ 0, CharTexSize / 8 * 6, CharTexSize / 2, CharTexSize / 4 },	// face lower
		{ CharTexSize / 2, 0, CharTexSize / 2, CharTexSize / 4 },	// torso upper
		{ CharTexSize / 2, CharTexSize / 4, CharTexSize / 2, CharTexSize / 8 },	// torso lower
		{ CharTexSize / 2, CharTexSize / 8 * 3, CharTexSize / 2, CharTexSize / 4 }, // leg upper
		{ CharTexSize / 2, CharTexSize / 8 * 5, CharTexSize / 2, CharTexSize / 4 },// leg lower
		{ CharTexSize / 2, CharTexSize / 8 * 7, CharTexSize / 2, CharTexSize / 8 },	// foot
		{ 0, CharTexSize / 8 * 6, CharTexSize / 2, CharTexSize / 4 },	// wings

		//
		// character face regions: center_x, center_y, default_width, default_height
		//
		{ FaceTexSize / 2, FaceTexSize / 2, FaceTexSize, FaceTexSize },	// face base
		{ FaceTexSize / 2, FaceTexSize / 2, FaceTexSize, FaceTexSize },	// wrinkle
		{ FaceTexSize / 2 + (int)(30 * face_pixel_scale), FaceTexSize / 2, FaceTexSize / 4, FaceTexSize / 4 },	// eye right
		{ FaceTexSize / 2 + (int)(33 * face_pixel_scale), FaceTexSize / 2 - (int)(20 * face_pixel_scale), FaceTexSize / 4, FaceTexSize / 4 },	// eye bow right
		{ FaceTexSize / 2, FaceTexSize / 2 + (int)(41 * face_pixel_scale), FaceTexSize / 4, FaceTexSize / 4 },	// mouth
		{ FaceTexSize / 2, FaceTexSize / 2 + (int)(12 * face_pixel_scale), FaceTexSize / 4, FaceTexSize / 4 },	// nose
		{ FaceTexSize / 2, FaceTexSize / 2, FaceTexSize, FaceTexSize },	// mark

		//
		// aries character skin regions: left, top, width, height
		//
		{ 0, 0, CharTexSize, CharTexSize / 2 }, // CR_ARIES_CHAR_SHIRT,
		{ 0, CharTexSize / 2, CharTexSize, CharTexSize / 2 }, // CR_ARIES_CHAR_SHIRT_OVERLAYER
		{ CharTexSize / 2, CharTexSize / 2, CharTexSize / 2, CharTexSize / 2 }, // CR_ARIES_CHAR_PANT,
		{ 0, CharTexSize / 2, CharTexSize / 2, CharTexSize / 8 },	// CR_ARIES_CHAR_HAND
		{ 0, CharTexSize / 8 * 6, CharTexSize / 2, CharTexSize / 4 },	// CR_ARIES_CHAR_FOOT
		{ 0, CharTexSize / 8 * 4, CharTexSize / 2, CharTexSize / 4 }, // CR_ARIES_CHAR_GLASS

		//
		// aries pet skin regions: left, top, width, height
		//
		{ 0, 0, CharTexSize / 2, CharTexSize / 2 }, // CR_ARIES_PET_HEAD,
		{ CharTexSize / 2, 0, CharTexSize / 2, CharTexSize }, // CR_ARIES_PET_BODY,
		{ CharTexSize / 2, CharTexSize / 2, CharTexSize / 2, CharTexSize / 2 }, // CR_ARIES_PET_TAIL,
		{ 0, CharTexSize / 2, CharTexSize / 2, CharTexSize / 2 }, // CR_ARIES_PET_WING,

		{ 0, 0, CharTexSize, CharTexSize }, // CR_ARIES_CHAR_SHIRT_TEEN,
	};
	const char* CCharCustomizeSysSetting::regionPaths[NUM_REGION_PATHS] =
	{
		// body texture paths
		"",
		"character/v3/Item/TextureComponents/ArmUpperTexture/",
		"character/v3/Item/TextureComponents/ArmLowerTexture/",
		"character/v3/Item/TextureComponents/HandTexture/",
		"",
		"",
		"character/v3/Item/TextureComponents/TorsoUpperTexture/",
		"character/v3/Item/TextureComponents/TorsoLowerTexture/",
		"character/v3/Item/TextureComponents/LegUpperTexture/",
		"character/v3/Item/TextureComponents/LegLowerTexture/",
		"character/v3/Item/TextureComponents/FootTexture/",
		"character/v3/Item/TextureComponents/WingTexture/",

		// face texture paths
		"character/v3/CartoonFace/face/",	// face base
		"character/v3/CartoonFace/faceDeco/",// wrinkle
		"character/v3/CartoonFace/eye/",	// eye right
		"character/v3/CartoonFace/eyebrow/",// eye bow right
		"character/v3/CartoonFace/mouth/",	// mouth
		"character/v3/CartoonFace/nose/",	// nose
		"character/v3/CartoonFace/mark/",	// mark

		// aries character skin paths
		"character/v3/Item/TextureComponents/AriesCharShirtTexture/",	// aries char shirt
		"character/v3/Item/TextureComponents/AriesCharShirtTexture/",	// aries char shirt over layer
		"character/v3/Item/TextureComponents/AriesCharPantTexture/",	// aries char pant
		"character/v3/Item/TextureComponents/AriesCharHandTexture/",	// aries char hand
		"character/v3/Item/TextureComponents/AriesCharFootTexture/",	// aries char foot
		"character/v3/Item/TextureComponents/AriesCharGlassTexture/",	// aries char glass

		// aries pet skin paths
		"character/v3/Item/TextureComponents/AriesPetHeadTexture/",	// aries pet head
		"character/v3/Item/TextureComponents/AriesPetBodyTexture/",	// aries pet body
		"character/v3/Item/TextureComponents/AriesPetTailTexture/",	// aries pet tail
		"character/v3/Item/TextureComponents/AriesPetWingTexture/",	// aries pet wing

		// aries teen character skin paths
		"character/v6/Item/ShirtTexture/",	// aries char shirt teen

		"",
		// object paths
		"character/v3/Item/ObjectComponents/Cape/",
		"character/v3/Item/ObjectComponents/Head/",
		"character/v3/Item/ObjectComponents/Shoulder/",
		"character/v3/Item/ObjectComponents/Weapon/",
		"character/v3/Item/ObjectComponents/Shield/", // added by andy -- 2009.5.10
		"character/v3/Item/ObjectComponents/Skirt/",
		"character/v3/Item/ObjectComponents/FaceAddon/", // added by andy -- 2009.5.10

		// aries character object paths
		"character/v3/Item/TextureComponents/AriesCharWingTexture/",	// aries char wing
		"character/v3/Item/ObjectComponents/Back/",	// aries char back
		"character/v3/Item/ObjectComponents/GroundEffect/",	// aries character ground effect
		"character/v3/Item/ObjectComponents/GemBuffEffect/",	// aries gem buff effect
	};

	const char* CCharCustomizeSysSetting::DefaultCartoonFaceTexture[CFS_TOTAL_NUM] =
	{
		"character/v3/CartoonFace/face/face_00.dds",
		"", // "character/v3/CartoonFace/faceDeco/wrinkle_00.dds",
		"character/v3/CartoonFace/eye/eye_00.dds",
		"character/v3/CartoonFace/eyebrow/eyebrow_00.dds",
		"character/v3/CartoonFace/mouth/mouth_00.dds",
		"character/v3/CartoonFace/nose/nose_00.dds",
		"", // "character/v3/CartoonFace/faceDeco/marks_00.dds",
	};


	/** slot order is so defined that the base body are updated one after another in the given order */
	const int CCharCustomizeSysSetting::slotOrder[] = {
		CS_HEAD,
		CS_NECK,
		CS_SHOULDER,
		CS_BOOTS,
		CS_BELT,
		CS_SHIRT,
		CS_PANTS,
		CS_CHEST,
		CS_BRACERS,
		CS_GLOVES,
		CS_HAND_RIGHT,
		CS_HAND_LEFT,
		CS_CAPE,
		CS_TABARD,
		CS_FACE_ADDON,
		CS_WINGS,
		// NOTE 2010/4/2: put the aries shirt above all others, the over layer of shirt will mask the pant hand foot and glass texture
		CS_ARIES_CHAR_SHIRT,
		CS_ARIES_CHAR_PANT,
		CS_ARIES_CHAR_HAND,
		CS_ARIES_CHAR_FOOT,
		CS_ARIES_CHAR_GLASS,
		CS_ARIES_CHAR_WING,
		CS_ARIES_PET_HEAD,
		CS_ARIES_PET_BODY,
		CS_ARIES_PET_TAIL,
		CS_ARIES_PET_WING,
		CS_ARIES_CHAR_BACK,
		CS_ARIES_CHAR_GROUND_EFFECT,
		CS_ARIES_CHAR_SHIRT_TEEN,
		CS_ARIES_CHAR_GEM_BUFF_EFFECT,
		CS_ARIES_CHAR_RING_BUFF_EFFECT
	};

	const int CCharCustomizeSysSetting::slotOrderWithRobe[] = {
		CS_HEAD,
		CS_NECK,
		CS_SHOULDER,
		CS_PANTS,
		CS_BOOTS,
		CS_SHIRT,
		CS_CHEST,
		CS_TABARD,
		CS_BELT,
		CS_BRACERS,
		CS_GLOVES,
		CS_HAND_RIGHT,
		CS_HAND_LEFT,
		CS_CAPE,
		CS_FACE_ADDON,
		CS_WINGS,
		CS_ARIES_CHAR_SHIRT,
		CS_ARIES_CHAR_PANT,
		CS_ARIES_CHAR_HAND,
		CS_ARIES_CHAR_FOOT,
		CS_ARIES_CHAR_GLASS,
		CS_ARIES_CHAR_WING,
		CS_ARIES_PET_HEAD,
		CS_ARIES_PET_BODY,
		CS_ARIES_PET_TAIL,
		CS_ARIES_PET_WING,
		CS_ARIES_CHAR_BACK,
		CS_ARIES_CHAR_GROUND_EFFECT,
		CS_ARIES_CHAR_SHIRT_TEEN,
		CS_ARIES_CHAR_GEM_BUFF_EFFECT,
		CS_ARIES_CHAR_RING_BUFF_EFFECT
	};

	/// This is not used. Unless you want to override attachment scale, defined in model
	const float CCharCustomizeSysSetting::shoulderscales[10][2] = {
		// male, female
		{ 1.0f, 1.0f },	// 0 = dummy
		{ 1.0f, 1.0f },	// human
		{ 1.0f, 1.0f },	// orc
		{ 1.0f, 1.0f },	// dwarf
		{ 1.0f, 1.0f },	// nightelf
		{ 1.0f, 1.0f },	// scourge
		{ 1.0f, 1.0f },	// tauren
		{ 1.0f, 1.0f },	// gnome
		{ 1.0f, 1.0f },	// troll
		{ 1.0f, 1.0f }	// goblin
	};
}

const char* CCharCustomizeSysSetting::GetRegionPath(int i)
{
	return regionPaths[i];
}

int CCharCustomizeSysSetting::GetSlotOrder(int nSlotIndex, bool hadRobe)
{
	return hadRobe ? slotOrderWithRobe[nSlotIndex] : slotOrder[nSlotIndex];
}

void CCharCustomizeSysSetting::SetCharacterRegionPath(int slot, const std::string& path)
{
	static std::map <int, std::string> g_Cache;
	g_Cache[slot] = path;
	regionPaths[slot] = g_Cache[slot].c_str();
}

CCharCustomizeSysSetting& CCharCustomizeSysSetting::GetSingleton()
{
	static CCharCustomizeSysSetting g_singleton;
	return g_singleton;
}

CCharCustomizeSysSetting::~CCharCustomizeSysSetting()
{

}

CCharCustomizeSysSetting::CCharCustomizeSysSetting()
{

}

void CCharCustomizeSysSetting::SetCharTextureSize(int nCharacterTexSize /*= 256*/, int nCartoonFaceTexSize /*= 256*/)
{
	CharTexSize = nCharacterTexSize;
	FaceTexSize = nCartoonFaceTexSize;
}

void CCharCustomizeSysSetting::SetCharRegionCoordinates(int nRegionIndex, int xpos, int ypos, int xsize, int ysize)
{
	if (nRegionIndex >= 0 && nRegionIndex < NUM_REGIONS)
	{
		CharRegionCoords& cords = regions[nRegionIndex];
		cords.xpos = xpos;
		cords.ypos = ypos;
		cords.xsize = xsize;
		cords.ysize = ysize;
	}
	else
	{
		OUTPUT_LOG("warning: SetCharRegionCoordinates with invalid nRegionIndex %d\n", nRegionIndex);
	}
}
