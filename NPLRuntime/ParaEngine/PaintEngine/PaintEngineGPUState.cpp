//-----------------------------------------------------------------------------
// Class: Painter
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.27
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "PaintEngineGPUState.h"

using namespace ParaEngine;

ParaEngine::CPaintEngineGPUState::CPaintEngineGPUState(CPaintEngineGPUState &other)
	: CPainterState(other)
{
	isNew = true;
	needsClipBufferClear = other.needsClipBufferClear;
	clipTestEnabled = other.clipTestEnabled;
	currentClip = other.currentClip;
	canRestoreClip = other.canRestoreClip;
	rectangleClip = other.rectangleClip;
	m_bSpriteUseWorldMatrix = other.m_bSpriteUseWorldMatrix;
	matrixChanged = other.matrixChanged;
	compositionModeChanged = other.compositionModeChanged;
	clipChanged = other.clipChanged;
	m_nStencilValue = other.m_nStencilValue;
}

ParaEngine::CPaintEngineGPUState::CPaintEngineGPUState()
	:m_nStencilValue(0)
{
	isNew = true;
	needsClipBufferClear = true;
	clipTestEnabled = false;
	canRestoreClip = true;
	m_bSpriteUseWorldMatrix = false;
	matrixChanged = false;
	compositionModeChanged = false;
	clipChanged = false;
}

ParaEngine::CPaintEngineGPUState::~CPaintEngineGPUState()
{

}

void ParaEngine::CPaintEngineGPUState::SetSpriteTransform(const Matrix4 * pMatrix /*= NULL*/)
{
	matrixChanged = true;
	matComplete = (pMatrix != NULL) ? *pMatrix : Matrix4::IDENTITY;
}

void ParaEngine::CPaintEngineGPUState::SetSpriteUseWorldMatrix(bool bEnable)
{
	m_bSpriteUseWorldMatrix = bEnable;
}

bool ParaEngine::CPaintEngineGPUState::IsSpriteUseWorldMatrix()
{
	return m_bSpriteUseWorldMatrix;
}

