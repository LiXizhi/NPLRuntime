#include "RenderDeviceNull.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include <string.h>
#include <vector>

using namespace ParaEngine;

namespace
{
	class NullTexture final : public IRHITexture
	{
	public:
		NullTexture(uint32_t w, uint32_t h, PixelFormat fmt, uint32_t mips, bool isCube)
			: m_width(w), m_height(h), m_format(fmt), m_mips(mips > 0 ? mips : 1), m_isCube(isCube)
		{
		}
		uint32_t GetWidth() const override { return m_width; }
		uint32_t GetHeight() const override { return m_height; }
		uint32_t GetMipLevels() const override { return m_mips; }
		PixelFormat GetFormat() const override { return m_format; }
		bool IsCubeMap() const override { return m_isCube; }
		bool UpdateSubresource(uint32_t, uint32_t, int, int, uint32_t, uint32_t, const void*, uint32_t) override { return true; }
	private:
		uint32_t m_width;
		uint32_t m_height;
		PixelFormat m_format;
		uint32_t m_mips;
		bool m_isCube;
	};

	class NullBuffer final : public IRHIBuffer
	{
	public:
		NullBuffer(uint32_t byteSize, bool isIndex, const void* initialData)
			: m_data(byteSize, 0), m_isIndex(isIndex)
		{
			if (initialData && byteSize > 0)
				memcpy(m_data.data(), initialData, byteSize);
		}
		uint32_t GetSize() const override { return static_cast<uint32_t>(m_data.size()); }
		bool IsIndexBuffer() const override { return m_isIndex; }
		void* Map(uint32_t offset, uint32_t size, ERHIMapMode) override
		{
			if (offset >= m_data.size())
				return nullptr;
			return m_data.data() + offset;
		}
		void Unmap() override {}
		bool UpdateData(uint32_t offset, uint32_t size, const void* data) override
		{
			if (!data || offset + size > m_data.size())
				return false;
			memcpy(m_data.data() + offset, data, size);
			return true;
		}
	private:
		std::vector<uint8_t> m_data;
		bool m_isIndex;
	};

	class NullRenderTarget final : public IRHIRenderTarget
	{
	public:
		NullRenderTarget(uint32_t w, uint32_t h, PixelFormat colorFmt, PixelFormat depthFmt, uint32_t colorCount)
			: m_width(w), m_height(h), m_colorFormat(colorFmt), m_depthTex(nullptr)
		{
			uint32_t cnt = (colorCount > 0) ? colorCount : 1;
			for (uint32_t i = 0; i < cnt; ++i)
				m_colorTextures.push_back(new NullTexture(w, h, colorFmt, 1, false));
			if (depthFmt != PixelFormat::Unknown)
				m_depthTex = new NullTexture(w, h, depthFmt, 1, false);
		}
		~NullRenderTarget() override
		{
			for (auto* t : m_colorTextures)
				t->ReleaseRHIResource();
			if (m_depthTex)
				m_depthTex->ReleaseRHIResource();
		}
		uint32_t GetWidth() const override { return m_width; }
		uint32_t GetHeight() const override { return m_height; }
		PixelFormat GetFormat() const override { return m_colorFormat; }
		IRHITexture* GetColorTexture(uint32_t index) const override
		{
			return (index < m_colorTextures.size()) ? m_colorTextures[index] : nullptr;
		}
		IRHITexture* GetDepthTexture() const override { return m_depthTex; }
	private:
		uint32_t m_width;
		uint32_t m_height;
		PixelFormat m_colorFormat;
		std::vector<NullTexture*> m_colorTextures;
		NullTexture* m_depthTex;
	};

	class NullOcclusionQuery final : public IRHIOcclusionQuery
	{
	public:
		bool Begin() override { return true; }
		void End() override {}
		bool GetResult(uint64_t& outSamplesPassed, bool) override
		{
			outSamplesPassed = 1;
			return true;
		}
	};

	class NullShaderProgram final : public IRHIShaderProgram
	{
	public:
		bool IsValid() const override { return true; }
	};
}

ParaEngine::RenderDeviceNull::RenderDeviceNull()
{
	m_caps.backend = ERHIBackend::Null;
	m_caps.backendName = "Null";
	m_caps.adapterName = "Headless Null Adapter";
	m_caps.featureLevel = 0;
	m_caps.maxSimultaneousRTs = 4;
	m_caps.maxTextureStages = 8;
	m_caps.maxTextureSize = 8192;
	m_caps.maxAnisotropy = 16;
	m_caps.supportsFloatRenderTargets = true;
	m_caps.supportsDepthTextures = true;
	m_caps.supportsInstancing = true;
	m_caps.supportsOcclusionQuery = true;
	m_caps.supportsTwoSidedStencil = true;
	m_caps.supportsUserClipPlanes = true;
	m_caps.supportsBCCompression = true;
	m_caps.supportsRuntimeShaderCompile = true;
	m_caps.topLeftScreenOrigin = true;
	m_caps.clipSpaceZeroToOne = true;
}

IRHITexture* ParaEngine::RenderDeviceNull::CreateTexture(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels, ERHITextureUsage usage, const RHIMipData*, uint32_t)
{
	if (width == 0 || height == 0)
		return nullptr;
	return new NullTexture(width, height, format, mipLevels, HasFlag(usage, ERHITextureUsage::CubeMap));
}

IRHIBuffer* ParaEngine::RenderDeviceNull::CreateBuffer(uint32_t byteSize, bool isIndexBuffer, ERHIBufferUsage, const void* initialData)
{
	if (byteSize == 0)
		return nullptr;
	return new NullBuffer(byteSize, isIndexBuffer, initialData);
}

IRHIRenderTarget* ParaEngine::RenderDeviceNull::CreateRenderTarget(uint32_t width, uint32_t height, PixelFormat colorFormat, PixelFormat depthFormat, uint32_t colorTargetCount, uint32_t)
{
	if (width == 0 || height == 0)
		return nullptr;
	return new NullRenderTarget(width, height, colorFormat, depthFormat, colorTargetCount);
}

IRHIOcclusionQuery* ParaEngine::RenderDeviceNull::CreateOcclusionQuery()
{
	return new NullOcclusionQuery();
}

bool ParaEngine::RenderDeviceNull::SetRenderTargets(uint32_t count, IRHIRenderTarget* const* targets, IRHIRenderTarget* depthStencilOverride)
{
	NotifyRenderTargetsBound(count, targets, depthStencilOverride);
	return true;
}

bool ParaEngine::RenderDeviceNull::BlitRenderTarget(IRHIRenderTarget*, IRHIRenderTarget*, const RECT*, const RECT*, bool)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetStreamSourceFreq(uint32_t, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawIndexedInstanced(EPrimitiveType, int, uint32_t, uint32_t, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::ReadDepthPixel(int, int, float& outDepth)
{
	outDepth = 1.0f;
	return true;
}

IRHIShaderProgram* ParaEngine::RenderDeviceNull::CompileShaderProgram(const RHIShaderProgramDesc&)
{
	return new NullShaderProgram();
}

bool ParaEngine::RenderDeviceNull::BindShaderProgram(IRHIShaderProgram*)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetShaderConstants(const void*, uint32_t, const void*, uint32_t)
{
	return true;
}

uint32_t ParaEngine::RenderDeviceNull::GetRenderState(const ERenderState& State)
{
	return m_stateTracker.GetRenderState(State);
}

bool ParaEngine::RenderDeviceNull::SetRenderState(const ERenderState State, const uint32_t Value)
{
	m_stateTracker.SetRenderState(State, Value);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClipPlane(uint32_t Index, const float* pPlane)
{
	m_stateTracker.SetClipPlane(Index, pPlane);
	return true;
}

bool ParaEngine::RenderDeviceNull::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t, uint32_t)
{
	if (pDataOut && nWidth > 0 && nHeight > 0)
	{
		memset(pDataOut, 0, static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4u);
	}
	return true;
}

int ParaEngine::RenderDeviceNull::GetMaxSimultaneousTextures()
{
	return static_cast<int>(m_caps.maxTextureStages);
}

bool ParaEngine::RenderDeviceNull::SetTexture(uint32_t, DeviceTexturePtr_type)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawPrimitive(EPrimitiveType, uint32_t, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawIndexedPrimitive(EPrimitiveType, int, uint32_t, uint32_t, uint32_t, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawPrimitiveUP(EPrimitiveType, uint32_t, const void*, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::DrawIndexedPrimitiveUP(EPrimitiveType, uint32_t, uint32_t, uint32_t, const void*, PixelFormat, const void*, uint32_t)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetTransform(ETransformsStateType, DeviceMatrix_ptr)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetFVF(uint32_t)
{
	return true;
}

void ParaEngine::RenderDeviceNull::SetCursorPosition(int, int, uint32_t)
{
}

bool ParaEngine::RenderDeviceNull::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
{
	m_stateTracker.SetSamplerState(stage, type, value);
	return true;
}

bool ParaEngine::RenderDeviceNull::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
{
	if (value)
		*value = m_stateTracker.GetSamplerState(stage, type);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetVertexDeclaration(VertexDeclarationPtr)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
{
	if (!ppDecl)
		return false;
	*ppDecl = new CVertexDeclaration(pVertexElements);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetIndices(IndexBufferDevicePtr_type)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetStreamSource(uint32_t, VertexBufferDevicePtr_type, uint32_t, uint32_t)
{
	return true;
}

ParaViewport ParaEngine::RenderDeviceNull::GetViewport()
{
	return m_stateTracker.GetViewport();
}

bool ParaEngine::RenderDeviceNull::SetViewport(const ParaViewport& viewport)
{
	m_stateTracker.SetViewport(viewport);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearColor(const Color4f& color)
{
	m_stateTracker.SetClearColor(color);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearDepth(const float depth)
{
	m_stateTracker.SetClearDepth(depth);
	return true;
}

bool ParaEngine::RenderDeviceNull::SetClearStencil(const int stencil)
{
	m_stateTracker.SetClearStencil(stencil);
	return true;
}

bool ParaEngine::RenderDeviceNull::Clear(bool, bool, bool)
{
	return true;
}

bool ParaEngine::RenderDeviceNull::SetScissorRect(RECT* pRect)
{
	m_stateTracker.SetScissorRect(pRect);
	return true;
}

bool ParaEngine::RenderDeviceNull::GetScissorRect(RECT* pRect)
{
	if (pRect)
		*pRect = m_stateTracker.GetScissorRect();
	return true;
}

bool ParaEngine::RenderDeviceNull::BeginScene()
{
	return true;
}

bool ParaEngine::RenderDeviceNull::EndScene()
{
	return true;
}

bool ParaEngine::RenderDeviceNull::Present()
{
	return true;
}

void ParaEngine::RenderDeviceNull::Flush()
{
}
