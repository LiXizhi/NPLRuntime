//-----------------------------------------------------------------------------
// Class:	ParaXStaticModelRenderPass
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.5
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXStaticModelRenderPass.h"

using namespace ParaEngine;

ParaEngine::ParaXStaticModelRenderPass::ParaXStaticModelRenderPass() 
	: indexStart(0), indexCount(0), m_nIndexStart(0), order(0), geoset(0), p(0.f), blendmode(0)
{

}

bool ParaEngine::ParaXStaticModelRenderPass::init_FX(CParaXStaticModel *m_, SceneState* pSceneState, CParameterBlock* pMaterialParams /*= NULL*/)
{
	return false;
}

void ParaEngine::ParaXStaticModelRenderPass::deinit_FX(SceneState* pSceneState)
{
	
}
