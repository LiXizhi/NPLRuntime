#pragma once
#include <map>
#include <set>

#define USE_CUT_CFG 1 //是否使用预设的裁剪参数（避免实时运算过多）

namespace ParaEngine {
	struct FastCutInfo {
		//modelName_blockIdx_neighborDir__modelName_blockIdx
		std::string name;
		int faces[10];
	};

	class BlockTessellateFastCutCfg {
	public:
		static bool isInited;
		static std::map<std::string, std::set<int>> _fastCutMap;
		static void init();
	};
}
