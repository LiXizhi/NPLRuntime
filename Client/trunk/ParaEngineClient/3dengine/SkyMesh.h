#pragma once
#include "TileObject.h"

namespace ParaEngine
{
	struct AssetEntity;
	class CMeshObject;
	/**
	* the sky object in the scene. There are several types of sky.
	* The sky object will automatically adapt to the current fog parameters in the scene manager.
	* It is important that the sky object is the first object to be drawn in the 3D scene.
	*/
	class CSkyMesh : public CTileObject
	{

	public:
		CSkyMesh(void);
		virtual ~CSkyMesh(void);
		virtual CBaseObject::_SceneObjectType GetType(){ return CBaseObject::SkyMesh; };

		enum SkyType
		{
			/** the mesh is just a static mesh object. Suitable for FPS and cut scenes */
			Sky_StaticMesh,
			/** a simulated sky */
			Sky_Simulated
		};

	public:
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CSkyMesh; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CSkyMesh"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CSkyMesh);
		ATTRIBUTE_METHOD1(CSkyMesh, GetSkyColorFactor_s, Vector3*)		{ (*p1).x = cls->GetSkyColorFactor().r; (*p1).y = cls->GetSkyColorFactor().g; (*p1).z = cls->GetSkyColorFactor().b; return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSkyColorFactor_s, Vector3)		{ LinearColor c(p1.x, p1.y, p1.z, 1); cls->SetSkyColorFactor(c); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, GetSkyMeshFile_s, const char**)	{ *p1 = cls->GetSkyMeshFile(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSkyMeshFile_s, const char*)	{ cls->SetSkyMeshFile(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, GetSkyMeshTexture_s, const char**)	{ *p1 = cls->GetSkyMeshTexture(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSkyMeshTexture_s, const char*)	{ cls->SetSkyMeshTexture(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetFogBlendAngleFrom_s, float*)	{ *p1 = cls->GetFogBlendAngleFrom(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetFogBlendAngleFrom_s, float)	{ cls->SetFogBlendAngleFrom(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, GetFogBlendAngleTo_s, float*)	{ *p1 = cls->GetFogBlendAngleTo(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetFogBlendAngleTo_s, float)	{ cls->SetFogBlendAngleTo(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, IsSimulatedSkyEnabled_s, bool*)	{ *p1 = cls->IsSimulatedSkyEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, EnableSimulatedSky_s, bool)	{ cls->EnableSimulatedSky(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetIsAutoDayTime_s, bool*)	{ *p1 = cls->GetIsAutoDayTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetIsAutoDayTime_s, bool)	{ cls->SetIsAutoDayTime(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetSunGlowTexture_s, const char**)	{ *p1 = cls->GetSunGlowTexture(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSunGlowTexture_s, const char*)	{ cls->SetSunGlowTexture(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetCloudTexture_s, const char**)	{ *p1 = cls->GetCloudTexture(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetCloudTexture_s, const char*)	{ cls->SetCloudTexture(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetSunColor_s, Vector3*)		{ (*p1) = cls->GetSunColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSunColor_s, Vector3)		{ cls->SetSunColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetLightSkyColor_s, Vector3*)		{ (*p1) = cls->GetLightSkyColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetLightSkyColor_s, Vector3)		{ cls->SetLightSkyColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetDarkSkyColor_s, Vector3*)		{ (*p1) = cls->GetDarkSkyColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetDarkSkyColor_s, Vector3)		{ cls->SetDarkSkyColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetSunIntensity_s, Vector2*)		{ (*p1) = cls->GetSunIntensity(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSunIntensity_s, Vector2)		{ cls->SetSunIntensity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetCloudColor_s, Vector3*)		{ (*p1) = cls->GetCloudColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetCloudColor_s, Vector3)		{ cls->SetCloudColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetCloudVelocity_s, Vector2*)		{ (*p1) = cls->GetCloudVelocity(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetCloudVelocity_s, Vector2)		{ cls->SetCloudVelocity(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetCloudOffset_s, Vector2*)		{ (*p1) = cls->GetCloudOffset(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetCloudOffset_s, Vector2)		{ cls->SetCloudOffset(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetCloudThickness_s, float*)	{ *p1 = cls->GetCloudThickness(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetCloudThickness_s, float)	{ cls->SetCloudThickness(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetSunSize_s, Vector2*)		{ (*p1) = cls->GetSunSize(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetSunSize_s, Vector2)		{ cls->SetSunSize(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetMoonSize_s, Vector2*)		{ (*p1) = cls->GetMoonSize(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetMoonSize_s, Vector2)		{ cls->SetMoonSize(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CSkyMesh, GetFogColor_s, Vector3*)		{ (*p1) = cls->GetFogColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CSkyMesh, SetFogColor_s, Vector3)		{ cls->SetFogColor(p1); return S_OK; }

		/** change the sky mesh model */
		void SetSkyMeshFile(const char* filename);
		/** get the sky mesh model file name. */
		const char* GetSkyMeshFile();

		/** change the sky mesh model */
		void SetSkyMeshTexture(const char* filename, int nTexIndex = 2);
		/** get the sky mesh model file name. */
		const char* GetSkyMeshTexture(int nTexIndex = 2);

		/**
		* a color to be multiplied to the sky, giving it a special color hue
		* @return
		*/
		const LinearColor& GetSkyColorFactor(){ return m_skycolorFactor; };
		/**
		* a color to be multiplied to the sky, giving it a special color hue
		* @param color
		*/
		void SetSkyColorFactor(const LinearColor& color){ m_skycolorFactor = color; };

		///render the sky
		virtual HRESULT Draw(SceneState * sceneState);

		void DrawSimulatedSky(EffectManager* pEffectManager, SceneState * sceneState, CSunLight &sunlight, RenderDevicePtr pd3dDevice, Vector3 &vPos);

		void DrawStaticMeshSky(EffectManager* pEffectManager, SceneState * sceneState, RenderDevicePtr pd3dDevice, CSunLight &sunlight, Vector3 vPos);

		void DrawStaticMeshSkyInner(SceneState * sceneState);

		virtual HRESULT RendererRecreated();
		// -- set current sky mode and parameters
		void CreateSkyVertexColorSimple();
		/**
		* Create the sky mesh as a static mesh object. It may be a sky box/dome/plane or whatever.
		* The static mesh will be scaled by the specified amount along x,y,z axis and then
		* translate up or down along the y axis.
		* in many cases, the mesh data in the mesh asset is of unit size.
		* @param strObjectName: sky name
		* @param MeshAsset: it must be a static mesh entity.
		* @param fScaleX: the static mesh local transform scale along the x axis
		* @param fScaleY: the static mesh local transform scale along the y axis
		* @param fScaleZ: the static mesh local transform scale along the z axis
		* @param fHeightOffset: the translation along the y axis.
		*/
		void CreateSkyStaticMesh(const string& strObjectName, AssetEntity& MeshAsset, FLOAT fScaleX, FLOAT fScaleY, FLOAT fScaleZ, FLOAT fHeightOffset = 0);

		virtual DVector3 GetPosition(){ return m_vPos; };
		virtual void SetPosition(const DVector3 & v){ m_vPos = v; };

		/**
		* from which angle the sky fog begins, in most cases.  0<FogBlendAngleFrom<FogBlendAngleTo<PI/2
		* if FogBlendAngleTo>=FogBlendAngleFrom, no sky fog will be drawn
		* @param fValue
		*/
		void SetFogBlendAngleFrom(float fValue){ m_fFogBlendAngleFrom = fValue; };
		float GetFogBlendAngleFrom(){ return m_fFogBlendAngleFrom; };

		/**
		* to which angle the sky fog ends, in most cases.  0<FogBlendAngleFrom<FogBlendAngleTo<PI/2.
		* if FogBlendAngleTo>=FogBlendAngleFrom, no sky fog will be drawn
		* @param fValue
		*/
		void SetFogBlendAngleTo(float fValue){ m_fFogBlendAngleTo = fValue; };
		float GetFogBlendAngleTo(){ return m_fFogBlendAngleTo; };

		/** set whether we render the sky as simulated mesh */
		void EnableSimulatedSky(bool bEnable);
		/** get whether we render the sky as simulated mesh */
		bool IsSimulatedSkyEnabled(){ return (m_typeSky == Sky_Simulated); };

		/** init device objects.
		* the create() function must be called, before this function is called.*/
		virtual HRESULT InitDeviceObjects();
		/** delete device objects */
		virtual HRESULT DeleteDeviceObjects();

	private:
		/// sky type
		SkyType m_typeSky;

		/// current position in the world
		DVector3           m_vPos;

		// pointer to the static mesh
		ref_ptr<CMeshObject> m_pStaticMesh;

		/** Fog blend angle range, default value is [PI/20,PI/10].
		*/
		static float	m_fFogBlendAngleFrom;
		static float	m_fFogBlendAngleTo;
		/** a color to be multiplied to the sky, giving it a special color hue.*/
		LinearColor m_skycolorFactor;

		void SetSkyMesh(AssetEntity& MeshAsset, float fHeightOffset = 0);

		//////////////////////////////////////////////////////////////////////////
		//
		// for simulated sky
		//
		//////////////////////////////////////////////////////////////////////////
		bool m_bSimsky_BufferInited;

		bool m_bSimsky_AutoDayTime;
		/** whether the sim sky will automatically change the coloring according to the current day time. */
		bool GetIsAutoDayTime() { return m_bSimsky_AutoDayTime; }
		void SetIsAutoDayTime(bool bValue) { m_bSimsky_AutoDayTime = bValue; }
		ParaVertexBuffer m_simsky_vb;
		ParaIndexBuffer m_simsky_ib;

		int m_nSimsky_vertexCount;
		int m_nSimsky_indexCount;
		int m_nSimsky_primitiveCount;
		float m_fSimsky_verticalRadius;
		float m_fSimsky_horizonRadius;
		int m_nSimsky_verticalSampleCount;
		int m_nSimsky_horizonSampleCount;
		Vector3 m_vSimsky_sunDirection;
		Vector3 m_vSimsky_sunPosition;

		Vector3 m_vSimsky_position;

		/** water color texture */
		asset_ptr<TextureEntity> m_texSimsky_sunsetGlowTexture;
		const char* GetSunGlowTexture();
		void SetSunGlowTexture(const char* sValue);

		/** Modulate the reflection texture by a texture to fade out at the edges.
		This avoids an objectionable artifact when the reflection map doesn't cover enough area. */
		asset_ptr<TextureEntity> m_texSimsky_cloudTexture;
		const char* GetCloudTexture();
		void SetCloudTexture(const char* sValue);

		Vector3 m_vSimsky_sunColor;
		Vector3 GetSunColor() { return m_vSimsky_sunColor; }
		void SetSunColor(const Vector3 & vValue) { m_vSimsky_sunColor = vValue; }

		Vector3 m_vSimsky_lightSkyColor;

		Vector3 GetLightSkyColor() { return m_vSimsky_lightSkyColor; }
		void SetLightSkyColor(const Vector3 & vValue) { m_vSimsky_lightSkyColor = vValue; }

		Vector3 m_vSimsky_darkSkyColor;
		Vector3 GetDarkSkyColor() { return m_vSimsky_darkSkyColor; }
		void SetDarkSkyColor(const Vector3 & vValue) { m_vSimsky_darkSkyColor = vValue; }

		Vector2 m_vSimsky_sunIntensity;
		Vector2 GetSunIntensity() { return m_vSimsky_sunIntensity; }
		void SetSunIntensity(const Vector2 & vValue) { m_vSimsky_sunIntensity = vValue; }

		Vector3 m_vSimsky_cloudColor;
		Vector3 GetCloudColor() { return m_vSimsky_cloudColor; }
		void SetCloudColor(const Vector3 & vValue) { m_vSimsky_cloudColor = vValue; }

		Vector2 m_vSimsky_cloudDirection;
		Vector2 GetCloudDirection() { return m_vSimsky_cloudDirection; }
		void SetCloudDirection(const Vector2 & vValue) { m_vSimsky_cloudDirection = vValue; }

		Vector2 m_vSimsky_cloudVelocity;
		Vector2 GetCloudVelocity() { return m_vSimsky_cloudVelocity; }
		void SetCloudVelocity(const Vector2 & vValue) { m_vSimsky_cloudVelocity = vValue; }

		Vector2 m_vSimsky_cloudOffset;
		Vector2 GetCloudOffset() { return m_vSimsky_cloudOffset; }
		void SetCloudOffset(const Vector2 & vValue) { m_vSimsky_cloudOffset = vValue; }

		Vector2 m_vSimsky_sunSize;
		Vector2 GetSunSize() { return m_vSimsky_sunSize; }

		/** @param vSunSize:
			vSunSize.x: controls the sun size. default to 500.0. the smaller the bigger. typically between (100,2000)
			vSunSize.y: controls the sun halo size. more information see the shader.
			*/
		void SetSunSize(const Vector2 & vValue) { m_vSimsky_sunSize = vValue; }

		Vector3 m_vSimsky_fogColor;
		Vector3 GetFogColor() const { return m_vSimsky_fogColor; }
		void SetFogColor(Vector3 val) { m_vSimsky_fogColor = val; }

		Vector2 m_vSimsky_moonSize;
		Vector2 GetMoonSize() const { return m_vSimsky_moonSize; }
		void SetMoonSize(Vector2 val) { m_vSimsky_moonSize = val; }

		float m_fSimsky_cloudThickness;
		float GetCloudThickness() const { return m_fSimsky_cloudThickness; }
		void SetCloudThickness(float val) { m_fSimsky_cloudThickness = val; }

		/** call this function every frame to change the time of day. it will automatically change the day and night parameters */
		void UpdateSimSky(float fTime);
		bool m_bSimsky_isDaytime;

		/**
		* Create a simulated sky
		*/
		void CreateSkySimulated(int verticalSampleCount, int horizonSampleCount, float verticalRadius, float horizonRadius);
	};
}