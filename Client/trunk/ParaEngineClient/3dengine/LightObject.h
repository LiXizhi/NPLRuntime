#pragma once
#include "ParaXEntity.h"
#include "SphereObject.h"
#include <string>
namespace ParaEngine
{
	class CParaXModel;
	class CLightParam;

	/** this is an independent local light scene object. 
	* local lights are mostly contained in the mesh object. But an independent light object like this 
	* is useful, when we want to manipulate the light object at runtime. */
	class CLightObject : public CSphereObject
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::LightObject;};
		CLightObject(void);
		virtual ~CLightObject(void);
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields
		ATTRIBUTE_DEFINE_CLASS(CLightObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CLightObject);
		
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CLightObject, GetLightType_s, int*)	{*p1 = cls->GetLightType(); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetLightType_s, int)	{cls->SetLightType(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, GetRange_s, float*)	{*p1 = cls->GetRange(); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetRange_s, float)	{cls->SetRange(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, GetDiffuse_s, Vector3*)		{*p1 = *(Vector3*)(&cls->GetDiffuse()); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetDiffuse_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetDiffuse(c); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, GetAttenuation0_s, float*)	{*p1 = cls->GetAttenuation0(); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetAttenuation0_s, float)	{cls->SetAttenuation0(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, GetAttenuation1_s, float*)	{*p1 = cls->GetAttenuation1(); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetAttenuation1_s, float)	{cls->SetAttenuation1(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, GetAttenuation2_s, float*)	{*p1 = cls->GetAttenuation2(); return S_OK;}
		ATTRIBUTE_METHOD1(CLightObject, SetAttenuation2_s, float)	{cls->SetAttenuation2(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CLightObject, IsDeferredLightOnly_s, bool*)	{ *p1 = cls->IsDeferredLightOnly(); return S_OK; }
		ATTRIBUTE_METHOD1(CLightObject, SetDeferredLightOnly_s, bool)	{ cls->SetDeferredLightOnly(p1); return S_OK; }

	public:
		virtual std::string ToString(DWORD nMethod);

		/** Rotate the object.This only takes effects on objects having 3D orientation, such as
		* static mesh and physics mesh. The orientation is computed in the following way: first rotate around x axis,
		* then around y, finally z axis.
		* @param x: rotation around the x axis.
		* @param y: rotation around the y axis.
		* @param z: rotation around the z axis.
		*/
		virtual void Rotate(float x, float y, float z);

		/** set the scale of the object. This function takes effects on both character object and mesh object.
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);

		/** reset the object to its default settings.*/
		virtual void Reset();


		/**
		* Init from light struct.
		* @param pLight the initial light parameter, if NULL, a default white spot light will be created.
		* @param ppMesh: the mesh used to draw the object
		* @param vCenter: center world position.
		* @param mat: local transformation matrix
		* @param bCopyParams if true, the light object will make an internal copy of the light parameters.
		*	if not, it will only keep a reference to the light parameter. Only set this to false, when one
		*	need to manipulate it through the GUI.
		* @return S_OK if succeeds.
		*/
		HRESULT InitObject(CLightParam* pLight, MeshEntity* ppMesh, const Vector3& vCenter, const Matrix4& mat, bool bCopyParams = true);

		/** derived class can override this function to place the object in to the render pipeline.
		* if this function return -1, the SceneObject will automatically place the object into the render pipeline.
		* if return 0, it means the object has already placed the object and the scene object should skip this object.
		*/
		virtual int PrepareRender(CBaseCamera* pCamera, SceneState * sceneState);

		/** it only draws an arrow, when the scene's show local light parameter is true. */
		virtual HRESULT Draw(SceneState * sceneState);

		HRESULT RenderMesh(SceneState * sceneState);

		
		virtual void Cleanup();

		/** set local transform directly */
		void SetLocalTransform(const Matrix4& mXForm);
		/** set local transform by first uniform scale, then rotate around Z, X, Y axis sequentially. */
		void SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ);
		/** set local transform by first uniform scale, then rotate using a quaternion. */
		void SetLocalTransform(float fScale, const Quaternion& quat);
		/** get local transform*/
		void GetLocalTransform(Matrix4* localTransform);

		/** return the global light in render coordinate system. */
		CLightParam* GetLightParams();

		virtual AssetEntity* GetPrimaryAsset();
		virtual void SetAssetFileName(const std::string& sFilename);
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		/**
		* return the world matrix of the object for rendering
		* @param out: the output.
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: same as out. or NULL if not exists.
		*/
		virtual Matrix4* GetRenderMatrix(Matrix4& out, int nRenderNumber = 0);
		virtual void RenderDeferredLightMesh(SceneState * sceneState);
	public:
		/**
		* Set type
		* @param nType D3DLIGHTTYPE
		*  - D3DLIGHT_POINT          = 1,
		*  - D3DLIGHT_SPOT           = 2,
		*  - D3DLIGHT_DIRECTIONAL    = 3,
		*/
		void SetLightType(int nType);
		/**
		* see SetLightType()
		* @return 
		*/
		int GetLightType();

		/**
		* set diffuse color of the light
		* @param color 
		*/
		void SetDiffuse(const LinearColor& color);
		/** @see SetDiffuse()*/
		const LinearColor& GetDiffuse();
		
		/**
		* Cutoff range 
		* @param range 
		*/
		void SetRange(float range);
		
		/**
		* @see SetRange()
		* @return 
		*/
		float GetRange();

		/**
		* Constant attenuation 
		* @param range 
		*/
		void SetAttenuation0(float Attenuation0);

		/**
		* @see SetAttenuation0()
		* @return 
		*/
		float GetAttenuation0();


		/**
		* Linear attenuation 
		* @param range 
		*/
		void SetAttenuation1(float Attenuation1);

		/**
		* @see SetAttenuation1()
		* @return 
		*/
		float GetAttenuation1();

		/**
		* Quadratic attenuation 
		* @param range 
		*/
		void SetAttenuation2(float Attenuation2);

		/**
		* @see SetAttenuation2()
		* @return 
		*/
		float GetAttenuation2();

		/** whether the light is enabled for deferred render pipeline only. */
		bool IsDeferredLightOnly() const;
		void SetDeferredLightOnly(bool val);
	protected:
		/** current position in the map */
		//Vector3           m_vPos;
		/** local transform. usually a rotation with scaling.  */
		Matrix4            m_mxLocalTransform;

		AnimIndex m_CurrentAnim;

		/** mesh geometry */
		ref_ptr<ParaXEntity>      m_pAnimatedMesh;
		ref_ptr<MeshEntity>		  m_pDeferredShadingMesh;

		/** light parameters. */
		CLightParam* m_pLightParams;

		/** if true, the light object will delete the m_pLightParams object at destruction time. */
		bool m_bDeleteLightParams;

		/** whether the light is enabled for deferred render pipeline only. */
		bool m_bDeferredLightOnly;
	};
}
