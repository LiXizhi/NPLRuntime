#pragma once

namespace ParaEngine
{

	/**
	* a reflective plane in the scene
	* currently it only supports horizontal surface.Vertical surface and arbitrary surface may be supported in the future
	*/
	class CMirrorSurface : public IAttributeFields
	{
	public:
		CMirrorSurface();
		virtual ~CMirrorSurface();

		ATTRIBUTE_DEFINE_CLASS(CMirrorSurface);

	public:

		/**
		* set the world position of the mirror. It will automatically enable or disable the surface based 
		* on the relative position of the object and the current camera eye position in the scene state object. 
		* @param vPos currently only y component is used.
		* @param bUseCloset if true, multiple calls of this functions to an enabled surface will have the same result
		*	as the closest position to the current camera eye position. 
		*/
		void SetPosition(Vector3 vPos, bool bUseCloset=true);

		/** a plane object usually defines the normal of the plane. The world position of the mirror surface is
		* determined by the plane offset by the position. 
		* @param plane: default value is (0,1,0,0). */
		void SetPlane(const Plane& plane);
		
		/**
		* @return whether it is enabled.
		*/
		bool IsEnabled(){return m_bEnable;};

		/**
		* set enabled. 
		* @param bEnabled 
		*/
		void SetEnabled(bool bEnabled){m_bEnable = bEnabled;};

		/**
		* Get the reflection texture. This function will enable this object. see SetEnabled();
		* when the reflection map is built this object will be disabled. 
		* @return NULL if reflection texture is not ready.
		*/
		DeviceTexturePtr_type GetReflectionTexture();
	protected:
		/**
		* set up the rendering matrices for ocean rendering.
		* @param bPostPushMatrices:When matrices are set, push them to the global matrix stack.
		* @param bPrePopMatrices: Before setting matrices, first pop matrix on the global matrix stack.
		*/
		void SetMatrices(bool bPostPushMatrices=false, bool bPrePopMatrices=false);

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
		* render the scene to the reflection surface if it is enabled. 
		* after calling this function, this mirror object is always disabled. 
		* at least one scene object must call GetReflectionTexture() in order to 
		* update reflection map in the subsequent frames.
		* This function is called by the scene manager object.
		*/
		void RenderReflectionTexture();

	private:
		// whether the mirror surface is enabled. 
		bool m_bEnable;
		// whether device is created.
		bool m_bInitialized;
		// world position of the center point of the mirror, currently only y component is used.
		Vector3 m_vPos;

		// the plane surface of the mirror.
		Plane	m_reflectionPlane;
		float		m_reflectionMapOverdraw;
		int			m_reflectionTextureWidth;
		int         m_reflectionTextureHeight;

		LPDIRECT3DSURFACE9       m_pDepthStencilSurface;
		DeviceTexturePtr_type       m_pReflectionTexture;
		LPDIRECT3DSURFACE9       m_pReflectionSurface;

		Matrix4               m_reflectionProjectionMatrix;
		Matrix4               m_worldViewProjectionInverseTransposeMatrix;
		bool                     m_reflectViewMatrix;
		bool		m_bDisableFogInReflection;

		friend class CSceneObject;
		friend class COceanManager;
	};

}