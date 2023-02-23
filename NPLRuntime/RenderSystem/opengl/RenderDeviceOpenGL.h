#pragma once
#include "Framework/Interface/Render/IRenderDevice.h"
#include "OpenGL.h"


namespace ParaEngine
{
	class RenderDeviceOpenGL : public IRenderDevice
	{

	public:
		RenderDeviceOpenGL();
		virtual ~RenderDeviceOpenGL();
		virtual uint32_t GetRenderState(const ERenderState& State) override;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) override;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) override;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) override;
		virtual int GetMaxSimultaneousTextures() override;
		virtual bool SetTexture(uint32_t stage, DeviceTexturePtr_type texture) override;
		virtual bool DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) override;
		virtual bool DrawPrimitiveUP_GL(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride);
		virtual bool DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value) override;
		virtual bool SetVertexDeclaration(CVertexDeclaration* pVertexDeclaration) override;
		virtual bool CreateVertexDeclaration(VertexElement* pVertexElements, CVertexDeclaration** ppDecl) override;
		virtual bool SetIndices(IndexBufferDevicePtr_type pIndexData) override;
		virtual bool SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride) override;
		void BeginRenderTarget(uint32_t width,uint32_t height) ;
		void EndRenderTarget();
		virtual bool BeginScene() override;
		virtual bool EndScene() override;
		int GetStencilBits();
		virtual bool DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount) override;
		virtual bool DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) override;
		virtual bool DrawIndexedPrimitiveUP_GL(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void * pIndexData, const int nIndexSize, const void* pVertexStreamZeroData, const int nVertexSize);
		virtual bool SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix) override;
		virtual bool SetFVF(uint32_t FVF) override;
		virtual void SetCursorPosition(int X, int Y, uint32_t Flags) override;
		virtual bool GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value) override;
		virtual ParaViewport GetViewport() override;
		virtual bool SetViewport(const ParaViewport& viewport) override;
		virtual bool Clear(bool color, bool depth, bool stencil) override;
		virtual bool SetScissorRect(RECT* pRect) override;
		virtual bool GetScissorRect(RECT* pRect) override;
		virtual bool Present() override  = 0;
		virtual bool SetClearColor(const Color4f& color) override;
		virtual bool SetClearDepth(const float depth) override;
		virtual bool SetClearStencil(const int stencil) override;
		virtual void Flush() override;
		virtual void ApplyBlendingModeChange();
	protected:
		void BindVAO(const void* vertex_data = nullptr, const int vertex_size = 0, const void* index_data = nullptr, const int index_size = 0);
	private:
		bool m_AlphaBlendingChanged;
		bool m_BlendingChanged;
		bool m_EnableBlending;
		bool m_EnableSeparateAlphaBlending;
		uint32_t m_BlendingSource;
		uint32_t m_BlendingDest;
		uint32_t m_BlendingAlphaSource;
		uint32_t m_BlendingAlphaDest;
		uint32_t m_StencilRefValue;
		uint32_t m_StencilPass;
		uint32_t m_SamplerStates[8][8] = {};
		ParaEngine::CVertexDeclaration* m_CurrentVertexDeclaration;
		VertexBufferDevicePtr_type m_CurrentVertexBuffer;
		IndexBufferDevicePtr_type m_CurrentIndexBuffer;
		uint32_t m_RenderTargetWidth;
		uint32_t m_RenderTargetHeight;
		uint32_t m_LastRenderTargetWidth;
		uint32_t m_LastRenderTargetHeight;
		bool m_isBeginRenderTarget ;
		ParaViewport m_CurrentViewPort;
		GLuint m_vao;
	};

	inline RenderDeviceOpenGL* GETGL(IRenderDevice* device)
	{
		return (RenderDeviceOpenGL*)device;
	}
}