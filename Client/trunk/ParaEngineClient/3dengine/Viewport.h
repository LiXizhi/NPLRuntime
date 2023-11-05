#pragma once
#include "IAttributeFields.h"
#include "2dengine/GUIPosition.h"

namespace ParaEngine
{
	class CSceneObject;
	class CAutoCamera;
	class CGUIRoot;
	class CViewportManager;
	class CRenderTarget;

	enum STEREO_EYE{
		STEREO_EYE_NORMAL = 0,
		STEREO_EYE_LEFT,
		STEREO_EYE_RIGHT,
		STEREO_EYE_ODS,
	};

	/** a region of view port to render into. 
	*/
	class CViewport : public IAttributeFields
	{
	public:
		CViewport(CViewportManager* pViewportManager);
		virtual ~CViewport(void);

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_ViewPort; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CViewport"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char* sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CViewport, SetAlignment_s, const char*)	{ cls->SetAlignment(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CViewport, SetLeft_s, int)	{ cls->SetLeft(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, GetLeft_s, int*) { *p1 = cls->GetLeft(); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, SetTop_s, int)	{ cls->SetTop(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, GetTop_s, int*) { *p1 = cls->GetTop(); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, SetODSFov_s, float) { cls->m_stereoODSparam.fov = (p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, GetODSFov_s, float*) { *p1 = cls->m_stereoODSparam.fov; return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, SetWidth_s, int)	{ cls->SetWidth(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, GetWidth_s, int*) { *p1 = cls->GetWidth(); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, SetHeight_s, int)	{ cls->SetHeight(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, GetHeight_s, int*) { *p1 = cls->GetHeight(); return S_OK; }

		ATTRIBUTE_METHOD(CViewport, ApplyViewport_s)	{ cls->ApplyViewport(); return S_OK; }
		
		ATTRIBUTE_METHOD1(CViewport, GetRenderScript_s, const char**)	{ *p1 = cls->GetRenderScript().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetRenderScript_s, const char*)	{ cls->SetRenderScript(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, GetRenderTargetName_s, const char**)	{ *p1 = cls->GetRenderTargetName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetRenderTargetName_s, const char*)	{ cls->SetRenderTargetName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, GetPipelineOrder_s, int*)	{ *p1 = cls->GetPipelineOrder(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetPipelineOrder_s, int)	{ cls->SetPipelineOrder(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, IsUseSceneCamera_s, bool*) { *p1 = cls->IsUseSceneCamera(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetUseSceneCamera_s, bool) { cls->SetUseSceneCamera(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, IsEnabled_s, bool*) { *p1 = cls->IsEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetIsEnabled_s, bool) { cls->SetIsEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, GetZOrder_s, int*) { *p1 = cls->GetZOrder(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetZOrder_s, int) { cls->SetZOrder(p1); return S_OK; }
		
		ATTRIBUTE_METHOD1(CViewport, IsDeltaTimeDisabled_s, bool*) { *p1 = cls->IsDeltaTimeDisabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, DisableDeltaTime_s, bool) { cls->DisableDeltaTime(p1); return S_OK; }

	public:

		struct ZOrderLessCompare
		{
			inline bool operator()(const CViewport* _Left, const CViewport* _Right) const
			{
				return (_Left ? _Left->GetZOrder() : 1000) < (_Right ? _Right->GetZOrder() : 1000);
			};
		};

		struct StereoODSparam {
			float morePitch;

			bool isODS;
			float moreRotZ;//roll
			float moreRotY;//yaw
			float moreRotX;//pitch
			float fov;
			float fov_h;
			float aspectRatio;
			float eyeShiftDistance;
			bool needRecoverCamera;
			DVector3 oldEyePos;
			DVector3 oldLookAtPos;
			Vector3 oldRightDir;
			float oldFov;

			bool m_bOmniAlwaysUseUpFrontCamera;
			int m_nOmniForceLookatDistance;
			float oldPitch;
			float oldCameraDistance;

			int ods_group_idx;
			int ods_group_size;
			StereoODSparam(){
				isODS = false;
				moreRotZ = 0.0f;
				moreRotY = 0.0f;
				moreRotX = 0.0f;
				fov = MATH_PI / 4;
				aspectRatio = 1.0f;
				eyeShiftDistance = 0.0f;
				needRecoverCamera = false;
				m_bOmniAlwaysUseUpFrontCamera = true;
				m_nOmniForceLookatDistance = 20;
				ods_group_idx = -1;
				ods_group_size = 0;
				oldPitch = 0;
				oldCameraDistance = 8;
				oldRightDir = Vector3(0,0,1);
			}
			inline StereoODSparam& operator = (const StereoODSparam& target)
			{
				isODS = target.isODS;
				moreRotZ = target.moreRotZ;
				moreRotY = target.moreRotY;
				moreRotX = target.moreRotX;
				fov = target.fov;
				fov_h = target.fov_h;
				aspectRatio = target.aspectRatio;
				eyeShiftDistance = target.eyeShiftDistance;
				needRecoverCamera = target.needRecoverCamera;
				oldEyePos = target.oldEyePos;
				oldLookAtPos = target.oldLookAtPos;
				oldFov = target.oldFov;
				m_bOmniAlwaysUseUpFrontCamera = target.m_bOmniAlwaysUseUpFrontCamera;
				m_nOmniForceLookatDistance = target.m_nOmniForceLookatDistance;
				ods_group_idx = target.ods_group_idx;
				ods_group_size = target.ods_group_size;
				oldPitch = target.oldPitch;
				oldCameraDistance = target.oldCameraDistance;
				oldRightDir = target.oldRightDir;
				return *this;
			}
		};

		/** build the render list, and render the entire scene.
		* @param dTimeDelta: fAnimation delta time in seconds.
		* @param nPipelineOrder: the current pipeline order, default to PIPELINE_3D_SCENE, which is anything before UI.
		* specify over PIPELINE_POST_UI_3D_SCENE for anything after UI is drawn.
		*/
		HRESULT Render(double dTimeDelta, int nPipelineOrder);

		void ApplyCamera(CAutoCamera* pCamera);

		/** called when back buffer size changed. */
		void OnParentSizeChanged(int nWidth, int nHeight);

		/** make this viewport the current active one. */
		void SetActive();

	public:
		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		CSceneObject* GetScene() { return m_pScene; }
		void SetScene(CSceneObject* val) { m_pScene = val; }

		CAutoCamera* GetCamera();
		void SetCamera(CAutoCamera* val);

		/** whether to share the global scene camera */
		bool IsUseSceneCamera();
		/** set whether to share the global scene camera, if false, we will create our custom camera */
		void SetUseSceneCamera(bool bUseSceneCamera);

		CGUIRoot* GetGUIRoot() { return m_pGUIRoot; }
		void SetGUIRoot(CGUIRoot* val) { m_pGUIRoot = val; }

		/* script to call to render this viewport. default to "" */
		const std::string& GetRenderScript() const;
		void SetRenderScript(const std::string&& val);

		/** render target to use. if "", default to current back buffer*/
		const std::string&  GetRenderTargetName() const;
		void SetRenderTargetName(const std::string& val);

		shared_ptr<CRenderTarget> GetRenderTarget();
		void SetRenderTarget(shared_ptr<CRenderTarget> target);

		/** -1 or RENDER_PIPELINE_ORDER. if -1, it will be rendered for all pipeline stage */
		int GetPipelineOrder() const;
		void SetPipelineOrder(int val);

		/** reposition the control using the same parameter definition used when control is created.
		* see InitObject() for parameter definition. */
		void SetPosition(const char* alignment, int left, int top, int width, int height);

		void SetModified();

		/** @param x, y: [in|out] a position on back buffer. If it is screen position, it should be multiplied by UI scaling.
		* @param pWidth, pHeight: the view port's size is returned.
		*/
		void GetPointOnViewport(int& x, int& y, int* pWidth, int* pHeight);
		bool IsPointOnViewport(int x, int y);

		int GetWidth();
		int GetHeight();
		float GetAspectRatio();

		void SetAlignment(const char* alignment);
		void SetLeft(int left);
		int GetLeft();
		void SetTop(int top);
		int GetTop();
		void SetWidth(int width);
		void SetHeight(int height);
		ParaEngine::STEREO_EYE GetEyeMode() const;
		void SetEyeMode(ParaEngine::STEREO_EYE val);

		/** Camera yaw angle increment when recording Stereo video.*/
		void SetStereoODSparam(StereoODSparam& param);
		StereoODSparam& GetStereoODSparam();

		/** return last viewport */
		ParaViewport ApplyViewport();

		ParaViewport SetViewport(DWORD x, DWORD y, DWORD width, DWORD height);

		void UpdateRect();

		/** get the viewport as used for a given render target of given size. such as shadow map. */
		ParaViewport GetTextureViewport(float fTexWidth, float fTexHeight);

		int GetZOrder() const;
		void SetZOrder(int val);

		bool IsEnabled() const;
		void SetIsEnabled(bool val);

		/** draw post processing quad for this viewport's area. */
		bool DrawQuad();

		/** draw post processing quad for this viewport's area. another approach added by devilwalk*/
		bool DrawQuad2();

		/** get viewport transform in terms of scale and offset */
		void GetViewportTransform(Vector2*  pvScale, Vector2* pvOffset = NULL);

		/** we will ignore animation frame move when this viewport is rendered. Default to false. This is used when rendering the same scene multiple times from different angles. 
		* Only the first viewport needs to frame move the internal animation. 
		*/
		bool IsDeltaTimeDisabled();

		/** we will ignore animation frame move when this viewport is rendered. */
		void DisableDeltaTime(bool bDisabled = true);

	protected:
		float GetStereoEyeSeparation();
	private:
		CSceneObject* m_pScene;
		ref_ptr<CAutoCamera> m_pCamera;
		CGUIRoot* m_pGUIRoot;
		
		CGUIPosition m_position;
		// absolute position
		RECT m_rect;
		CViewportManager* m_pViewportManager;
		float m_fScalingX;
		float m_fScalingY;
		float m_fAspectRatio;
		bool m_bIsModifed;
		bool m_bIsEnabled;
		bool m_bDisableDeltaTime;
		
		int m_nZOrder;
		std::string m_sName;
		std::string m_sRenderTargetName;
		std::shared_ptr<CRenderTarget> m_pRenderTarget;

		STEREO_EYE m_nEyeMode;

		/* script to call to render this viewport. default to "" */
		std::string m_sRenderScript;

		/** -1 or RENDER_PIPELINE_ORDER. if -1, it will be rendered for all pipeline stage */
		int m_nPipelineOrder;

		StereoODSparam m_stereoODSparam;
	};

}

