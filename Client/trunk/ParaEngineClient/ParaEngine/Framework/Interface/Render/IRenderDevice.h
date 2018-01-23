#pragma once

#include <memory>
#include <stdint.h>
#include "Framework/RenderSystem/RenderTypes.h"
#include "Framework/Common/Math/Rect.h"
#include "core/PEtypes.h"

#if USE_DIRECTX_RENDERER
#include "d3d9.h"
#else
typedef uint32_t DeviceTexturePtr_type;
typedef uint32_t VertexBufferDevicePtr_type;
typedef uint32_t IndexBufferDevicePtr_type;

//struct PARAMATRIX;
//struct PARAVECTOR2;
//struct PARAVECTOR3;
//struct PARAVECTOR4;
//
//typedef PARAMATRIX  DeviceMatrix;
//typedef PARAMATRIX*  DeviceMatrix_ptr;
//typedef PARAVECTOR2*  DeviceVector2_ptr;
//typedef PARAVECTOR3*  DeviceVector3_ptr;
//typedef PARAVECTOR4*  DeviceVector4_ptr;
//typedef PARAVECTOR2  DeviceVector2;
//typedef PARAVECTOR3  DeviceVector3;
//typedef PARAVECTOR4  DeviceVector4;
#endif

//#include "d3d9.h"


namespace ParaEngine
{
	class CVertexDeclaration;
	struct VertexElement;
	class ParaViewport;
	class IRenderContext;

	class IRenderDevice
	{
	public:
		IRenderDevice() = default;
		virtual ~IRenderDevice() = default;

		virtual uint32_t GetRenderState(const ERenderState& State) = 0;
		virtual bool SetRenderState(const ERenderState State, const uint32_t Value) = 0;
		virtual bool SetClipPlane(uint32_t Index, const float* pPlane) = 0;
		virtual bool ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat = 0, uint32_t nDataType = 0) = 0;
		virtual int GetMaxSimultaneousTextures() = 0;
		virtual bool SetTexture(uint32_t stage, DeviceTexturePtr_type texture) = 0;
		virtual bool DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount) = 0;
		virtual bool DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount) = 0;
		virtual bool DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount,const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride) = 0;
		virtual bool DrawIndexedPrimitiveUP(
			EPrimitiveType PrimitiveType,
			uint32_t MinVertexIndex,
			uint32_t NumVertices,
			uint32_t PrimitiveCount,
			const void * pIndexData,
			PixelFormat IndexDataFormat,
			const void* pVertexStreamZeroData,
			uint32_t VertexStreamZeroStride) = 0;

		virtual bool SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix) = 0;
		virtual bool SetFVF(uint32_t FVF) = 0;
		virtual void SetCursorPosition(int X, int Y, uint32_t Flags) = 0;

		virtual bool SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value) = 0;
		virtual bool GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value) = 0;

		virtual bool SetVertexDeclaration(CVertexDeclaration* pVertexDeclaration) = 0;
		virtual bool CreateVertexDeclaration(VertexElement* pVertexElements, CVertexDeclaration** ppDecl) = 0;
		virtual bool SetIndices(IndexBufferDevicePtr_type pIndexData) = 0;
		virtual bool SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride) = 0;
		virtual Rect GetViewport() = 0;
		virtual bool SetViewport(const Rect& viewport) = 0;

		virtual bool Clear(uint32_t Count, const void* pRects, uint32_t Flags, uint32_t Color, float Z, uint32_t Stencil) = 0;

		virtual bool SetScissorRect(RECT* pRect) = 0;
		virtual bool GetScissorRect(RECT* pRect) = 0;


		virtual bool BeginScene() = 0;
		virtual bool EndScene() = 0;

		virtual bool Present() = 0;
	};

	using IRenderDevicePtr = std::shared_ptr<IRenderDevice>;
}

