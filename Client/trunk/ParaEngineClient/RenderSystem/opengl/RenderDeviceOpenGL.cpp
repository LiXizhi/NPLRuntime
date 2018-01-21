#include <stdexcept>
#include "RenderDeviceOpenGL.h"
using namespace ParaEngine;

uint32_t ParaEngine::RenderDeviceOpenGL::GetRenderState(const ERenderState& State)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::RenderDeviceOpenGL::SetRenderState(const ERenderState State, const uint32_t Value)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::RenderDeviceOpenGL::SetClipPlane(uint32_t Index, const float* pPlane)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::RenderDeviceOpenGL::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat /*= 0*/, uint32_t nDataType /*= 0*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int ParaEngine::RenderDeviceOpenGL::GetMaxSimultaneousTextures()
{
	throw std::logic_error("The method or operation is not implemented.");
}
