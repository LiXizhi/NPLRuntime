//-----------------------------------------------------------------------------
// Class: RenderDevice
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date: 2014.9.11
// Desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef USE_DIRECTX_RENDERER
#ifdef USE_OPENGL_RENDERER
#include "OpenGLWrapper.h"
#endif
#include "VertexDeclarationOpenGL.h"
#include "IParaEngineApp.h"
#include "ViewportManager.h"
#include "RenderDeviceOpenGL.h"

using namespace ParaEngine;


/** define this to ignore all draw calls to find program bottleneck. If defined, but FPS is still low, it is CPU bound. */
// #define DEBUG_IGNORE_DRAW_CALL
/** define this to draw only one triangle in each draw calls to find program bottleneck. If defined, but FPS is still low, it is CPU-GPU bus bottleneck, just reduce draw call count. */
// #define DEBUG_DRAW_CALL_SINGLE_TRIANGLE

/** define this will enable error check in debug mode. yet breaks parallelism between CPU and GPU.
you may notice a significant drop of FPS when issuing many draw calls, such as 500-1000 calls. */
// #define ALLOW_RENDER_ERROR_CHECK

/* glGetError()
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505
*/

// some render state caches
namespace ParaEngine
{
#define MAX_SAMPLER_COUNT 8
#define MAX_SAMPLER_STATES_COUNT 8
	static VertexDeclarationPtr s_currentVertexDeclaration = 0;
	static VertexBufferDevicePtr_type s_currentVertexBuffer = 0;
	static IndexBufferDevicePtr_type s_currentIndexBuffer = 0;
	static DWORD    s_blendingSource = GL_SRC_ALPHA;
	static DWORD    s_blendingDest = GL_ONE_MINUS_SRC_ALPHA;
	static DWORD    s_blendingAlphaSource = GL_SRC_ALPHA;
	static DWORD    s_blendingAlphaDest = GL_ONE_MINUS_SRC_ALPHA;
	static bool		s_bEnableSeparateAlphaBlending = false;
	static bool		s_bAlphaBlendingChanged = true;
	static bool		s_bEnableBlending = true;
	static DWORD    s_currentCullingMode = D3DCULL_NONE;
	static DWORD	s_sampler_states[MAX_SAMPLER_COUNT][MAX_SAMPLER_STATES_COUNT] = {};
	static DWORD	s_stencil_ref_value = 0;
	static DWORD	s_stencil_pass = 0;
	static D3DVIEWPORT9 s_currentViewport = {0,0,0,0,0.f,1.f};
	static RECT		s_scissorRect = {0,0,0,0};
	static bool		s_isBeginRenderTarget = false;
	static int		s_render_target_width = 0;
	static int		s_render_target_height = 0;
}

ParaEngine::RenderDevice::RenderDevice()
{
#ifdef ALLOW_RENDER_ERROR_CHECK
#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0
#else
	OUTPUT_LOG("Opengl ERROR_CHECK enabled. should be disabled in release mode\n");
#endif
#endif
}

RenderDevice* RenderDevice::GetInstance()
{
	static RenderDevice g_instance;
	return &g_instance;
}

HRESULT RenderDevice::DrawPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	ApplyBlendingModeChange();


	// NOT tested code!!!!
	if (PrimitiveType == D3DPT_TRIANGLELIST)
		glDrawArrays(GL_TRIANGLES, StartVertex, PrimitiveCount * 3);
	else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		glDrawArrays(GL_TRIANGLE_STRIP, StartVertex, PrimitiveCount + 2);
	else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		glDrawArrays(GL_TRIANGLE_FAN, StartVertex, PrimitiveCount + 2);




	PE_CHECK_GL_ERROR_DEBUG();
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return S_OK;
}

HRESULT RenderDevice::DrawPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	ApplyBlendingModeChange();
	if (s_currentIndexBuffer)
		pd3dDevice->SetIndices(0);
	if (s_currentVertexBuffer)
		pd3dDevice->SetStreamSource(0, 0, 0, 0);
	if (s_currentVertexDeclaration)
		s_currentVertexDeclaration->ApplyAttribute(pVertexStreamZeroData);

	if (PrimitiveType == D3DPT_TRIANGLELIST)
		glDrawArrays(GL_TRIANGLES, 0, PrimitiveCount * 3);
	else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		glDrawArrays(GL_TRIANGLE_STRIP, 0, PrimitiveCount + 2);
	else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		glDrawArrays(GL_TRIANGLE_FAN, 0, PrimitiveCount + 2);

	PE_CHECK_GL_ERROR_DEBUG();
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return S_OK;
}

HRESULT RenderDevice::DrawIndexedPrimitive(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinIndex, UINT NumVertices, UINT indexStart, UINT PrimitiveCount)
{
	ApplyBlendingModeChange();
#ifdef DEBUG_DRAW_CALL_SINGLE_TRIANGLE
	PrimitiveCount = 1;
#endif
	// opengl manual: see http://www.informit.com/articles/article.aspx?p=2033340&seqNum=4
	// glDrawElementsBaseVertex not supported in OpenGL ES 2.0
	if (BaseVertexIndex == 0)
	{
#ifndef DEBUG_IGNORE_DRAW_CALL
		if (PrimitiveType == D3DPT_TRIANGLELIST)
			glDrawElements(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
			glDrawElements(GL_TRIANGLE_STRIP, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
			glDrawElements(GL_TRIANGLE_FAN, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart));
#endif
	}
	else
	{
#if ! defined(PARAENGINE_MOBILE) && ! defined(USE_NULL_RENDERER)
		if (PrimitiveType == D3DPT_TRIANGLELIST)
			glDrawElementsBaseVertex(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart), BaseVertexIndex);
		else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
			glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart), BaseVertexIndex);
		else if (PrimitiveType == D3DPT_TRIANGLEFAN)
			glDrawElementsBaseVertex(GL_TRIANGLE_FAN, PrimitiveCount + 2, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16)*indexStart), BaseVertexIndex);
#else
		// OpenGL ES2.0 Fix: glDrawElementsBaseVertex not supported in opengl es.
		// inform the developer to avoid using vertex offset, instead one should only use index offset.
		OUTPUT_LOG("error: glDrawElementsBaseVertex not supported in opengl es.");
		PE_ASSERT(false);
#endif
	}
	PE_CHECK_GL_ERROR_DEBUG();
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return S_OK;
}

HRESULT RenderDevice::DrawIndexedPrimitiveUP(RenderDevicePtr pd3dDevice, int nStatisticsType, D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void * pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	ApplyBlendingModeChange();
	if (s_currentIndexBuffer)
		pd3dDevice->SetIndices(0);
	if (s_currentVertexBuffer)
		pd3dDevice->SetStreamSource(0, 0, 0, 0);
	if (s_currentVertexDeclaration)
		s_currentVertexDeclaration->ApplyAttribute(pVertexStreamZeroData);

	if (PrimitiveType == D3DPT_TRIANGLELIST)
		glDrawElements(GL_TRIANGLES, PrimitiveCount * 3, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == D3DPT_TRIANGLESTRIP)
		glDrawElements(GL_TRIANGLE_STRIP, PrimitiveCount+2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == D3DPT_TRIANGLEFAN)
		glDrawElements(GL_TRIANGLE_FAN, PrimitiveCount+2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	else if (PrimitiveType == D3DPT_LINELIST)
		glDrawElements(GL_LINES, PrimitiveCount * 2, GL_UNSIGNED_SHORT, (GLvoid*)(pIndexData));
	PE_CHECK_GL_ERROR_DEBUG();
	IncrementDrawBatchAndVertices(1, PrimitiveCount, nStatisticsType);
	return S_OK;
}

HRESULT RenderDevice::SetClipPlane(DWORD Index, const float* pPlane)
{
	return S_OK;
}
HRESULT RenderDevice::SetTexture(DWORD Stage, DeviceTexturePtr_type pTexture)
{
#ifdef USE_OPENGL_RENDERER
	GL::bindTexture2DN(Stage, pTexture);
#endif
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	if (pValue)
		*pValue = s_sampler_states[Sampler][Type];
	return S_OK;
}

HRESULT RenderDevice::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	switch (Type)
	{
	case D3DSAMP_MINFILTER:
		s_sampler_states[Sampler][Type] = Value;
		break;
	case D3DSAMP_MAGFILTER:
		s_sampler_states[Sampler][Type] = Value;
		break;
	case D3DSAMP_ADDRESSU:
		s_sampler_states[Sampler][Type] = Value;
		break;
	case D3DSAMP_ADDRESSV:
		s_sampler_states[Sampler][Type] = Value;
		break;
	default:
		break;
	}
	return S_OK;
}

HRESULT RenderDevice::SetIndices(IndexBufferDevicePtr_type pIndexData)
{
	if (pIndexData != s_currentIndexBuffer)
	{
		s_currentIndexBuffer = pIndexData;
#ifdef USE_OPENGL_RENDERER
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexData);
		PE_CHECK_GL_ERROR_DEBUG();
#endif

	}
	return S_OK;
}

HRESULT RenderDevice::SetStreamSource(UINT StreamNumber, VertexBufferDevicePtr_type pStreamData, UINT OffsetInBytes, UINT Stride)
{
	if (pStreamData != s_currentVertexBuffer)
	{
		s_currentVertexBuffer = pStreamData;
#ifdef USE_OPENGL_RENDERER
		glBindBuffer(GL_ARRAY_BUFFER, pStreamData);
		PE_CHECK_GL_ERROR_DEBUG();
		if (pStreamData && s_currentVertexDeclaration)
		{
			s_currentVertexDeclaration->EnableAttribute();
			s_currentVertexDeclaration->ApplyAttribute();
			PE_CHECK_GL_ERROR_DEBUG();
		}
#endif
	}
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::SetVertexDeclaration(VertexDeclarationPtr pDecl)
{
	if (pDecl != 0)
	{
#ifdef USE_OPENGL_RENDERER
		GL::bindVAO(0);
#endif
		// TODO: we should use VAO for vertex declaration.
		pDecl->EnableAttribute();
		pDecl->ApplyAttribute();

		PE_CHECK_GL_ERROR_DEBUG();
	}
	s_currentVertexDeclaration = pDecl;
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::CreateVertexDeclaration(VertexElement* pVertexElements, VertexDeclarationPtr* ppDecl)
{
	VertexDeclarationPtr pDecl = new CVertexDeclaration(pVertexElements);
	*ppDecl = pDecl;
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::BeginScene()
{
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::EndScene()
{
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::Clear(DWORD Count, const void* pRects, DWORD Flags, DWORD Color, float Z, DWORD Stencil)
{
	DWORD fields = 0;
	if ((Flags & D3DCLEAR_ZBUFFER) != 0)
	{
		fields |= GL_DEPTH_BUFFER_BIT;
		glClearDepth(Z);
		SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	}
	if ((Flags & D3DCLEAR_TARGET) != 0)
	{
		LinearColor color(Color);
		glClearColor(color.r, color.g, color.b, color.a);
		fields |= GL_COLOR_BUFFER_BIT;
	}
	if ((Flags & D3DCLEAR_STENCIL) != 0)
	{
		glClearStencil(Stencil);
		fields |= GL_STENCIL_BUFFER_BIT;
	}
	PE_CHECK_GL_ERROR_DEBUG();

	if (IsUsingRenderTarget())
	{
		if ((Flags & D3DCLEAR_ZBUFFER) != 0)
			glDepthMask(GL_TRUE);
		glClear(fields);
	}
	else
	{

#ifndef PARAENGINE_MOBILE
		glClear(fields);
#else
		// this is done by cocos at the beginning of each tick, we just set default values.
		// glClear(fields);
		if (Flags == D3DCLEAR_STENCIL)
			glClear(GL_STENCIL_BUFFER_BIT);
#endif

	}
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::SetFVF(DWORD FVF)
{
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::SetTransform(D3DTRANSFORMSTATETYPE State, DeviceMatrix_ptr pMatrix)
{
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::RendererRecreated()
{
	s_currentVertexDeclaration = 0;
	s_currentVertexBuffer = 0;
	s_currentIndexBuffer = 0;
	s_blendingSource = GL_SRC_ALPHA;
	s_blendingDest = GL_ONE_MINUS_SRC_ALPHA;
	s_currentCullingMode = D3DCULL_NONE;
	s_blendingAlphaSource = GL_SRC_ALPHA;
	s_blendingAlphaDest = GL_ONE_MINUS_SRC_ALPHA;
	s_bAlphaBlendingChanged = true;
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return S_OK;
}

uint32 ParaEngine::RenderDevice::GetStencilBits()
{
	static bool once = true;
	static GLint g_sStencilBits = -1;
	if (once)
	{
		glGetIntegerv(GL_STENCIL_BITS, &g_sStencilBits);
		if (g_sStencilBits <= 0)
		{
			OUTPUT_LOG("warn: Stencil buffer is not enabled.\n");
		}
		once = false;
	}
	return g_sStencilBits;
}

bool ParaEngine::RenderDevice::CheckRenderError(const char* filename, const char* func, int nLine)
{
#ifdef ALLOW_RENDER_ERROR_CHECK
	GLenum __error = glGetError();
	if (__error)
	{
		ParaEngine::CLogger::GetSingleton().WriteFormated("OpenGL error 0x%04X in %s %s %d\n", __error, filename, func, nLine);
		return false;
	}
#endif
	return true;
}

void ParaEngine::RenderDevice::GetScreenSize(int &nScreenWidth, int &nScreenHeight)
{
	if (IsUsingRenderTarget())
	{
		nScreenWidth = s_render_target_width;
		nScreenHeight = s_render_target_height;
	}
	else
	{
		// back buffer size.
		CGlobals::GetApp()->GetWindowCreationSize(&nScreenWidth, &nScreenHeight);
	}
}

HRESULT ParaEngine::RenderDevice::SetViewport(const D3DVIEWPORT9* pViewport)
{
	s_currentViewport = *pViewport;
	int nScreenWidth, nScreenHeight;
	GetScreenSize(nScreenWidth, nScreenHeight);

	glViewport((GLint)(pViewport->X), (GLint)(nScreenHeight - pViewport->Y- pViewport->Height), (GLsizei)(pViewport->Width), (GLsizei)(pViewport->Height));
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::GetViewport(D3DVIEWPORT9* pViewport)
{
	*pViewport = s_currentViewport;
	return S_OK;
}

void ParaEngine::RenderDevice::SetCursorPosition(int X, int Y, DWORD Flags)
{

}

HRESULT ParaEngine::RenderDevice::SetScissorRect(RECT* pRect)
{
	if (!pRect || (pRect->left <= 0 && pRect->top <= 0 && pRect->right <= 0 && pRect->bottom <= 0))
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		int nScreenWidth, nScreenHeight;
		GetScreenSize(nScreenWidth, nScreenHeight);
		//int nViewportOffsetY = nScreenHeight - (s_currentViewport.Y + s_currentViewport.Height);
		//glScissor((GLint)(pRect->left + s_currentViewport.X), (GLint)(nViewportOffsetY+nScreenHeight - pRect->bottom), (GLsizei)(pRect->right - pRect->left), (GLsizei)(pRect->bottom - pRect->top));
		glScissor((GLint)(pRect->left), (GLint)(nScreenHeight - pRect->bottom), (GLsizei)(pRect->right - pRect->left), (GLsizei)(pRect->bottom - pRect->top));
	}
	return S_OK;
}

HRESULT ParaEngine::RenderDevice::GetScissorRect(RECT* pRect)
{
	*pRect = s_scissorRect;
	return S_OK;
}

void ParaEngine::RenderDevice::BeginRenderTarget(int nWidth, int nHeight)
{
	s_render_target_width = nWidth;
	s_render_target_height = nHeight;
	s_isBeginRenderTarget = true;
}

void ParaEngine::RenderDevice::EndRenderTarget()
{
	s_render_target_width = 0;
	s_render_target_height = 0;
	s_isBeginRenderTarget = false;
}

bool ParaEngine::RenderDevice::IsUsingRenderTarget()
{
	return s_isBeginRenderTarget;
}

HRESULT RenderDevice::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	switch (State)
	{
	case D3DRS_CULLMODE:
	{
		if (s_currentCullingMode != Value)
		{
			s_currentCullingMode = Value;
			if (Value == D3DCULL_CCW){
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CW);//glCullFace(GL_FRONT);
			}
			else if (Value == D3DCULL_CW){
				glEnable(GL_CULL_FACE);
				glFrontFace(GL_CCW);//glCullFace(GL_BACK);
			}
			else if (Value == D3DCULL_NONE){
				glDisable(GL_CULL_FACE);
			}
			PE_CHECK_GL_ERROR_DEBUG();
		}
		break;
	}
	case D3DRS_DEPTHBIAS:
	{
		break;
	}
	case D3DRS_ZENABLE:
	{
		// z test enable
		if (Value)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_ZFUNC:
	{
		// z test enable
		if (Value == D3DCMP_LESSEQUAL)
			glDepthFunc(GL_LEQUAL);
		else if (Value == D3DCMP_GREATER)
			glDepthFunc(GL_GREATER);
		else if (Value == D3DCMP_GREATEREQUAL)
			glDepthFunc(GL_GEQUAL);
		else if (Value == D3DCMP_LESS)
			glDepthFunc(GL_LESS);
		else if (Value == D3DCMP_NEVER)
			glDepthFunc(GL_NEVER);
		else if (Value == D3DCMP_NOTEQUAL)
			glDepthFunc(GL_NOTEQUAL);
		else if (Value == D3DCMP_ALWAYS)
			glDepthFunc(GL_ALWAYS);
		else
			glDepthFunc(GL_LEQUAL);

		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_ZWRITEENABLE:
	{
		// z write enable
		glDepthMask(Value ? GL_TRUE : GL_FALSE);
		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_SEPARATEALPHABLENDENABLE:
	{
		bool bEnabled = Value ? true : false;
		if (s_bEnableSeparateAlphaBlending != bEnabled)
		{
			s_bEnableSeparateAlphaBlending = bEnabled;
			s_bAlphaBlendingChanged = true;
		}
		break;
	}
	case D3DRS_ALPHABLENDENABLE:
	{
		bool bEnabled = Value ? true : false;
		if (s_bEnableBlending != bEnabled)
		{
			s_bEnableBlending = bEnabled;
			s_bAlphaBlendingChanged = true;
		}
		break;
	}
	case D3DRS_SRCBLENDALPHA:
	case D3DRS_DESTBLENDALPHA:
	case D3DRS_SRCBLEND:
	case D3DRS_DESTBLEND:
	{
		GLenum glValue = D3DBLEND_ONE;
		if (Value == D3DBLEND_ONE)
			glValue = GL_ONE;
		else if (Value == D3DBLEND_ZERO)
			glValue = GL_ZERO;
		else if (Value == D3DBLEND_SRCALPHA)
			glValue = GL_SRC_ALPHA;
		else if (Value == D3DBLEND_INVSRCALPHA)
			glValue = GL_ONE_MINUS_SRC_ALPHA;
		else if (Value == D3DBLEND_DESTALPHA)
			glValue = GL_DST_ALPHA;
		else if (Value == D3DBLEND_INVDESTALPHA)
			glValue = GL_ONE_MINUS_DST_ALPHA;

		if (State == D3DRS_SRCBLEND){
			if (s_blendingSource != glValue)
			{
				s_blendingSource = glValue;
				s_bAlphaBlendingChanged = true;
			}
		}
		else if (State == D3DRS_DESTBLEND){
			if (s_blendingDest != glValue){
				s_blendingDest = glValue;
				s_bAlphaBlendingChanged = true;
			}
		}
		else if (State == D3DRS_SRCBLENDALPHA){
			if (s_blendingAlphaSource != glValue){
				s_blendingAlphaSource = glValue;
				s_bAlphaBlendingChanged = true;
			}
		}
		else if (State == D3DRS_DESTBLENDALPHA){
			if (s_blendingAlphaDest != glValue){
				s_blendingAlphaDest = glValue;
				s_bAlphaBlendingChanged = true;
			}
		}
		// blending mode is delayed until the next draw call.
		// ApplyBlendingModeChange();
		break;
	}
	case D3DRS_STENCILENABLE:
	{
		if (Value)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_STENCILFUNC:
	{
		if (Value == D3DCMP_EQUAL)
		{
			// draw only where stencil's value is Value
			glStencilFunc(GL_EQUAL, s_stencil_ref_value, 0xFF);
		}
		else if (Value == D3DCMP_ALWAYS)
		{
			glStencilFunc(GL_ALWAYS, s_stencil_ref_value, 0xFF);
		}
		else if (Value == D3DCMP_NEVER)
		{
			glStencilFunc(GL_NEVER, s_stencil_ref_value, 0xFF);
		}
		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_STENCILWRITEMASK:
	{
		glStencilMask(Value);
		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_STENCILPASS:
	{
		if (Value == D3DSTENCILOP_REPLACE)
		{
			s_stencil_pass = GL_REPLACE;
			glStencilOp(s_stencil_pass, GL_KEEP, GL_KEEP);
		}
		PE_CHECK_GL_ERROR_DEBUG();
		break;
	}
	case D3DRS_STENCILREF:
	{
		s_stencil_ref_value = Value;
		break;
	}
	case D3DRS_SCISSORTESTENABLE:
	{
		if(Value)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
		break;
	}
	default:
		break;
	}

	return S_OK;
}

void ParaEngine::RenderDevice::ApplyBlendingModeChange()
{
	if (s_bAlphaBlendingChanged)
	{
		s_bAlphaBlendingChanged = false;
		if (s_bEnableBlending)
		{
			if (s_bEnableSeparateAlphaBlending)
			{
				glEnable(GL_BLEND);
#ifdef USE_OPENGL_RENDERER
				glBlendFuncSeparate(s_blendingSource, s_blendingDest, s_blendingAlphaSource, s_blendingAlphaDest);
#endif
			}
			else
			{
				if (s_blendingSource == GL_ONE && s_blendingDest == GL_ZERO)
				{
					glDisable(GL_BLEND);
				}
				else
				{
					glEnable(GL_BLEND);
					glBlendFunc(s_blendingSource, s_blendingDest);
				}
			}
		}
		else
		{
			glDisable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ZERO);
		}
		PE_CHECK_GL_ERROR_DEBUG();
	}
}

bool ParaEngine::RenderDevice::ReadPixels(int nLeft, int nTop, int nWidth, int nHeight, void* pDataOut, DWORD nDataFormat, DWORD nDataType)
{
	// needs to invert Y, since opengl use upward Y.
	ParaViewport viewport;
	CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
	glReadPixels(nLeft, viewport.Height-nTop, nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)(pDataOut));
	return true;
}
#endif
