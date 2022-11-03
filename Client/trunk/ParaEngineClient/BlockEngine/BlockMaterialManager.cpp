//-----------------------------------------------------------------------------
// Class:	config
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.2.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BlockMaterialManager.h"

using namespace ParaEngine;

CBlockMaterialManager::CBlockMaterialManager()
{
	// TODO: WXA: we need to manage unique int16 material id per world.  
	// ID == 0 is reserved for air (empty) material, just skip rendering the face. 
}

CBlockMaterialManager::~CBlockMaterialManager()
{
}
