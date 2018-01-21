#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "glad/glad.h"
#include "GLWarpper/GLTexture2D.h"
namespace ParaEngine
{
	class RenderDeviceOpenGL : public IRenderDevice
	{

	public:
		virtual uint32_t GetRenderState(const ERenderState& State) override;


		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) override;


		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) override;


		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) override;


		virtual int GetMaxSimultaneousTextures() override;

	};
}