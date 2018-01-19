#pragma once
#include "CustomCharCommon.h"

namespace ParaEngine
{
	/** Character customization settings. like texture size and region path. */
	class CCharCustomizeSysSetting
	{
	public:
		CCharCustomizeSysSetting();
		~CCharCustomizeSysSetting();
		static CCharCustomizeSysSetting& GetSingleton();

		/**
		* NOTE by andy: 2011.5.23
		* set character customization system region path setting
		* allow different projects share the same CCS implementation and separate region paths for asset management
		* @param slot: region slot id
		* @param path: new region path
		* NOTE: path string recorded in a static map <slot, path>
		*/
		void SetCharacterRegionPath(int slot, const std::string& path);

		/** the default character skin and cartoon face texture size.
		* @param nCharacterTexSize: the character texture size. default to 256. 512 is also fine.
		* @param nCartoonFaceTexSize: the character texture size. default to 256. 128 is also fine.
		* @note: One must call SetCharRegionCoordinates afterwards to change the region accordingly.
		*/
		void SetCharTextureSize(int nCharacterTexSize = 256, int nCartoonFaceTexSize = 256);

		/** set char region coordinates. This function together with SetCharTextureSize makes CCS regions fully customizable via scripting interface.
		* however, the region index are predefined and can not be changed.
		* @param nRegionIndex: see enumeration CharRegions. it must be smaller than NUM_REGIONS.
		* @param xpos, ypos, xsize, ysize: see struct CharRegionCoords. defines a rect region within the skin or face texture.
		*/
		void SetCharRegionCoordinates(int nRegionIndex, int xpos, int ypos, int xsize, int ysize);
	public:
		/** get region path
		* @param i: CharRegions
		*/
		static const char* GetRegionPath(int i);
		static int GetSlotOrder(int nSlotIndex, bool hadRobe);
		
		static int CharTexSize;
		static int FaceTexSize;
		static CharRegionCoords regions[];
		static const char* DefaultCartoonFaceTexture[];
		static const char* regionPaths[];
		static const int slotOrder[];
		static const int slotOrderWithRobe[];
		static const float shoulderscales[10][2];
	};
}