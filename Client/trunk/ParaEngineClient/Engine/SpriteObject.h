#pragma once
#include "TileObject.h"

namespace ParaEngine
{
struct D3DXSpriteEntity;

/**
* sprite object rendering style: bit fields.
* here is some example:
* animated floor/seafloor:		SPRITE_HORIZONTAL_PLANE
* Trees/MagicEffect/Billboard: SPRITE_Y_FACINGCAMERA
* Particles/Magic effect:		SPRITE_FACINGCAMERA
* Object Lables/text:			SPRITE_3D_SCREEN
* Game Interface:				SPRITE_2D
*/
/// The following are mutually exclusive ones
/// @def No camera based transform is applied. It's draw like any static plane in the 3d world
#define SPRITE_HORIZONTAL_PLANE		0	
/// @def Object only rotate round the Y axis to stay facing the camera
#define SPRITE_Y_FACINGCAMERA       1	
/// @def A matric m_mxWorldMatrix is used for object's position, facing and size, which is all in 3d world
#define SPRITE_FREETRANSFORM        2   
/// @def facing camera, but still in the 3d world space.
#define SPRITE_FACINGCAMERA         3	
/// @def object is drawn at the 3D position, but its size (m_fWidth, m_fHeight) is in 2d screen.
#define SPRITE_3D_SCREEN            4	
/// @def object's position and size are all in 2d screen.
#define SPRITE_2D                   5   
/// @def mark the END ofmutually exclusive ones
#define SPRITE_END1                 7   

/// The following are combinable attribute
/// @def Draw transparent texture
#define SPRITE_RENDER_ALPHA         8    
/// @def Draw transparent texture
#define SPRITE_RENDER_ZBUFFER      16    
/// @def Draw transparent texture
#define SPRITE_RENDER_BATCH        32    


struct TextureEntity;
/** OBSOLETED
* Sprite object can either be used in the 3D space or 2D screen ordinates.
* in 3D mode, it is the same as billboard with animated textures. I have optimized
* texture and vertex buffer loading by duplicating each frame in the sprite's vertex
* buffer. Please look at the CParaWorldAsset class for more information about SpriteBuffer
* Asset. 
* 
* TODO: Batch all sprite object together. It could improve performance. Since
* render state change can be avoided for multiple calls to sprite draw method. 
* also many Sprite object may use the same vertex and texture assets, which may
* further increase the performance with batch rendering. Batching could be done 
* by saving task to the sceneState object. Please see sceneState at CBaseObject
* for more information.
* A particle system will be designed later using Sprite objects.
*/
class CSpriteObject : public CTileObject
{
public:
	virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::SpriteObject;};
						
	/// size of the object in the world coordinates
	FLOAT				  m_fWidth, m_fHeight;			
	/// used in free transform style only to specify world tranform,it can be set by the user
	Matrix4			  m_mxWorldMatrix;				
	/// Sprite
	D3DXSpriteEntity*     m_pSprite;					

	//-- info in the animation texture
	/// texture
	TextureEntity*        m_ppTexture;					
	/// rect of the first image in the texture
	RECT m_rcFirstImageRect;							
	/// how many frames are there. 
    UINT m_num_frames;									

	//-- rendering attributes
	/// see above macro definitions
	DWORD m_sprite_style;								
	BOOL CheckSpriteStyleField(DWORD dwType){ 
		if(dwType<=SPRITE_END1)
			return (m_sprite_style&SPRITE_END1) == dwType;
		else
			return ((m_sprite_style & dwType)>0);};
	/// The ref transparent key
	Color m_colorkey;								

	//-- frames to play in the texture
	/// m_nStartFrame+m_nCurrentFrameOffset => current frame 
	UINT	m_nCurrentFrameOffset;		
	/// animation is looped between m_nStartFrame and m_nEndFrame
	UINT	m_nStartFrame;				
	UINT	m_nEndFrame;
	/// whether to loop the animation
	bool	m_bLoop;					
	/// whether to play animation
	bool	m_bAnimate;					
	/// the time to change to next frame
	double	m_dTimeInterval;			

protected:
	double	m_dTimeSinceLastUpdate;
	
	/// always loop between m_nStartFrame and m_nEndFrame(inclusive).
	void AdvanceFrame(double fTimeDelta)
	{
		if(m_bAnimate)
		{
			m_dTimeSinceLastUpdate+=fTimeDelta;
			if(m_dTimeSinceLastUpdate > m_dTimeInterval*m_num_frames)
			{
				m_nCurrentFrameOffset = m_nStartFrame;
				m_dTimeSinceLastUpdate = 0;
			}
			else if(m_dTimeSinceLastUpdate > m_dTimeInterval)
			{
				m_nCurrentFrameOffset++;
				m_dTimeSinceLastUpdate = 0;
			}

			if((m_nStartFrame+m_nCurrentFrameOffset) > m_nEndFrame)
			{
				if(m_bLoop)
					m_nCurrentFrameOffset = m_nStartFrame;
				else
					m_nCurrentFrameOffset = m_nEndFrame;
			}
		}
	};
public:
	/// -- for rendering
	virtual HRESULT Draw( SceneState * sceneState);
	/// used as KEY for batch rendering
	virtual AssetEntity* GetPrimaryAsset(){return (AssetEntity*)m_ppTexture;};				
	/// -- for init and delete
	HRESULT InitObject(D3DXSpriteEntity* pSprite,TextureEntity* ppTexture, Vector3 vPos, 
		FLOAT fWidth=1.0f, FLOAT fHeight=1.0f, DWORD sprite_style=0);
	void SetRenderState(LPDIRECT3DDEVICE9 pd3dDevice);
	void RestoreRenderState(LPDIRECT3DDEVICE9 pd3dDevice);

public:
	CSpriteObject(void);
	virtual ~CSpriteObject(void);
};
}