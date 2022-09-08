#pragma once
#include <map>
#include <set>

#define USE_CUT_CFG 1 //是否使用预设的裁剪参数（避免实时运算过多）

namespace ParaEngine 
{
	/** */
	struct FastCutInfo 
	{
		/*
		local modelName2Id = {
			stairs = 1,
			slab = 2,
			slope = 3,
		}
		*/
		// modelName*10000000 + blockIdx* 100000 + neighborDir*1000 + modelName*100 + blockIdx * 1
		int32 name;
		uint8 faces[10];
	};

	class BlockTessellateFastCutCfg 
	{
	public:
		BlockTessellateFastCutCfg() {
			init();
		};
		static bool isInited;
		static std::map<int32, const FastCutInfo*> _fastCutMap;
		static void init();
		static int32 GetModelIDFromModelName(const std::string & name);

		/** return NULL if not found. */
		static const FastCutInfo* GetCutInfo(int32 encodedKey);
	};
}
