//-----------------------------------------------------------------------------
// Class:	CSpriteObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaDev team
// Date:	2004.3.17
// Revised: 2004.3.17
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneState.h"
#include "SpriteObject.h"

using namespace ParaEngine;

CSpriteObject::CSpriteObject(void):
		m_fWidth(0), 
		m_fHeight(0),
		m_dTimeInterval(0.1f),
		m_pSprite(NULL),
		m_ppTexture(NULL),
		m_num_frames(0),
		m_colorkey(COLOR_XRGB(0, 0, 0)),
		m_nCurrentFrameOffset(0),
		m_nStartFrame(0),
		m_nEndFrame(0),
		m_dTimeSinceLastUpdate(0),
		m_bLoop(true),
		m_bAnimate(true)
{
	m_rcFirstImageRect.left = 0;
	m_rcFirstImageRect.top = 0;
	m_rcFirstImageRect.right = 0;
	m_rcFirstImageRect.bottom = 0;
	SetPrimaryTechniqueHandle(TECH_SPRITE);

}

CSpriteObject::~CSpriteObject(void)
{
}

//---------------------------------------------------------------------------
// name draw
/// desc: render the scene. it will automatically animate itself. 
/// Note: render state is kind of chaos: Use State manager in future versions
//---------------------------------------------------------------------------
HRESULT CSpriteObject::Draw( SceneState * sceneState)
{
	if(! sceneState->bIsBatchRender )
	{
		// add to batch list
		sceneState->listPRSprite.push_back(this);
		return S_OK;
	}
	auto pd3dDevice = sceneState->m_pd3dDevice;
	
	//-- set world transformation matrix
	Matrix4 mx;
	Matrix4 temp;
	if(CheckSpriteStyleField(SPRITE_HORIZONTAL_PLANE))
	{
		ParaMatrixRotationX(&mx, MATH_PI/2);	// no camera facing
		//--  scale object: the object in template is always of unit size with center at 0,0,0
		ParaMatrixScaling(&temp, m_fWidth, m_fHeight, 1.0f);
		ParaMatrixMultiply(&mx, &temp, &mx);

		//-- AdvanceFrame if any
		AdvanceFrame(sceneState->dTimeDelta);

		Vector3 vPos = GetPosition();
		mx._41 = vPos.x;
		mx._42 = vPos.y;
		mx._43 = vPos.z;
	}
	else if(CheckSpriteStyleField(SPRITE_FREETRANSFORM))
	{
		mx = m_mxWorldMatrix;
	}
	else //if(CheckSpriteStyleField(SPRITE_Y_FACINGCAMERA))
	{
		mx = sceneState->BillBoardInfo().matBillboardMatrix;	// billboard camera facing:Y axis
		//--  scale object: the object in template is always of unit size with center at 0,0,0
		ParaMatrixScaling(&temp, m_fWidth, m_fHeight, 1.0f);
		ParaMatrixMultiply(&mx, &temp, &mx);

		//-- AdvanceFrame if any
		AdvanceFrame(sceneState->dTimeDelta);

		Vector3 vPos = GetPosition();
		mx._41 = vPos.x;
		mx._42 = vPos.y;
		mx._43 = vPos.z;
	}
	
    pd3dDevice->SetTransform( D3DTS_WORLD, mx.GetConstPointer() );

	
	//-- Render the billboard
	pd3dDevice->DrawPrimitive(RenderDeviceBase::DRAW_PERF_TRIANGLES_MESH,  D3DPT_TRIANGLESTRIP, (m_nStartFrame+m_nCurrentFrameOffset)*4, 2 );
	
	return S_OK;
}

void ParaEngine::CSpriteObject::SetRenderState(IRenderDevice* pd3dDevice)
{
	//TODO: render state is kind of chaos: Use sceneState::StateManager in future versions
	CGlobals::GetEffectManager()->SetCullingMode(false);
	pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	
	//if(! CheckSpriteStyleField(SPRITE_RENDER_ZBUFFER))
	//	pd3dDevice->SetRenderState( D3DRS_ZENABLE,      FALSE );

	if(CheckSpriteStyleField(SPRITE_RENDER_ALPHA))
	{
		//-- enable alpha and reference color for billboard
		pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
		pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		//pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		//pd3dDevice->SetRenderState( D3DRS_ALPHAREF,        0x08 );
		//pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	}
	else
	{
		pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
	}
}

void ParaEngine::CSpriteObject::RestoreRenderState(IRenderDevice* pd3dDevice)
{
	//-- restore state
	if(CheckSpriteStyleField(SPRITE_RENDER_ALPHA))
	{
		// only for billboard, so we turn it off
		pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
		//pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,    FALSE );
	}
	//if(! CheckSpriteStyleField(SPRITE_RENDER_ZBUFFER))
	//	pd3dDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
	CGlobals::GetEffectManager()->SetCullingMode(true);
}

/**
sprite_style: is set using the following
// -- The following are mutuallly exclusive ones----//
#define SPRITE_HORIZONTAL_PLANE		0	// No camera based transform is applied. It's draw like any static plane in the 3d world
#define SPRITE_Y_FACINGCAMERA       1	// Object only rotate round the Y axis to stay facing the camera
#define SPRITE_FREETRANSFORM        2   // A matric m_mxWorldMatrix is used for object's position, facing and size, which is all in 3d world
#define SPRITE_FACINGCAMERA         3	// facing camera, but still in the 3d world space.
#define SPRITE_3D_SCREEN            4	// object is drawn at the 3D position, but its size (m_fWidth, m_fHeight) is in 2d screen.
#define SPRITE_2D                   5   // object's position and size are all in 2d screen.
#define SPRITE_END1                 7   // mark the END of mutuallly exclusive ones
// here is some example:---//
// animated floor/seafloor:		SPRITE_HORIZONTAL_PLANE
// Trees/MagicEffect/Billboard: SPRITE_Y_FACINGCAMERA
// Particles/Magic effect:		SPRITE_FACINGCAMERA
// Object Lables/text:			SPRITE_3D_SCREEN
// Game Interface:				SPRITE_2D

// -- The following are combinable attribute--------//
#define SPRITE_RENDER_ALPHA         8    // Draw transparent texture
#define SPRITE_RENDER_ZBUFFER      16    // Enable z_buffer
#define SPRITE_RENDER_BATCH        32    // Enable post rendering, let a batch of sprite to be rendered together.

*/
HRESULT CSpriteObject::InitObject(D3DXSpriteEntity* pSprite,TextureEntity* ppTexture, Vector3 vPos, FLOAT fWidth, FLOAT fHeight, DWORD sprite_style)
{
	m_pSprite = pSprite;
	m_ppTexture = ppTexture;
	m_vPos = vPos;
	m_fWidth = fWidth;
	m_fHeight = fHeight;

	m_sprite_style = sprite_style;
	return S_OK;
}