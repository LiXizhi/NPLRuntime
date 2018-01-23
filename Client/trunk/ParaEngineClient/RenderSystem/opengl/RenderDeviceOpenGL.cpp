#include <stdexcept>
#include "RenderDeviceOpenGL.h"
#include "Renderer/VertexDeclaration.h"
#include "Math/ParaViewport.h"
//#include "Math/ParaColor.h"
using namespace ParaEngine;

ParaEngine::RenderDeviceOpenGL::RenderDeviceOpenGL()
	:m_AlphaBlendingChanged(true)
	,m_EnableBlending(false)
	,m_EnableSeparateAlphaBlending(false)
{

}

uint32_t ParaEngine::RenderDeviceOpenGL::GetRenderState(const ERenderState& State)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool ParaEngine::RenderDeviceOpenGL::SetRenderState(const ERenderState State, const uint32_t Value)
{
	switch (State)
	{
	case ERenderState::CULLMODE:
		{
			if (Value == RSV_CULL_CW) {
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);//glCullFace(GL_FRONT);
			}
			else if (Value == RSV_CULL_CCW) {
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);//glCullFace(GL_BACK);
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
		if (m_EnableSeparateAlphaBlending != bEnabled)
		{
			m_EnableSeparateAlphaBlending = bEnabled;
			m_AlphaBlendingChanged = true;
		}
		break;
	}
	case ERenderState::ALPHABLENDENABLE:
	{
		bool bEnabled = Value ? true : false;
		if (m_EnableBlending != bEnabled)
		{
			m_EnableBlending = bEnabled;
			m_AlphaBlendingChanged = true;
		}
		break;
	}
	case ERenderState::SRCBLENDALPHA:
	case ERenderState::DESTBLENDALPHA:
	case ERenderState::SRCBLEND:
	case ERenderState::DESTBLEND:
	{
		GLenum glValue = RSV_BLEND_ONE;
		if (Value == RSV_BLEND_ONE)
			glValue = GL_ONE;
		else if (Value == RSV_BLEND_ZERO)
			glValue = GL_ZERO;
		else if (Value == RSV_BLEND_SRCALPHA)
			glValue = GL_SRC_ALPHA;
		else if (Value == RSV_BLEND_INVSRCALPHA)
			glValue = GL_ONE_MINUS_SRC_ALPHA;
		else if (Value == RSV_BLEND_DESTALPHA)
			glValue = GL_DST_ALPHA;
		else if (Value == RSV_BLEND_INVDESTALPHA)
			glValue = GL_ONE_MINUS_DST_ALPHA;

		if (State == ERenderState::SRCBLEND) {
			if (m_BlendingSource != glValue)
			{
				m_BlendingSource = glValue;
				m_AlphaBlendingChanged = true;
			}
		}
		else if (State == ERenderState::DESTBLEND) {
			if (m_BlendingDest != glValue) {
				m_BlendingDest = glValue;
				m_AlphaBlendingChanged = true;
			}
		}
		else if (State == ERenderState::SRCBLENDALPHA) {
			if (m_BlendingAlphaSource != glValue) {
				m_BlendingAlphaSource = glValue;
				m_AlphaBlendingChanged = true;
			}
		}
		else if (State == ERenderState::DESTBLENDALPHA) {
			if (m_BlendingAlphaDest != glValue) {
				m_BlendingAlphaDest = glValue;
				m_AlphaBlendingChanged = true;
			}
		}
		// blending mode is delayed until the next draw call.
		// ApplyBlendingModeChange();
		break;
	}
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
	return true;
}

int ParaEngine::RenderDeviceOpenGL::GetMaxSimultaneousTextures()
{
	return 4;
}

bool ParaEngine::RenderDeviceOpenGL::SetTexture(uint32_t stage, uint32_t texture)
{
	glActiveTexture(GL_TEXTURE0 + stage);
	glBindTexture(GL_TEXTURE, texture);
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
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::DrawPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t PrimitiveCount, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	ApplyBlendingModeChange();
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

	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		glDrawArrays(GL_TRIANGLES, 0, PrimitiveCount * 3);
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		glDrawArrays(GL_TRIANGLE_STRIP, 0, PrimitiveCount + 2);
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		glDrawArrays(GL_TRIANGLE_FAN, 0, PrimitiveCount + 2);

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
	glBindVertexArray(0);
	pVertexDeclaration->EnableAttribute();
	pVertexDeclaration->ApplyAttribute();
	m_CurrentVertexDeclaration = pVertexDeclaration;
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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexData);
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetStreamSource(uint32_t StreamNumber, VertexBufferDevicePtr_type pStreamData, uint32_t OffsetInBytes, uint32_t Stride)
{
	glBindBuffer(GL_ARRAY_BUFFER, pStreamData);
	return true;
	
}

void ParaEngine::RenderDeviceOpenGL::BeginRenderTarget(uint32_t width, uint32_t height)
{
	m_RenderTargetWidth = width;
	m_RenderTargetHeight = height;
	m_isBeginRenderTarget = true;

}

void ParaEngine::RenderDeviceOpenGL::EndRenderTarget()
{
	m_RenderTargetWidth = 0;
	m_RenderTargetHeight = 0;
	m_isBeginRenderTarget = false;
}

bool ParaEngine::RenderDeviceOpenGL::BeginScene()
{
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::EndScene()
{
	return true;

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
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::DrawIndexedPrimitiveUP(EPrimitiveType PrimitiveType, uint32_t MinVertexIndex, uint32_t NumVertices, uint32_t PrimitiveCount, const void * pIndexData, PixelFormat IndexDataFormat, const void* pVertexStreamZeroData, uint32_t VertexStreamZeroStride)
{
	ApplyBlendingModeChange();
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

	if (PrimitiveType == EPrimitiveType::TRIANGLELIST)
		glDrawElements(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::TRIANGLESTRIP)
		glDrawElements(GL_TRIANGLE_STRIP, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::TRIANGLEFAN)
		glDrawElements(GL_TRIANGLE_FAN, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == EPrimitiveType::LINELIST)
		glDrawElements(GL_LINES, PrimitiveCount * 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
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

Rect ParaEngine::RenderDeviceOpenGL::GetViewport()
{
	return m_CurrentViewPort;
}

bool ParaEngine::RenderDeviceOpenGL::SetViewport(const Rect& viewport)
{
	m_CurrentViewPort = viewport;
	glViewport((GLint)(viewport.x), (GLint)(viewport.y), (GLsizei)(viewport.z), (GLsizei)(viewport.w));
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::Clear(uint32_t Count, const void* pRects, uint32_t Flags, uint32_t Color, float Z, uint32_t Stencil)
{
	DWORD fields = 0;
	if ((Flags & CLEAR_ZBUFFER) != 0)
	{
		fields |= GL_DEPTH_BUFFER_BIT;
		glClearDepth(Z);
		SetRenderState(ERenderState::ZFUNC,RSV_CMP_LESSEQUAL);
	}
	if ((Flags & CLEAR_TARGET) != 0)
	{
		//LinearColor color(Color);
		glClearColor(0, 0, 0, 0);
		fields |= GL_COLOR_BUFFER_BIT;
	}
	if ((Flags & CLEAR_STENCIL) != 0)
	{
		glClearStencil(Stencil);
		fields |= GL_STENCIL_BUFFER_BIT;
	}

	if (m_isBeginRenderTarget)
	{
		if ((Flags & CLEAR_ZBUFFER) != 0)
			glDepthMask(GL_TRUE);
		glClear(fields);
	}

	return true;
}

bool ParaEngine::RenderDeviceOpenGL::SetScissorRect(RECT* pRect)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

bool ParaEngine::RenderDeviceOpenGL::GetScissorRect(RECT* pRect)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return true;
}

void ParaEngine::RenderDeviceOpenGL::ApplyBlendingModeChange()
{
	if (m_AlphaBlendingChanged)
	{
		m_AlphaBlendingChanged = false;
		if (m_EnableBlending)
		{
			if (m_EnableSeparateAlphaBlending)
			{
				glEnable(GL_BLEND);
				glBlendFuncSeparate(m_BlendingSource, m_BlendingDest, m_BlendingAlphaSource, m_BlendingAlphaDest);
			}
			else
			{
				if (m_BlendingSource == GL_ONE && m_BlendingDest == GL_ZERO)
				{
					glDisable(GL_BLEND);
				}
				else
				{
					glEnable(GL_BLEND);
					glBlendFunc(m_BlendingSource, m_BlendingDest);
				}
			}
		}
		else
		{
			glDisable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ZERO);
		}
	}
}

