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

		ATTRIBUTE_METHOD1(CViewport, SetAlignment_s, const char*)	{ cls->SetAlignment(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetLeft_s, int)	{ cls->SetLeft(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetTop_s, int)	{ cls->SetTop(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetWidth_s, int)	{ cls->SetWidth(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetHeight_s, int)	{ cls->SetHeight(p1); return S_OK; }
		ATTRIBUTE_METHOD(CViewport, ApplyViewport_s)	{ cls->ApplyViewport(); return S_OK; }
		
		ATTRIBUTE_METHOD1(CViewport, GetRenderScript_s, const char**)	{ *p1 = cls->GetRenderScript().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetRenderScript_s, const char*)	{ cls->SetRenderScript(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, GetRenderTargetName_s, const char**)	{ *p1 = cls->GetRenderTargetName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetRenderTargetName_s, const char*)	{ cls->SetRenderTargetName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CViewport, GetPipelineOrder_s, int*)	{ *p1 = cls->GetPipelineOrder(); return S_OK; }
		ATTRIBUTE_METHOD1(CViewport, SetPipelineOrder_s, int)	{ cls->SetPipelineOrder(p1); return S_OK; }
		
	public:

		struct ZOrderLessCompare
		{
			inline bool operator()(const CViewport* _Left, const CViewport* _Right) const
			{
				return (_Left ? _Left->GetZOrder() : 1000) < (_Right ? _Right->GetZOrder() : 1000);
			};
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

		CAutoCamera* GetCamera() ;
		void SetCamera(CAutoCamera* val) { m_pCamera = val; }

		CGUIRoot* GetGUIRoot() { return m_pGUIRoot; }
		void SetGUIRoot(CGUIRoot* val) { m_pGUIRoot = val; }

		/* script to call to render this viewport. default to "" */
		const std::string& GetRenderScript() const;
		void SetRenderScript(const std::string&& val);

		/** render target to use. if "", default to current back buffer*/
		const std::string&  GetRenderTargetName() const;
		void SetRenderTargetName(const std::string& val);

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

		/** get viewport transform in terms of scale and offset */
		void GetViewportTransform(Vector2*  pvScale, Vector2* pvOffset = NULL);
	protected:
		float GetStereoEyeSeparation();
	private:
		CSceneObject* m_pScene;
		CAutoCamera* m_pCamera;
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
		
		int m_nZOrder;
		std::string m_sName;
		std::string m_sRenderTargetName;
		CRenderTarget* m_pRenderTarget;

		STEREO_EYE m_nEyeMode;

		/* script to call to render this viewport. default to "" */
		std::string m_sRenderScript;

		/** -1 or RENDER_PIPELINE_ORDER. if -1, it will be rendered for all pipeline stage */
		int m_nPipelineOrder;
	};

}

