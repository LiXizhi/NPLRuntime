#pragma once
#include "BaseObject.h"
#include "CanvasCamera.h"
#include "TextureEntity.h"

namespace ParaEngine
{
	/**
	*  we can put a mesh or character object on canvas. The canvas will render the object into a texture. 
	*/
	class C3DCanvas :public CBaseObject
	{
	public:
		C3DCanvas(void);
		virtual ~C3DCanvas(void);

	public:
		/** init device objects.
		* the create() function must be called, before this function is called.*/
		void InitDeviceObjects();
		/** restore device object*/
		void RestoreDeviceObjects();
		/** Invalid device object*/
		void InvalidateDeviceObjects();
		/** delete device objects */
		void DeleteDeviceObjects();
		/** clean up all resource objects */
		void Cleanup();


		/**
		* the canvas texture, which can be used as any other ordinary texture on 3D or 2D object.
		* @return 
		*/
		TextureEntity* GetTexture();

		/**
		* Get the canvas texture size
		* @param nWidth 
		* @param nHeight 
		*/
		void GetTextureSize(int& nWidth, int& nHeight){
			nWidth = m_nTextureWidth;
			nHeight = m_nTextureHeight;
		};
		/**
		* render the object to the canvas texture surface if it is enabled. 
		*/
		void Draw(float fDeltaTime);
		
		/**
		* save to file. 
		* @param sFileName a texture file path to save the file to. 
		*	we support ".dds", ".jpg", ".png" files. If the file extension is not recognized, ".png" file is used. 
		* @param nImageSize: if this is zero, the original size is used. If it is dds, all mip map levels are saved.
		*/
		void SaveToFile(const char* sFileName, int nImageSize = 0);

		/**
		* Zoom the camera
		* @param fAmount 
		*/
		void Zoom(float fAmount);
		/** zoom to show all objects */
		void ZoomAll();

		//rotate the 3d object, in radius
		/**
		* rotate the camera round the object on canvas
		* @param dx 
		* @param dy relative amount in radian. 
		* @param dz 
		*/
		void Rotate(float dx, float dy, float dz);

		/**
		* pan the camera
		* @param dx relative amount in pixels
		* @param dy relative amount in pixels
		*/
		void Pan(float dx, float dy);
		
		/**get the current actor */
		CBaseObject * GetActor();
		
		/**
		* set the canvas size in pixels
		* @param nWidth 
		* @param nHeight 
		*/
		void SetSize(int nWidth, int nHeight);
		
		/** a canvas can be bound to a certain selection group, so that it will always display the first object in the selection group. 
		* by default, it binds to group 0. 
		* @param nGroupID If this is negative, it does not bind to any selection group. One need to ensure that 
		* object is removed from the canvas before it is deleted.
		*/
		void SetBindingGroupID(int nGroupID){m_nBindedGroupID = nGroupID;};

		/**
		* @param bEnable 
		*/
		void SetEnabled(bool bEnable){ m_bEnabled = bEnable; };
		/**
		* 
		* @return 
		*/
		bool IsEnabled(){ return m_bEnabled; };

		/** this is an optional 2D mask, which is drawn over the entire canvas after scene is rendered in to it. 
		* @param pTexture 
		*/
		void SetMaskTexture(TextureEntity* pTexture);

	private:
		/**
		* set the actor 
		* @param pActor 
		*/
		void SetActor(CBaseObject * pActor);

		/// the actor on stage
		CBaseObject *	m_pActor;

		/** a canvas can be bound to a certain selection group, so that it will always display the first object in the selection group. 
		* by default, it binds to group 0. If it is negative, it does not bind to any selection group. One need to ensure that 
		* object is removed from the canvas before it is deleted. */
		int m_nBindedGroupID;

		/// actor position 
		DVector3 m_vActorPosition;

		/// actor facing. 
		float  m_fActorFacing;

		CCanvasCamera		m_camera;
		bool m_bAutoRotate;
		// whether device is created.
		bool m_bInitialized;

		// whether the canvas need to be updated.
		bool m_bNeedUpdate;
		int			m_nTextureWidth;
		int         m_nTextureHeight;
		bool m_bEnabled;

		LPDIRECT3DSURFACE9      m_pDepthStencilSurface;

		/** render target*/
		asset_ptr<TextureEntity>			m_canvasTexture;
		LPDIRECT3DSURFACE9      m_pCanvasSurface;

		/** this is an optional 2D mask, which is drawn over the entire canvas after scene is rendered in to it. */
		asset_ptr<TextureEntity> m_pMask;
	};

}
