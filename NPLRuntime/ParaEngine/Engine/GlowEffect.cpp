//-----------------------------------------------------------------------------
// Class:	CGlowEffect
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.5.31
// Note: based on the paper Real-Time Glow By Greg James and John O¡¯Rorke on Gamasutra, May 26, 2004
// remark: pay attention to texel to pixel mapping. there is half texel offset issue with directX. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "DirectXEngine.h"
#include "EffectManager.h"
#include "effect_file.h"
#include "SceneObject.h"
#include "GlowEffect.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#include "TextureD3D9.h"
#endif

using namespace ParaEngine;

/**@def turn on to show the glow and final image intermittently every 30 frames */
//#define TEST_GLOW

CGlowEffect::CGlowEffect(void)
{
	m_pRTHalfSizeTexture = NULL;
	m_pRTGlowSourceTexture = NULL;
	m_pRTBlurHorizTexture = NULL;
	m_pRTBlurVertTexture = NULL;


	m_pBackBuffer = NULL;
	m_pZBuffer = NULL;
}

CGlowEffect::~CGlowEffect(void)
{

}

HRESULT CGlowEffect::InvalidateDeviceObjects()
{

	SAFE_RELEASE(m_pRTHalfSizeTexture);
	SAFE_RELEASE(m_pRTGlowSourceTexture);
	SAFE_RELEASE(m_pRTBlurHorizTexture);
	SAFE_RELEASE(m_pRTBlurVertTexture);

	//SAFE_RELEASE(m_pZBuffer);
	return S_OK;
}

HRESULT CGlowEffect::RestoreDeviceObjects()
{
	HRESULT hr=S_OK;

	auto pRenderDevice = CGlobals::GetRenderDevice();
	
	auto viewport = CGlobals::GetRenderDevice()->GetViewport( );
	m_glowtextureWidth = (float)viewport.Width/2;
	m_glowtextureHeight = (float)viewport.Height/2;

	EPixelFormat colorFormat = EPixelFormat::A8R8G8B8;
	m_pRTHalfSizeTexture = pRenderDevice->CreateTexture(viewport.Width, viewport.Height, colorFormat, ETextureUsage::RenderTarget);
	m_pRTGlowSourceTexture = pRenderDevice->CreateTexture(m_glowtextureWidth, m_glowtextureHeight, colorFormat, ETextureUsage::RenderTarget);
	m_pRTBlurHorizTexture = pRenderDevice->CreateTexture(m_glowtextureWidth, m_glowtextureHeight, colorFormat, ETextureUsage::RenderTarget);

	
	m_pRTBlurVertTexture = m_pRTGlowSourceTexture;
	m_pRTBlurVertTexture->AddRef();
	return hr;
}

HRESULT CGlowEffect::Render(float fGlowThreshold, float fGlowAmount, bool bUseAlpha)
{
//#ifdef _DEBUG
//	bUseAlpha = true;
//#endif
	
	HRESULT hr=S_OK;

	auto pRenderDevice = CGlobals::GetRenderDevice();

	// get current render target
	m_pBackBuffer = pRenderDevice->GetRenderTarget(0);
	m_pZBuffer = pRenderDevice->GetDepthStencil();
	if(!m_pBackBuffer || !m_pZBuffer)
		return E_FAIL;

	// Copy scene to lower resolution render target texture
	
	if(!m_pBackBuffer->StretchRect(m_pRTHalfSizeTexture, NULL, NULL, ETextureFilterType::Linear)) {
		OUTPUT_LOG("StretchRect() failed!\r\n");
		return hr;
	}

	EffectManager* pEffectManager =  CGlobals::GetEffectManager();
	pEffectManager->BeginEffect(TECH_FULL_SCREEN_GLOW);
	CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
	
	if(pEffectFile != 0 && pEffectFile->begin(true))
	{
		// full screen glow effect is completed in four steps.
		assert(pEffectFile->totalPasses() == 4);
		mesh_vertex_plain quadVertices[4] = {
			{Vector3(-1,-1,0), Vector2(0,1)},
			{ Vector3(1, -1, 0), Vector2(1, 1) },
			{ Vector3(-1, 1, 0), Vector2(0, 0) },
			{ Vector3(1, 1, 0), Vector2(1, 0) },
		};
		
		// offset the texture coordinate by half texel in order to match texel to pixel. 
		// This takes me hours to figure out. :-(
		float fhalfTexelWidth = 0.5f/m_glowtextureWidth;
		float fhalfTexelHeight = 0.5f/m_glowtextureHeight;

		for (int i=0;i<4;++i)
		{
			quadVertices[i].uv.x += fhalfTexelWidth;
			quadVertices[i].uv.y += fhalfTexelHeight;
		}

		// shader constants
		/*
#ifdef TEST_GLOW
		static int i=0;
		++i;
		if(i>30){
			fGlowAmount  = 1.0f;
			fGlowThreshold = 0.f;
			if(i>60)
				i=0;
		}
		else
		{
			fGlowAmount  = 0.f;
			fGlowThreshold = 1.0f;
		}
		
#endif*/
		/*
		pEffectFile->setParameter(CEffectFile::k_ConstVector0, (const float*)&Vector4(1/m_glowtextureWidth, 1/m_glowtextureHeight, 0.0f, 0.0f));
		LinearColor glowness = pEffectManager->GetGlowness();
		glowness.a = fGlowThreshold;
		glowness.r *= fGlowAmount;
		glowness.g *= fGlowAmount;
		glowness.b *= fGlowAmount;
		pEffectFile->setParameter(CEffectFile::k_ConstVector1, (const float*)&glowness);
		*/
	
		bUseAlpha = true;
		if(bUseAlpha)
		{
			//////////////////////////////////////////////////////////////////////////
			// make the glow source. Multiply texture alpha * RGB to get the glow sources
			CGlobals::GetRenderDevice()->SetRenderTarget( 0, m_pRTGlowSourceTexture);
			CGlobals::GetRenderDevice()->SetDepthStencil( NULL );

			if(pEffectFile->BeginPass(0))
			{
				pEffectFile->setTexture(0, m_pRTHalfSizeTexture);
				auto fx = pEffectFile->GetDeviceEffect();
				fx->SetFloat(fx->GetParameterByName("glowThreshold"),fGlowThreshold);
				
				pEffectFile->CommitChanges();

				HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

				pEffectFile->EndPass();
			}
		}
		
		
		//////////////////////////////////////////////////////////////////////////
		// Blur glow sources in the horizontal axis

		CGlobals::GetRenderDevice()->SetRenderTarget( 0, m_pRTBlurHorizTexture );
		CGlobals::GetRenderDevice()->SetDepthStencil( NULL );
		// no need to clear alpha channel?
		// pRenderDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00, 1.0f, 0 ); 

		if(pEffectFile->BeginPass(1))
		{
			
			SetBlurEffectParameters(1.0f / m_glowtextureWidth,0,fGlowAmount,pEffectFile);	
			pEffectFile->setTexture(0, m_pRTGlowSourceTexture);
			pEffectFile->CommitChanges();

			HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

			pEffectFile->EndPass();
		}

		// set texture 0 to NULL so same texture is never simultaneously a source and render target
		pEffectFile->setTexture( 0, (IParaEngine::ITexture*)NULL );

		//////////////////////////////////////////////////////////////////////////
		// Blur the horizontal blur in the vertical direction

		CGlobals::GetRenderDevice()->SetRenderTarget( 0, m_pRTBlurVertTexture );
		CGlobals::GetRenderDevice()->SetDepthStencil( NULL );
		// no need to clear alpha channel?
		// pRenderDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00, 1.0f, 0 ); 

		//fGlowAmount = 2;
		if(pEffectFile->BeginPass(1))
		{
			//pEffectFile->setTexture( 1, m_pRTBlurHorizTexture );
			SetBlurEffectParameters(0,1.0f / m_glowtextureHeight,fGlowAmount,pEffectFile);	
			pEffectFile->setTexture(0, m_pRTBlurHorizTexture );  //editor here!!!
			pEffectFile->CommitChanges();

			HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

			pEffectFile->EndPass();
		}

		// Set all textures to NULL to prevent RTT being bound as source and destination at the same time,
		pEffectFile->setTexture( 0,(IParaEngine::ITexture*)NULL );

		//////////////////////////////////////////////////////////////////////////
		// Add the final blur image to the back buffer

		CGlobals::GetRenderDevice()->SetRenderTarget( 0, m_pBackBuffer);
		CGlobals::GetRenderDevice()->SetDepthStencil( NULL );
		// no need to clear alpha channel?
		// pRenderDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00, 1.0f, 0 ); 

		for (int i=0;i<4;++i)
		{
			quadVertices[i].uv.x -= fhalfTexelWidth * 0.5f;
			quadVertices[i].uv.y -= fhalfTexelHeight * 0.5f;
		}

		if(pEffectFile->BeginPass(3))
		{
			pEffectFile->setTexture(0, m_pRTHalfSizeTexture );
			pEffectFile->setTexture(1,m_pRTBlurVertTexture);

			Vector4 glowParam = pEffectManager->GetGlowness();
			pEffectFile->SetRawValue("glowParams",(DeviceVector4*)&glowParam,0,sizeof(Vector4));

			pEffectFile->CommitChanges();

			HRESULT hr = CGlobals::GetRenderDevice()->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP,2,quadVertices,sizeof(mesh_vertex_plain));

			pEffectFile->EndPass();
			pEffectFile->setTexture( 0, (IParaEngine::ITexture*)NULL );
			pEffectFile->setTexture(1,	(IParaEngine::ITexture*)NULL);
		}

		// restore depth stencil buffer
		CGlobals::GetRenderDevice()->SetDepthStencil( m_pZBuffer );
		pEffectFile->end();
	}
	else
	{
		CGlobals::GetScene()->EnableFullScreenGlow(false);
		//Glow effect will not be load when graphics setting set to middle
		//so we just disable this warnning 
		//OUTPUT_LOG("can not set full screen glow effect error \r\n");
	}
	SAFE_RELEASE(m_pZBuffer);
	return hr;
}


void CGlowEffect::SetBlurEffectParameters(float dx,float dy,float blurAmount,CEffectFile* pEffect)
{
	const int sampleCount = 9;
	float sampleWeights[sampleCount];
	float sampleOffset[sampleCount * 2];

	sampleWeights[0] = ComputeGaussian(0,blurAmount);
	sampleOffset[0] = 0;
	sampleOffset[1] = 0;

	float totalWeight = sampleWeights[0];

	for (int i=0;i<sampleCount/2;i++)
	{
		float weight = ComputeGaussian( (float)(i+1),blurAmount);
		sampleWeights[i*2+1] = weight;
		sampleWeights[i*2+2] = weight;

		totalWeight += weight * 2;

		float offset = i * 2 + 1.5f;
		float offsetX = dx * offset;
		float offsetY = dy * offset;

		int index = (i * 2) * 2 + 2;
		sampleOffset[index] = offsetX;
		sampleOffset[index+1] = offsetY;
		sampleOffset[index+2] = -offsetX;
		sampleOffset[index+3] = -offsetY;
	}

	for(int i=0;i<sampleCount;i++)
	{
		sampleWeights[i] /= totalWeight;
	}


	//optimize code here!!!
	auto fx = pEffect->GetDeviceEffect();
	fx->SetFloatArray(fx->GetParameterByName("sampleWeight"),sampleWeights,sampleCount);
	fx->SetFloatArray(fx->GetParameterByName("sampleOffset"),sampleOffset,sampleCount * 2);
}

float CGlowEffect::ComputeGaussian(float n,float blurAmount)
{
	return  (1.0f / sqrtf(2 * 3.141592f * blurAmount)) * expf(-(n * n) / (2 * blurAmount * blurAmount));
}


void CGlowEffect::SaveGlowTexturesToFile(const string& filename)
{
	D3DXSaveTextureToFile(filename.c_str(),D3DXIFF_JPG, GetD3DTex(m_pRTGlowSourceTexture), NULL );
	D3DXSaveTextureToFile((filename+"halfsize.jpg").c_str(),D3DXIFF_JPG, GetD3DTex(m_pRTHalfSizeTexture), NULL );
	D3DXSaveTextureToFile((filename+"horiz.jpg").c_str(),D3DXIFF_JPG, GetD3DTex(m_pRTBlurHorizTexture), NULL );
}