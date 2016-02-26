//-----------------------------------------------------------------------------
// Class:	C3DCanvas
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.7.24
// desc	: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DirectXEngine.h"
#include "SceneObject.h"
#include "EffectManager.h"
#include "ShapeSphere.h"
#include "ParaWorldAsset.h"
#include "BipedObject.h"

#include "SelectionManager.h"
#include "3DCanvas.h"

/*
TODO: what if actor is deleted. Use selection manager may be a better choice. 
*/


/** @def default canvas map width in pixels */
#define DEFAULT_CANVAS_MAP_WIDTH	256
/** @def default canvas map height in pixels */
#define DEFAULT_CANVAS_MAP_HEIGHT	256

#ifndef CHECK_RETURN_CODE
#define CHECK_RETURN_CODE(text, hr) if(FAILED((hr))){OUTPUT_LOG(text);return;}
#endif

using namespace ParaEngine;

C3DCanvas::C3DCanvas(void)
	: m_vActorPosition(0, 0, 0)
{
	m_nTextureWidth = DEFAULT_CANVAS_MAP_WIDTH;
	m_nTextureHeight = DEFAULT_CANVAS_MAP_HEIGHT;
	m_bAutoRotate = false;
	m_bNeedUpdate = true;
	m_bEnabled = true;
	m_bInitialized = false;
	m_pActor = NULL;
	m_pDepthStencilSurface = NULL;
	m_pCanvasSurface = NULL;

	m_fActorFacing = 0.f;
	m_nBindedGroupID = 0;

#ifdef _DEBUG
	//m_pMask = CGlobals::GetAssetManager()->LoadTexture("", "Texture//add_bro.png", TextureEntity::StaticTexture);
#endif
	
	m_canvasTexture = new TextureEntity();
	m_canvasTexture->SurfaceType = TextureEntity::RenderTarget;
	m_canvasTexture->SetTextureInfo(TextureEntity::TextureInfo(m_nTextureWidth, m_nTextureHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
}

C3DCanvas::~C3DCanvas(void)
{
	Cleanup();
}

void C3DCanvas::Cleanup()
{
	InvalidateDeviceObjects();
	DeleteDeviceObjects();
}

TextureEntity* C3DCanvas::GetTexture()
{
	m_bNeedUpdate = true;
	return m_canvasTexture.get();
};

void C3DCanvas::InitDeviceObjects()
{
	m_canvasTexture->InitDeviceObjects();
}
void C3DCanvas::RestoreDeviceObjects()
{
	HRESULT hr;

	LPDIRECT3DDEVICE9 pD3dDevice = CGlobals::GetRenderDevice();

	m_bInitialized=true;
	
	int nWidth = m_nTextureWidth;
	int nHeight = m_nTextureHeight;
	/*hr = pD3dDevice->CreateTexture(nWidth, 	nHeight, 
		1, D3DUSAGE_RENDERTARGET, 
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pCanvasTexture, NULL);
	CHECK_RETURN_CODE("CreateTexture Canvas Texture", hr);  

	hr = m_pCanvasTexture->GetSurfaceLevel(0, &m_pCanvasSurface);
	CHECK_RETURN_CODE("GetSurfaceLevel Canvas Surface", hr);*/

	hr = m_canvasTexture->RestoreDeviceObjects();
	CHECK_RETURN_CODE("CreateTexture Canvas Texture", hr);  
	
	hr = m_canvasTexture->GetTexture()->GetSurfaceLevel(0, &m_pCanvasSurface);
	CHECK_RETURN_CODE("GetSurfaceLevel Canvas Surface", hr);

	hr = pD3dDevice->CreateDepthStencilSurface(nWidth, nHeight, D3DFMT_D16, 
		D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencilSurface, NULL);
	CHECK_RETURN_CODE("failed creating depth stencil buffer", hr);
}

void C3DCanvas::InvalidateDeviceObjects()
{
	m_bInitialized=false;
	SAFE_RELEASE(m_pDepthStencilSurface);
	SAFE_RELEASE(m_pCanvasSurface);

	m_canvasTexture->InvalidateDeviceObjects();
}

void C3DCanvas::DeleteDeviceObjects()
{
	m_canvasTexture->DeleteDeviceObjects();
	// Bug fixed: we should specify the texture size, otherwise the back buffer size is used. 
	m_canvasTexture->SetTextureInfo(TextureEntity::TextureInfo(m_nTextureWidth, m_nTextureHeight, TextureEntity::TextureInfo::FMT_A8R8G8B8, TextureEntity::TextureInfo::TYPE_UNKNOWN));
}

void C3DCanvas::Rotate(float dx, float dy, float dz)
{
	m_bNeedUpdate = true;
	m_camera.Rotate(dx,dy,dz);
	m_bAutoRotate = false;
}

void C3DCanvas::Zoom(float fAmount)
{
	m_bNeedUpdate = true;
	m_camera.Zoom(fAmount);
}

void C3DCanvas::ZoomAll()
{
	m_bNeedUpdate = true;
	CBaseObject* pObject = GetActor();
	if(pObject!=0)
	{
		m_bNeedUpdate = true;
		IViewClippingObject* pViewObject = pObject->GetViewClippingObject();
		float fRadius = pViewObject->GetRadius();
		m_camera.ZoomSphere(CShapeSphere(Vector3(0,fRadius/2,0), fRadius));
	}
}

void C3DCanvas::Pan(float dx, float dy)
{
	m_bNeedUpdate = true;
	Vector3 vEye = m_camera.GetEyePosition();
	Vector3 vLookAt = m_camera.GetLookAtPosition();

	/* scale dy from pixel to meters*/
	dy = fabs(vLookAt.y - vEye.y)/m_nTextureHeight*dy;
	m_camera.Pan(dx, dy);
}

CBaseObject * C3DCanvas::GetActor()
{
	SelectedItem item;
	if(m_nBindedGroupID>=0)
	{
		bool bSelected = CGlobals::GetSelectionManager()->GetObject(m_nBindedGroupID,0, &item);
		if(bSelected && item.Is3DObject())
		{
			if (m_pActor != item.GetObject())
			{
				m_bNeedUpdate = true;
				m_pActor = item.GetAs3DObject();
				
				if(m_pActor->IsBiped())
				{
					// for biped object. we will look at its physics height.
					float fHeight = ((CBipedObject*)m_pActor)->GetPhysicsHeight();
					float fRadius = ((CBipedObject*)m_pActor)->GetPhysicsRadius();
					fHeight *= 0.718f; // make it a little lower.
					
					m_camera.ZoomSphere(CShapeSphere(Vector3(0,fHeight,0), fRadius)); 
				}
				else
				{
					IViewClippingObject* pViewObject = m_pActor->GetViewClippingObject();
					float fRadius = pViewObject->GetRadius();
					// object, take a guess with 0.518f, we just get a little closer to the object.
					m_camera.ZoomSphere(CShapeSphere(Vector3(0,fRadius/2,0), fRadius*0.518f)); 
				}
				m_bAutoRotate = true;
			}
		}
		else
			m_pActor = NULL;
	}

	return m_pActor;
}

void C3DCanvas::SetActor(CBaseObject * pActor)
{
	m_bNeedUpdate = true;
	// This function is obsoleted.
}

void C3DCanvas::SetSize(int nWidth, int nHeight)
{
	if(m_nTextureWidth!=nWidth && m_nTextureHeight != nHeight)
	{
		m_bNeedUpdate = true;
		m_nTextureWidth = nWidth;
		m_nTextureHeight = nHeight;

		InvalidateDeviceObjects();
		DeleteDeviceObjects();
		InitDeviceObjects();
		RestoreDeviceObjects();
	}
}

void C3DCanvas::SaveToFile(const char* sFileName, int nImageSize)
{
	string sFile = sFileName;
	string sExt = CParaFile::GetFileExtension(sFileName);

	D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

	if(sExt == "dds")
	{
		FileFormat = D3DXIFF_DDS;
	}
	else if(sExt == "jpg")
	{
		FileFormat = D3DXIFF_JPG;
	}
	else // if(sExt == "png")
	{
		sFile = CParaFile::ChangeFileExtension(sFile, "png");
	}

	if(nImageSize<=0 || nImageSize>= m_nTextureWidth)
	{
		if( SUCCEEDED(D3DXSaveTextureToFile(sFile.c_str(), FileFormat, m_canvasTexture->GetTexture(), NULL )) )
		{
			OUTPUT_LOG("canvas portrait %d taken for %s", m_nTextureWidth, sFile.c_str());
		}
	}
	else
	{
		// if the size is somewhere in the middle.
		/*int nMipLevel = 0;
		int nWidth = m_nTextureWidth;
		while (nImageSize <nWidth)
		{
			nMipLevel++;
			nWidth = nWidth / 2;
		}*/
		LPDIRECT3DTEXTURE9 pTex = m_canvasTexture->GetTexture();
		if(pTex)
		{
			LPDIRECT3DSURFACE9 pSur = NULL;
			pTex->GetSurfaceLevel(0, &pSur);

			if(pSur)
			{
				LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
				D3DFORMAT colorFormat = D3DFMT_A8R8G8B8;
				LPDIRECT3DTEXTURE9 pTextureDest = NULL;
				LPDIRECT3DSURFACE9 pSurDest = NULL;
				if(FAILED(pd3dDevice->CreateTexture((int)nImageSize, (int)nImageSize, 1, D3DUSAGE_RENDERTARGET, colorFormat, D3DPOOL_DEFAULT, &pTextureDest , NULL)))
				{
					SAFE_RELEASE(pSur);
					return;
				}
				if(FAILED(pTextureDest->GetSurfaceLevel(0, &pSurDest)))
				{
					SAFE_RELEASE(pTextureDest);
					SAFE_RELEASE(pSur);
					return;
				}
				
				// Copy scene to lower resolution render target texture
				if( SUCCEEDED(pd3dDevice->StretchRect( pSur, NULL, pSurDest, NULL, D3DTEXF_LINEAR )) ) 
				{
					if(SUCCEEDED(D3DXSaveSurfaceToFile(sFile.c_str(), FileFormat, pSurDest, NULL,NULL )))
					{
						OUTPUT_LOG("canvas portrait %d taken for %s",nImageSize, sFile.c_str());
					}
				}
				SAFE_RELEASE(pSur);
				SAFE_RELEASE(pSurDest);
				SAFE_RELEASE(pTextureDest);
			}
		}

	}
}

void C3DCanvas::SetMaskTexture(TextureEntity* pTexture)
{
	m_pMask = pTexture;
}

void C3DCanvas::Draw(float fDeltaTime)
{
	CBaseObject* pActor = GetActor();
	if(!IsEnabled() || pActor==0)
		return;
	if(m_bNeedUpdate == false)
		return;

	if(!m_bInitialized)
	{
		InitDeviceObjects();
		RestoreDeviceObjects();
	}
	if(m_pCanvasSurface == 0 || m_pDepthStencilSurface==0)
		return;

	if(m_bAutoRotate)
	{
		m_camera.Rotate(0,0.3f*fDeltaTime, 0);
	}

	m_bNeedUpdate = false;
	
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	LPDIRECT3DSURFACE9 pOldRenderTarget =  CGlobals::GetDirectXEngine().GetRenderTarget();
	CGlobals::GetDirectXEngine().SetRenderTarget(0, m_pCanvasSurface);

	// save old render origin
	Vector3 vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

	// set depth surface
	LPDIRECT3DSURFACE9 pOldZBuffer = NULL;
	if(FAILED(pd3dDevice->GetDepthStencilSurface(&pOldZBuffer)))
		return;
	pd3dDevice->SetDepthStencilSurface( m_pDepthStencilSurface );

	// save old actor position
	pActor->PushParam();

	pActor->SetPosition(m_vActorPosition);
	pActor->SetFacing(m_fActorFacing);
	

	// animate the camera and push result to transformation stack
	m_camera.FrameMove(fDeltaTime);

	CGlobals::GetProjectionMatrixStack().push(*m_camera.GetProjMatrix());
	CGlobals::GetViewMatrixStack().push(*m_camera.GetViewMatrix());
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true,true, true);

	// clear 
	pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, COLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);

	// disable fog
	pEffectManager->EnableFog(false);

	
	if(pEffectManager->BeginEffect(pActor->GetPrimaryTechniqueHandle(), &(CGlobals::GetSceneState()->m_pCurrentEffect)))
	{
		/** draw the main stage object */
		pActor->Draw(CGlobals::GetSceneState());
		CGlobals::GetSceneState()->GetFaceGroups()->Clear();
	}
	

	if(m_pMask!=0 && pEffectManager->BeginEffect(TECH_GUI, &(CGlobals::GetSceneState()->m_pCurrentEffect)))
	{
		/** draw the mask square in front of the screen. */
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline
			float sx = (float)m_nTextureWidth,sy = (float)m_nTextureHeight;
			
			DXUT_SCREEN_VERTEX v[10];
			v[0].x = 0;  v[0].y = sy; v[0].tu = 0;  v[0].tv = 1.0f;
			v[1].x = 0;  v[1].y = 0;  v[1].tu = 0;  v[1].tv = 0;
			v[2].x = sx; v[2].y = sy; v[2].tu = 1.f; v[2].tv = 1.f;
			v[3].x = sx; v[3].y = 0;  v[3].tu = 1.f; v[3].tv = 0;

			DWORD dwColor = LinearColor(1.f,1.f,1.f,1.f);
			int i;
			for(i=0;i<4;i++)
			{
				v[i].color = dwColor;
				v[i].z = 0;
				v[i].h = 1.0f;
			}
			CGlobals::GetRenderDevice()->SetTexture(0, m_pMask->GetTexture());
			RenderDevice::DrawPrimitiveUP( CGlobals::GetRenderDevice(), RenderDevice::DRAW_PERF_TRIANGLES_UI, D3DPT_TRIANGLESTRIP, 2, v, sizeof(DXUT_SCREEN_VERTEX) );
		}
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Restore state for the normal pipeline

	// enable fog
	pEffectManager->EnableFog(CGlobals::GetScene()->IsFogEnabled());

	// restore old actor position
	pActor->PopParam();
	
	// restore transformations
	CGlobals::GetProjectionMatrixStack().pop();
	CGlobals::GetViewMatrixStack().pop();
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true,true, true);

	// restore old depth surface
	pd3dDevice->SetDepthStencilSurface( pOldZBuffer);
	SAFE_RELEASE(pOldZBuffer);
	// Restore the old render target: i.e. the backbuffer
	HRESULT hr = CGlobals::GetDirectXEngine().SetRenderTarget(0, pOldRenderTarget);
	assert(hr == D3D_OK);

	// restore render origin
	CGlobals::GetScene()->RegenerateRenderOrigin(vOldRenderOrigin);

#ifdef _DEBUG
	//SaveToFile("E:\\cavas_obj.png");
#endif

}