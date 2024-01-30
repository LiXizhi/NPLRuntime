#include <stdexcept>
#include "ParaEngine.h"
#include "renderer/VertexDeclarationOpenGL.h"
#include "math/ParaViewport.h"
#include "RenderDeviceOpenGL.h"
#include "ViewportManager.h"
#include "OpenGLWrapper/GLType.h"
#include "OpenGLWrapper/GLTexture2D.h"
#include <stack>

using namespace ParaEngine;

//#define EMSCRIPTEN
/** define this, then DrawPrimitiveUP and DrawIndexedPrimitiveUP will use vertex buffer object. 
* client-side array (user pointer) is no longer supported in opengl 3.0 and Emscripten OpenGL ES2.
* we have to simulate it with vbo, but opengl VBO performance is very poor compared with client-side array. 
* Lots of discussions on the web, but no one is providing a sound solution to simulate client-side array with VBO. 
* We still recommend the good old client-side array where possible, since there is no equivalent VBO version. 
* only use VBO for emscripten. 
*/
#ifdef EMSCRIPTEN
#define USE_USER_POINTER_VBO
#endif

namespace ParaEngine
{
	/** for managing opengl vertex and index buffer object. */
	class CDynamicBufferObject
	{
	public:
		CDynamicBufferObject(int nBufferType = GL_ARRAY_BUFFER)
			: m_nBufferType(nBufferType), m_nCurrentSize(0), m_devicePointer(0), m_nNextFreeIndex(0), 
			m_nMinBufferSize(1024*100), // 100KB at least
			m_bUseRingBuffer(true)
		{
		};
		void GenerateBuffer()
		{
			glGenBuffers(1, &m_devicePointer);
			m_nCurrentSize = 0;
			m_nNextFreeIndex = 0;
		}
		
		void UploadUserPointer(int nCountBytes, const void* pData, int nFrom = 0)
		{
			int nSize = nCountBytes + nFrom;
			if (m_nCurrentSize < nSize)
			{
				int nTotalSize = (m_bUseRingBuffer && (m_nMinBufferSize > nSize)) ? m_nMinBufferSize : nSize;
				if (nFrom == 0 && nTotalSize == nSize) {
					glBufferData(m_nBufferType, nSize, pData, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
				}
				else
				{
					glBufferData(m_nBufferType, nTotalSize, nullptr, m_bUseRingBuffer ? GL_STREAM_DRAW : GL_DYNAMIC_DRAW);
					glBufferSubData(m_nBufferType, nFrom, nCountBytes, pData);
				}
				m_nCurrentSize = nSize;
			}
			else
			{
				if (m_bUseRingBuffer) {
					glBufferSubData(m_nBufferType, nFrom, nCountBytes, pData);
				}
				else
				{
#ifdef ORPHAN_VBO
					PE_ASSERT(nSize == nCountBytes);
					glBufferData(m_nBufferType, nSize, nullptr, GL_DYNAMIC_DRAW);
					glBufferData(m_nBufferType, nSize, pData, GL_DYNAMIC_DRAW);
#else
					glBufferSubData(m_nBufferType, nFrom, nCountBytes, pData);
#endif
				}

			}
			PE_CHECK_GL_ERROR_DEBUG();
			
			m_nNextFreeIndex = m_bUseRingBuffer ? nSize : 0;
		}

		/** @return return the beginning buffer offset */
		int UploadUserPointerRingBuffer(int nCount, const void* pData, int VertexStreamZeroStride)
		{
			int nOffsetIndex = (m_nNextFreeIndex == 0) ? 0 : ((int)(m_nNextFreeIndex / VertexStreamZeroStride) + 1);
			int nFromBytes = nOffsetIndex * VertexStreamZeroStride;
			int nTotalBytes = nCount * VertexStreamZeroStride;
			int nSize = nFromBytes + nTotalBytes;
			if (nSize < m_nCurrentSize)
			{
				// append to buffer
				UploadUserPointer(nCount * VertexStreamZeroStride, pData, nFromBytes);
			}
			else
			{
				// restart from beginning
				nOffsetIndex = 0;
				UploadUserPointer(nCount * VertexStreamZeroStride, pData, 0);
			}
			return nOffsetIndex;
		}

		VertexBufferDevicePtr_type GetDevicePointer() 
		{
			if (m_devicePointer == 0)
			{
				GenerateBuffer();
			}
			return m_devicePointer;
		}

		void EnableRingBuffer(bool bEnable) 
		{
			m_bUseRingBuffer = bEnable;
		}
	private:
		int m_nBufferType;
		int m_nCurrentSize;
		int m_nNextFreeIndex;
		VertexBufferDevicePtr_type m_devicePointer;
		bool m_bUseRingBuffer;
		int m_nMinBufferSize;
	};
}
// global buffer used by DrawPrimitiveUp as verter buffer object. 
CDynamicBufferObject m_userPointerVertexBuffer;
CDynamicBufferObject m_userPointerIndexBuffer(GL_ELEMENT_ARRAY_BUFFER);


namespace ParaEngine
{
	uint32_t ToGLBlendValue(uint32_t value)
	{
		switch (value)
		{
		case RSV_BLEND_ZERO:
			return GL_ZERO;
		case RSV_BLEND_ONE:
			return GL_ONE;
		case RSV_BLEND_SRCCOLOR:
			return GL_SRC_COLOR;
		case RSV_BLEND_INVSRCCOLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case RSV_BLEND_SRCALPHA:
			return GL_SRC_ALPHA;
		case RSV_BLEND_INVSRCALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case RSV_BLEND_DESTALPHA:
			return GL_DST_ALPHA;
		case RSV_BLEND_INVDESTALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case RSV_BLEND_DESTCOLOR:
			return GL_DST_COLOR;
		case RSV_BLEND_INVDESTCOLOR:
			return GL_ONE_MINUS_DST_COLOR;
		case RSV_BLEND_SRCALPHASAT:
			return GL_SRC_ALPHA_SATURATE;
		//case RSV_BLEND_BOTHSRCALPHA:
		//case RSV_BLEND_BOTHINVSRCALPHA:
		//case RSV_BLEND_BLENDFACTOR:
		//case RSV_BLEND_INVBLENDFACTOR:
		default:
			assert(false);//Unkonw blend value
			break;
		}
		return 0;
	}
}

bool ParaEngine::IRenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
	GLenum __error = glGetError();
	if (__error)
	{
		ParaEngine::CLogger::GetSingleton().WriteFormated("OpenGL error 0x%04X in %s %s %d\n", __error, filename, func, nLine);
		return false;
	}
	return true;
}



ParaEngine::RenderDeviceOpenGL::RenderDeviceOpenGL()
	:m_AlphaBlendingChanged(true)
	,m_EnableBlending(false)
	,m_EnableSeparateAlphaBlending(false)
	,m_BlendingChanged(true)
	,m_BlendingSource(RSV_BLEND_SRCALPHA)
	,m_BlendingDest(RSV_BLEND_INVSRCALPHA)
	,m_BlendingAlphaSource(RSV_BLEND_SRCALPHA)
	,m_BlendingAlphaDest(RSV_BLEND_INVSRCALPHA)
	,m_StencilPass(0)
	,m_StencilRefValue(0)
	,m_CurrentIndexBuffer(-1)
	,m_CurrentVertexBuffer(-1)
	,m_CurrentVertexDeclaration(0)
{
	for (int i = 0;i<8;i++)
	{
		for (int j =0;j<8;j++)
		{
			m_SamplerStates[i][j] = 0;
		}
	}
}

ParaEngine::RenderDeviceOpenGL::~RenderDeviceOpenGL()
{
	GL::ClearCache();
}

uint32_t ParaEngine::RenderDeviceOpenGL::GetRenderState(const ERenderState& State)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return 0;
}

bool ParaEngine::RenderDeviceOpenGL::SetRenderState(const ERenderState State, const uint32_t Value)
{
	switch (State)
	{
	case ERenderState::CULLMODE:
		{
			//http://www.cnblogs.com/onlyDIABLO/p/3853125.html
			if (Value == RSV_CULL_CW) {
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);//glCullFace(GL_FRONT);
			}
			else if (Value == RSV_CULL_CCW) {
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);//glCullFace(GL_BACK);
			}
			else if (Value == RSV_CULL_NONE) {
				glDisable(GL_CULL_FACE);
			}
		}
		break;
	case ERenderState::ZENABLE:
	{
		// z test enable
		if (Value)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		break;
	}
	case ERenderState::ZFUNC:
	{
		// z test enable
		if (Value == RSV_CMP_LESSEQUAL)
			glDepthFunc(GL_LEQUAL);
		else if (Value == RSV_CMP_GREATER)
			glDepthFunc(GL_GREATER);
		else if (Value == RSV_CMP_GREATEREQUAL)
			glDepthFunc(GL_GEQUAL);
		else if (Value == RSV_CMP_LESS)
			glDepthFunc(GL_LESS);
		else if (Value == RSV_CMP_NEVER)
			glDepthFunc(GL_NEVER);
		else if (Value == RSV_CMP_NOTEQUAL)
			glDepthFunc(GL_NOTEQUAL);
		else if (Value == RSV_CMP_ALWAYS)
			glDepthFunc(GL_ALWAYS);
		else
			glDepthFunc(GL_LEQUAL);
		break;
	}
	case ERenderState::ZWRITEENABLE:
	{
		// z write enable
		glDepthMask(Value ? GL_TRUE : GL_FALSE);
		break;
	}
	case ERenderState::SEPARATEALPHABLENDENABLE:
	{
		bool bEnabled = Value ? true : false;
		if (bEnabled == m_EnableSeparateAlphaBlending)break;
		m_EnableSeparateAlphaBlending = bEnabled;
		if (bEnabled && !m_EnableBlending)
			SetRenderState(ERenderState::ALPHABLENDENABLE, true);
		/*if (bEnabled)
		{
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}*/
		break;
	}
	case ERenderState::ALPHABLENDENABLE:
	{
		bool bEnabled = Value ? true : false;
		if (bEnabled == m_EnableBlending) break;
		m_EnableBlending = bEnabled;
		if (bEnabled)
		{
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
		
		break;
	}
	case ERenderState::SRCBLENDALPHA:
	{
		if (Value == m_BlendingAlphaSource)break;
		m_BlendingAlphaSource = Value;
		m_AlphaBlendingChanged = true;
	}
	break;
	case ERenderState::DESTBLENDALPHA:
	{
		if (Value == m_BlendingAlphaDest)break;
		m_BlendingAlphaDest = Value;
		m_AlphaBlendingChanged = true;
	}
	break;
	case ERenderState::SRCBLEND:
	{
		if (Value == m_BlendingSource)break;
		m_BlendingSource = Value;
		m_BlendingChanged = true;
	}
	break;
	case ERenderState::DESTBLEND:
	{
		if (Value == m_BlendingDest)break;
		m_BlendingDest = Value;
		m_BlendingChanged = true;
	}
	break;
	case ERenderState::STENCILENABLE:
	{
		if (Value)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
		break;
	}
	case ERenderState::STENCILFUNC:
	{
		if (Value == RSV_CMP_EQUAL)
		{
			// draw only where stencil's value is Value
			glStencilFunc(GL_EQUAL, m_StencilRefValue, 0xFF);
		}
		else if (Value == RSV_CMP_ALWAYS)
		{
			glStencilFunc(GL_ALWAYS, m_StencilRefValue, 0xFF);
		}
		else if (Value == RSV_CMP_NEVER)
		{
			glStencilFunc(GL_NEVER, m_StencilRefValue, 0xFF);
		}
		break;
	}
	case ERenderState::STENCILWRITEMASK:
	{
		glStencilMask(Value);
		break;
	}
	case ERenderState::STENCILPASS:
	{
		if (Value == RSV_STENCILOP_REPLACE)
		{
			m_StencilPass = GL_REPLACE;
			glStencilOp(m_StencilPass, GL_KEEP, GL_KEEP);
		}
		break;
	}
	case ERenderState::STENCILREF:
	{
		m_StencilRefValue = Value;
		break;
	}
	case ERenderState::SCISSORTESTENABLE:
	{
		if (Value)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
		break;
	}
	case ERenderState::COLORWRITEENABLE:
	{
		glColorMask((Value & 0x1) ? GL_TRUE : GL_FALSE, (Value & 0x2) ? GL_TRUE : GL_FALSE, (Value & 0x4) ? GL_TRUE : GL_FALSE, (Value & 0x8) ? GL_TRUE : GL_FALSE);
		break;
	}
	default:
		//assert(false,"Unknow render state.");
		break;
	}

	return S_OK;
}

bool ParaEngine::RenderDeviceOpenGL::SetClipPlane(uint32_t Index, const float* pPlane)
{
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, uint32_t nDataFormat /*= 0*/, uint32_t nDataType /*= 0*/)
{
	// needs to invert Y, since opengl use upward Y.
	glReadPixels(nLeft, m_RenderTargetHeight - nTop - nHeight, nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(pDataOut));
	return true;
}

int ParaEngine::RenderDeviceOpenGL::GetMaxSimultaneousTextures()
{
	return 4;
}


bool ParaEngine::RenderDeviceOpenGL::SetTexture(uint32_t stage, DeviceTexturePtr_type texture)
{
	texture->bindN(texture, stage);

	PE_CHECK_GL_ERROR_DEBUG();
	return true;
}



bool ParaEngine::RenderDeviceOpenGL::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
#ifdef USE_USER_POINTER_VBO
	unsigned int nVertexCount = 0;
	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		nVertexCount = PrimitiveCount * 3;
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		nVertexCount = PrimitiveCount + 2;
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		nVertexCount = PrimitiveCount + 2;
	else if (PrimitiveType == EPrimitiveType::LINELIST)
		nVertexCount = PrimitiveCount * 2;

#if defined(DEBUG)
	{
		// just to safe-check that NumVertices is passed properly
		auto pIndices = (unsigned short*)pIndexData;
		uint32_t maxVertexIndex = pIndices[nVertexCount - 1];
		for (int i = 0; i < nVertexCount; ++i)
		{
			if (pIndices[i] > maxVertexIndex)
				maxVertexIndex = pIndices[i];
		}
		PE_ASSERT((maxVertexIndex - MinVertexIndex + 1) == NumVertices);
		NumVertices = maxVertexIndex - MinVertexIndex + 1;
	}
#endif
	m_userPointerIndexBuffer.UploadUserPointer(nVertexCount * sizeof(uint16), pIndexData);
	m_userPointerVertexBuffer.UploadUserPointer(NumVertices * VertexStreamZeroStride, pVertexStreamZeroData, MinVertexIndex * VertexStreamZeroStride);
	pIndexData = nullptr; // pass null to use binded buffer object
#else
	if (m_CurrentVertexDeclaration)
	{
		m_CurrentVertexDeclaration->ApplyAttribute(pVertexStreamZeroData);
	}
#endif
	ApplyBlendingModeChange();
	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		glDrawElements(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		glDrawElements(GL_TRIANGLE_STRIP, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		glDrawElements(GL_TRIANGLE_FAN, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::LINELIST)
		glDrawElements(GL_LINES, PrimitiveCount * 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));

	PE_CHECK_GL_ERROR_DEBUG();

	return true;
}


bool ParaEngine::RenderDeviceOpenGL::DrawPrimitive(EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
{
	ApplyBlendingModeChange();
	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		glDrawArrays(GL_TRIANGLES, StartVertex, PrimitiveCount * 3);
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		glDrawArrays(GL_TRIANGLE_STRIP, StartVertex, PrimitiveCount + 2);
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		glDrawArrays(GL_TRIANGLE_FAN, StartVertex, PrimitiveCount + 2);

	PE_CHECK_GL_ERROR_DEBUG();

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	ApplyBlendingModeChange();
	int nOffset = 0;
#ifdef USE_USER_POINTER_VBO
	unsigned int nVertexCount = 0;
	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		nVertexCount = PrimitiveCount * 3;
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		nVertexCount = PrimitiveCount + 2;
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		nVertexCount = PrimitiveCount + 2;

	nOffset = m_userPointerVertexBuffer.UploadUserPointerRingBuffer(nVertexCount, pVertexStreamZeroData, VertexStreamZeroStride);
#else
	if (m_CurrentIndexBuffer)
	{
		SetIndices(0);
	}
	if (m_CurrentVertexBuffer)
	{
		SetStreamSource(0, 0, 0, 0);
	}
	if (m_CurrentVertexDeclaration)
	{
		m_CurrentVertexDeclaration->ApplyAttribute(pVertexStreamZeroData);
	}
#endif
	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		glDrawArrays(GL_TRIANGLES, nOffset, PrimitiveCount * 3);
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		glDrawArrays(GL_TRIANGLE_STRIP, nOffset, PrimitiveCount + 2);
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		glDrawArrays(GL_TRIANGLE_FAN, nOffset, PrimitiveCount + 2);

	PE_CHECK_GL_ERROR_DEBUG();

	return true;

}

bool ParaEngine::RenderDeviceOpenGL::SetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t value)
{
	switch (type)
	{
	case ParaEngine::ESamplerStateType::ADDRESSU:
	case ParaEngine::ESamplerStateType::ADDRESSV:
	case ParaEngine::ESamplerStateType::MAGFILTER:
	case ParaEngine::ESamplerStateType::MINFILTER:
		m_SamplerStates[stage][(int)type] = value;
		break;
	default:
		break;
	}
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetVertexDeclaration(CVertexDeclaration* pVertexDeclaration)
{
	if (pVertexDeclaration)
	{
		glBindVertexArray(0);
		pVertexDeclaration->EnableAttribute();
#ifdef USE_USER_POINTER_VBO
		auto currentVBO = m_CurrentVertexBuffer ? m_CurrentVertexBuffer : m_userPointerVertexBuffer.GetDevicePointer();
		glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
		pVertexDeclaration->ApplyAttribute();
#else
		pVertexDeclaration->ApplyAttribute();
#endif
		m_CurrentVertexDeclaration = pVertexDeclaration;
		//OUTPUT_LOG("RenderDeviceOpenGL: EnableAttribute & ApplyAttribute at SetVertexDeclaration");

		PE_CHECK_GL_ERROR_DEBUG();
	}

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::CreateVertexDeclaration(VertexElement* pVertexElements, CVertexDeclaration** ppDecl)
{
	CVertexDeclaration* pDecl = new CVertexDeclaration(pVertexElements);
	*ppDecl = pDecl;
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetIndices(IndexBufferDevicePtr_type pIndexData)
{
	if (pIndexData != m_CurrentIndexBuffer)
	{
		m_CurrentIndexBuffer = pIndexData;

#ifdef USE_USER_POINTER_VBO
		if (pIndexData == 0)
		{
			pIndexData = m_userPointerIndexBuffer.GetDevicePointer();
		}
#endif
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexData);

		PE_CHECK_GL_ERROR_DEBUG();
	}

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
{
	if (pStreamData != m_CurrentVertexBuffer)
	{
		m_CurrentVertexBuffer = pStreamData;
#ifdef USE_USER_POINTER_VBO
		if (pStreamData == 0)
		{
			pStreamData = m_userPointerVertexBuffer.GetDevicePointer();
		}
#endif
		glBindBuffer(GL_ARRAY_BUFFER, pStreamData);
		
		//OUTPUT_LOG("RenderDeviceOpenGL:glBindBuffer %d",pStreamData);
		if (m_CurrentVertexDeclaration)
		{		
			m_CurrentVertexDeclaration->EnableAttribute();
			m_CurrentVertexDeclaration->ApplyAttribute();
			//OUTPUT_LOG("RenderDeviceOpenGL: EnableAttribute & ApplyAttribute at SetStreamSource");
		}

		PE_CHECK_GL_ERROR_DEBUG();
	}

	return true;
}

std::stack<pair<uint32_t, uint32_t>> g_stackRenderTargetSizes;

void ParaEngine::RenderDeviceOpenGL::BeginRenderTarget(uint32_t width, uint32_t height)
{
	g_stackRenderTargetSizes.push(std::make_pair(m_RenderTargetWidth, m_RenderTargetHeight));
	m_RenderTargetWidth = width;
	m_RenderTargetHeight = height;
	m_isBeginRenderTarget = true;
}

void ParaEngine::RenderDeviceOpenGL::EndRenderTarget()
{
	m_isBeginRenderTarget = false;

	// safe pop last from queue
	if (!g_stackRenderTargetSizes.empty())
	{
		m_RenderTargetWidth = g_stackRenderTargetSizes.top().first;
		m_RenderTargetHeight = g_stackRenderTargetSizes.top().second;
		g_stackRenderTargetSizes.pop();
	}
}

bool ParaEngine::RenderDeviceOpenGL::BeginScene()
{
	// tricky: always render full screen
	if (CGlobals::GetApp()->IsRotateScreen())
	{
		m_RenderTargetWidth = CGlobals::GetApp()->GetRenderWindow()->GetHeight();
		m_RenderTargetHeight = CGlobals::GetApp()->GetRenderWindow()->GetWidth();
	}
	else
	{
		m_RenderTargetWidth = CGlobals::GetApp()->GetRenderWindow()->GetWidth();
		m_RenderTargetHeight = CGlobals::GetApp()->GetRenderWindow()->GetHeight();
	}
	
	if (!g_stackRenderTargetSizes.empty()) {
		std::stack<std::pair<uint32_t, uint32_t>> emptyStack;
		g_stackRenderTargetSizes.swap(emptyStack);
	}
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::EndScene()
{
	return true;
}

void ParaEngine::RenderDeviceOpenGL::Flush()
{
	glFinish();
}


int ParaEngine::RenderDeviceOpenGL::GetStencilBits()
{
	static bool once = true;
	static GLint g_sStencilBits = -1;
	if (once)
	{
		glGetIntegerv(GL_STENCIL_BITS, &g_sStencilBits);
		if (g_sStencilBits <= 0)
		{
			
		}
		once = false;
	}
	return g_sStencilBits;
}

bool ParaEngine::RenderDeviceOpenGL::DrawIndexedPrimitive(EPrimitiveType Type, int BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	ApplyBlendingModeChange();

	if (Type == EPrimitiveType::TRIANGLELIST)
		glDrawElements(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));
	else if (Type == EPrimitiveType::TRIANGLESTRIP)
		glDrawElements(GL_TRIANGLE_STRIP, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));
	else if (Type == EPrimitiveType::TRIANGLEFAN)
		glDrawElements(GL_TRIANGLE_FAN, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));

	PE_CHECK_GL_ERROR_DEBUG();

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetTransform(ETransformsStateType State, DeviceMatrix_ptr pMatrix)
{
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetFVF(uint32_t FVF)
{
	return true;
}

void ParaEngine::RenderDeviceOpenGL::SetCursorPosition(int X, int Y, uint32_t Flags)
{
}

bool ParaEngine::RenderDeviceOpenGL::GetSamplerState(uint32_t stage, ESamplerStateType type, uint32_t* value)
{
	if (value)
		*value = m_SamplerStates[stage][(int)type];
	return true;
}

ParaViewport ParaEngine::RenderDeviceOpenGL::GetViewport()
{
	return m_CurrentViewPort;
}

bool ParaEngine::RenderDeviceOpenGL::SetViewport(const ParaViewport& viewport)
{
	m_CurrentViewPort = viewport;
	glViewport((GLint)(viewport.X), (GLint)(m_RenderTargetHeight - viewport.Y - (GLsizei)viewport.Height), (GLsizei)(viewport.Width), (GLsizei)(viewport.Height));
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::Clear(bool color, bool depth, bool stencil)
{
	uint32_t fields = 0;
	if (depth)
	{
		fields |= GL_DEPTH_BUFFER_BIT;
	}
	if (color)
	{
		fields |= GL_COLOR_BUFFER_BIT;
	}
	if (stencil)
	{
		fields |= GL_STENCIL_BUFFER_BIT;
	}
	glClear(fields);

	PE_CHECK_GL_ERROR_DEBUG();

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetScissorRect(RECT* pRect)
{
	if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right <= 0 && pRect->bottom <= 0))
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		int nScreenWidth = m_RenderTargetWidth;
		int nScreenHeight = m_RenderTargetHeight;
		//int nViewportOffsetY = nScreenHeight - (m_CurrentViewPort.Y + m_CurrentViewPort.Height);
		//glScissor((GLint)(pRect->left + m_CurrentViewPort.X), (GLint)(nViewportOffsetY + nScreenHeight - pRect->bottom), (GLsizei)(pRect->right - pRect->left), (GLsizei)(pRect->bottom - pRect->top));
		glScissor((GLint)(pRect->left), (GLint)(nScreenHeight - pRect->bottom), (GLsizei)(pRect->right - pRect->left), (GLsizei)(pRect->bottom - pRect->top));
	}
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::GetScissorRect(RECT* pRect)
{

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetClearColor(const Color4f& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	PE_CHECK_GL_ERROR_DEBUG();
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetClearDepth(const float depth)
{
	glClearDepth(depth);
	PE_CHECK_GL_ERROR_DEBUG();
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetClearStencil(const int stencil)
{
	glClearStencil(stencil);
	PE_CHECK_GL_ERROR_DEBUG();
	return true;
}

void ParaEngine::RenderDeviceOpenGL::ApplyBlendingModeChange()
{
	
	if (m_AlphaBlendingChanged)
	{
		uint32_t asrc = ToGLBlendValue(m_BlendingAlphaSource);
		uint32_t adst = ToGLBlendValue(m_BlendingAlphaDest);

		uint32_t src = ToGLBlendValue(m_BlendingSource);
		uint32_t dst = ToGLBlendValue(m_BlendingDest);
		glBlendFuncSeparate(src, dst, asrc, adst);
		m_AlphaBlendingChanged = false;
	}else if (m_BlendingChanged)
	{
		uint32_t src = ToGLBlendValue(m_BlendingSource);
		uint32_t dst = ToGLBlendValue(m_BlendingDest);
		glBlendFunc(src, dst);
		m_BlendingChanged = false;
	}

}

RenderDevice* RenderDevice::GetInstance()
{
	static RenderDevice g_instance;
	return &g_instance;
}

HRESULT RenderDevice::DrawPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t StartVertex, uint32_t PrimitiveCount)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT RenderDevice::DrawPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT RenderDevice::DrawIndexedPrimitive(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType Type, int32_t BaseVertexIndex, uint32_t MinIndex, uint32_t NumVertices, uint32_t indexStart, uint32_t PrimitiveCount)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinIndex, NumVertices, indexStart, PrimitiveCount);
}

HRESULT RenderDevice::DrawIndexedPrimitiveUP(IRenderDevice* pRenderDevice, int nStatisticsType, EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void* pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return pRenderDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}