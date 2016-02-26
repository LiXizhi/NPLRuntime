//----------------------------------------------------------------------
// Class:	CGDIDC
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: rewritten by LiXizhi 2007.1.13
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "DirectXEngine.h"
#include "EffectManager.h"
#include "BaseObject.h"
#include "../GUIResource.h"
#include "GDIDC.h"
#include "GDIMisc.h"
#include "memdebug.h"

using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// CGDIDC
//////////////////////////////////////////////////////////////////////////
//static functions 

CGDIDC::CGDIDC()
	:m_transform(Matrix4::IDENTITY)
{
	m_pOldRenderSurface = NULL;
}
CGDIDC::CGDIDC(TextureEntity* pTexture)
: m_transform(Matrix4::IDENTITY)
{
	m_pTextureEntity = pTexture;
	m_pOldRenderSurface = NULL;
}

CGDIDC::~CGDIDC()
{
	Release();
}

TextureEntity* CGDIDC::GetRenderTarget()
{
	return m_pTextureEntity.get();
}

void CGDIDC::Release()
{
}

HRESULT CGDIDC::Begin()
{
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	
	// set render target
	m_pOldRenderSurface =  CGlobals::GetDirectXEngine().GetRenderTarget();
	if(m_pTextureEntity==0 || ((TextureEntityDirectX*)m_pTextureEntity.get())->GetTexture()==0)
		return E_FAIL;
	LPDIRECT3DSURFACE9 pRenderTarget=NULL;
	((TextureEntityDirectX*)m_pTextureEntity.get())->GetTexture()->GetSurfaceLevel(0, &pRenderTarget);
	if(FAILED(CGlobals::GetDirectXEngine().SetRenderTarget(0, pRenderTarget)))
	{
		m_pTextureEntity.reset();
		OUTPUT_LOG("error: failed setting render target in CGDIDC\r\n");
		return E_FAIL;
	}
	if(CGlobals::GetEffectManager()->BeginEffect(TECH_GUI, NULL) == false)
	{
		return E_FAIL;
	}
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU,  D3DTADDRESS_WRAP );
	pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV,  D3DTADDRESS_WRAP );
#endif
	return S_OK;
}

HRESULT CGDIDC::End()
{
	HRESULT hr = S_OK;
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	//pd3dDevice->SetTransform(D3DTS_TEXTURE0,CGlobals::GetIdentityMatrix());
	LPDIRECT3DSURFACE9 pRenderTarget =  CGlobals::GetDirectXEngine().GetRenderTarget();
	SAFE_RELEASE(pRenderTarget);

	// Restore the old render target
	hr = CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pOldRenderSurface);
	
	pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
#endif
	return hr;
}

HRESULT CGDIDC::Clear(const Color &color)
{
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	return pd3dDevice->Clear(0,NULL,D3DCLEAR_TARGET,color,1.0f,0);
#endif
}

void CGDIDC::SetTransform(const Matrix4 *matTransform)
{
	if (matTransform==NULL) {
		return;
	}
	m_transform=*matTransform;
}

void CGDIDC::GetTransform(Matrix4 *matTransform)
{
	if (matTransform==NULL) {
		return;
	}
	*matTransform=m_transform;
}

HRESULT CGDIDC::DrawTriangleStrip(TextureEntity *pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertices)
{
	if (pVertices==NULL) {
		return S_OK;
	}
	HRESULT hr = S_OK;
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();
	
	if (pTexture) {
		hr = pd3dDevice->SetTexture(0, ((TextureEntityDirectX*)pTexture)->GetTexture());
	}else
		hr=pd3dDevice->SetTexture(0,NULL);

	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=pd3dDevice->SetTransform(D3DTS_TEXTURE0,m_transform.GetPointer());
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=RenderDevice::DrawPrimitiveUP( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UI,D3DPT_TRIANGLESTRIP, nNumVertices-2,pVertices, sizeof(DXUT_SCREEN_VERTEX));
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
#endif
	return hr;
}
HRESULT CGDIDC::DrawTriangleStripPrim(LPDIRECT3DTEXTURE9 pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertices)
{
	if (pVertices==NULL) {
		return S_OK;
	}
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();

	HRESULT hr;
	hr=pd3dDevice->SetTexture(0,pTexture);

	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=RenderDevice::DrawPrimitiveUP( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UI,D3DPT_TRIANGLESTRIP, nNumVertices-2,pVertices, sizeof(DXUT_SCREEN_VERTEX));
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}

	return hr;
}

HRESULT CGDIDC::DrawTriangleList(TextureEntity *pTexture,UINT nNumVertices,CONST DXUT_SCREEN_VERTEX* pVertices)
{
	if (pVertices==NULL) {
		return S_OK;
	}
	LPDIRECT3DDEVICE9 pd3dDevice=CGlobals::GetRenderDevice();

	HRESULT hr = S_OK;
#ifdef USE_DIRECTX_RENDERER
	if (pTexture) {
		hr = pd3dDevice->SetTexture(0, ((TextureEntityDirectX*)pTexture)->GetTexture());
	}else
		hr=pd3dDevice->SetTexture(0,NULL);

	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
	hr=pd3dDevice->SetTransform(D3DTS_TEXTURE0,m_transform.GetPointer());
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}

	hr=RenderDevice::DrawPrimitiveUP( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_UI,D3DPT_TRIANGLELIST, nNumVertices/3,pVertices, sizeof(DXUT_SCREEN_VERTEX));
	if (FAILED(hr)) {
		OUTPUT_DEFAULT_ERROR(hr);
		return hr;
	}
#endif
	return hr;
}

void ParaEngine::CGDIDC::EnableWriteAlpha(bool bWriteAlpha)
{
	/*LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
	pd3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);*/
	
}
