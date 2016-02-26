#pragma once
#include <string>
#include <vector>
namespace ParaEngine
{
	struct SceneState;
	class CGUIText;
	class CBaseObject;
	using namespace std;
	/**
	* It is the data structure that holds description data about the head on display.
	*/
	struct HeadOn3DData
	{
	public:
		HeadOn3DData();

		///////////////////////
		//
		// UI text info that is displayed on head of character. 
		//
		///////////////////////

		/** text to be displayed */
		string m_sText;

		/** which UI control the head on display will be used as a template for drawing the text
		* it can be a single CGUIText Object or it can be a container with a direct children called "text"
		* if this is "" or empty, the default UI template will be used. The default UI template is an invisible CGUIText control called "_HeadOnDisplayText_"
		* By default, "_HeadOnDisplayText_" uses horizontal text alignment and system font.
		*/
		string m_sUITemplateName;

		/** the font color. The default color is green. */
		DWORD m_dwTextColor;

		/** offset from 3d object's head or origin. Default it (0,0,0) */
		Vector3 m_vOffset;

		/** the camera to object distance where UI scaling is m_fMaxUIScaling */
		float m_fNearZoomDist;
		/** the camera to object distance where UI scaling is m_fMinUIScaling */
		float m_fFarZoomDist;
		/** the min UI scaling factor. */
		float m_fMinUIScaling;
		/** the max UI scaling factor. */
		float m_fMaxUIScaling;
		/** within which the alpha will change from 1 to 0 at the far end. */ 
		float m_fAlphaFadePercentage;
		/** only used when 3d text is used. */
		float m_f3DTextFacing;
		///////////////////////
		//
		// UI object instance that is displayed on head: this feature is not used at the moment. 
		//
		///////////////////////
#ifdef HEADON_UI_OBJ

		/** the interactive UI object id to display. This is always displayed regardless of m_bShow. If 0, no UI instance is attached */
		int m_nUIObjID;
#endif

		/** whether to render the head on display over 3d object. Default it true. */
		bool m_bShow:1;
		/** Enable z buffer test. default to false. */
		bool m_bZEnable:1;
		/** Enable 3d scaling. the text are scaled according to camera to object distance 
		* fScaling = g_fMaxUIScaling - (fDist - g_fNearZoomDist) / (g_fFarZoomDist - g_fNearZoomDist) * (g_fMaxUIScaling-g_fMinUIScaling);
		*/
		bool m_bEnable3DScaling:1;
		/** this is only used when m_bEnable3DScaling. if true, we will use parent SceneObject's parameter for 3d scaling, otherwise the local settings are employed */
		bool m_bUseGlobal3DScaling:1;

		/** we will render the 3d text using the model's world transform. */
		bool m_bRender3DText:1;
	};

	/**
	* Some 3d object may contain some head on display objects. May it be text, icons, or 3D mark, this class provides 
	* common interface for such objects. 
	* The default implementation is very light-weighted. It only has a pointer and size member storage for object that does not have head on display.
	* each object can have specify multiple head on UI at different index. by default it is index 0.
	*/
	class IHeadOn3D
	{
	public:
		IHeadOn3D(void);
		virtual ~IHeadOn3D(void);

	public:
		/** set the text to be displayed on head on display*/
		void SetHeadOnText(const char* sText, int nIndex = 0);

		/** Get the text to be displayed on head on display*/
		const char* GetHeadOnText(int nIndex = 0);

		/** set which UI control the head on display will be used as a template for drawing the text
		* it can be a single CGUIText Object or it can be a container with a direct children called "text"
		* if this is "" or empty, the default UI template will be used. The default UI template is an invisible CGUIText control called "_HeadOnDisplayText_"
		* By default, "_HeadOnDisplayText_" uses horizontal text alignment and system font.
		* @param nIndex: there can be several UI template at position 0,1,2,.. so that position 0 may be used for character name, index 1 for group name, index 2 for question mark icon,etc. 
		*/
		void SetHeadOnUITemplateName(const char* sUIName, int nIndex = 0);

		/** get which UI control the head on display will be used as a template for drawing the text
		* it can be a single CGUIText Object or it can be a container with a direct children called "text"
		* if this is "" or empty, the default UI template will be used. The default UI template is an invisible CGUIText control called "_HeadOnDisplayText_"
		* By default, "_HeadOnDisplayText_" uses horizontal text alignment and system font.
		* @return: it returns NULL if no UI head on display.
		*/
		const char* GetHeadOnUITemplateName(int nIndex = 0);
		const std::string& GetHeadOnUITemplateNameS(int nIndex = 0);
		virtual CGUIBase* GetHeadOnUIObject(int nIndex = 0);

		/** set the text to be displayed on head on display*/
		void SetHeadOnTextColor(DWORD color, int nIndex = 0);

		/** Get the text to be displayed on head on display*/
		DWORD GetHeadOnTextColor(int nIndex = 0);

		/** set the offset where head on display should be rendered relative to the origin or head of the host 3d object*/
		void SetHeadOnOffest(const Vector3& vOffset, int nIndex = 0);

		/** Get the offset where head on display should be rendered relative to the origin or head of the host 3d object*/
		void GetHeadOnOffset(Vector3* pOut, int nIndex = 0);

		/** show or hide object's head on display*/
		void ShowHeadOnDisplay(bool bShow, int nIndex = 0);

		/** whether the object head on display shall be visible*/
		bool IsHeadOnDisplayShown(int nIndex = 0);

		/** whether z buffer test is enabled for Head On display. default to true. */
		bool IsHeadOnZEnabled(int nIndex = 0);

		/** set if z buffer test is enabled for Head On display. default to true.*/
		void SetHeadOnZEnabled(bool bZnabled, int nIndex = 0);

		/** whether the object contains head on display at given index*/
		virtual bool HasHeadOnDisplay(int nIndex=0);


		/** Enable 3d scaling. the text are scaled according to camera to object distance 
		* fScaling = g_fMaxUIScaling - (fDist - g_fNearZoomDist) / (g_fFarZoomDist - g_fNearZoomDist) * (g_fMaxUIScaling-g_fMinUIScaling);
		*/
		bool IsHeadOn3DScalingEnabled(int nIndex = 0);

		/** Enable 3d scaling */
		void SetHeadOn3DScalingEnabled(bool bZnabled, int nIndex = 0);

		/** this is only used when m_bEnable3DScaling. if true, we will use parent SceneObject's paramter for 3d scaling, otherwise the local settings are employed */
		bool IsHeadOnUseGlobal3DScaling(int nIndex = 0);

		/** this is only used when m_bEnable3DScaling. if true, we will use parent SceneObject's paramter for 3d scaling, otherwise the local settings are employed */
		void SetHeadOnUseGlobal3DScaling(bool bZnabled, int nIndex = 0);

		void SetHeadOnNearZoomDist(float fValue, int nIndex = 0);
		float GetHeadOnNearZoomDist(int nIndex = 0);

		void SetHeadOnFarZoomDist(float fValue, int nIndex = 0);
		float GetHeadOnFarZoomDist(int nIndex = 0);

		void SetHeadOnMinUIScaling(float fValue, int nIndex = 0);
		float GetHeadOnMinUIScaling(int nIndex = 0);

		void SetHeadOnMaxUIScaling(float fValue, int nIndex = 0);
		float GetHeadOnMaxUIScaling(int nIndex = 0);

		void SetHeadOnAlphaFadePercentage(float fValue, int nIndex = 0);
		float GetHeadOnAlphaFadePercentage(int nIndex = 0);

		/** turn on 3d facing when this function is called */
		void SetHeadOn3DFacing(float fValue, int nIndex = 0);
		float GetHeadOn3DFacing(int nIndex = 0);

#ifdef HEADON_UI_OBJ	
		/** get the UI object ID that is attached to this character. if 0, it means there is no such UI object. */
		int GetHeadOnUIObjID(int nIndex);

		/** set the UI object ID that is attached to this character. if 0, it means there is no such UI object. */
		void SetHeadOnUIObjID(int nID, int nIndex);

		/** whether there is an head on object.*/
		bool HasHeadOnUIObj(int nIndex);
#endif

		/** draw the headon UI of a specified object. 
		* call this function in a loop like below
		{
			CGUIText* pObjUITextDefault = NULL;
			int nCounter = 0;
			for each obj in a render frame
			{
				if(IHeadOn3D::DrawHeadOnUI(&obj, nCounter, &sceneState, &pObjUITextDefault) == false)
					break;
			}
			IHeadOn3D::DrawHeadOnUI(NULL, nCounter);
		}
		* @param pObj : if this is NULL, it means the last object. it will stop batch rendering by calling EndPaint()
		* @param nCounter: both in and out parameter. if this is 0, it means the start of the character and BeginPaint() is called automatically. 
		*   this function will automatically increase this value if object is drawn
		* @param pSceneState: pointer to a scene state. it can be NULL.
		* @param pObjUITextDefault: see the example above for how to use this. It is the default UI template to use if the object does not provide one. 
		* @param bZEnablePass: if true, we will only draw UI objects with z buffer test enabled.  The pipeline will first draw z enabled pass and then non-Z-enabled pass. 
		* @param b3DTextPass: whether to pass 3d text
		* @return: true if object is drawn. 
		*/
		static bool DrawHeadOnUI(CBaseObject* pObj, int& nCounter, SceneState* pSceneState = NULL, CGUIText** ppObjUITextDefault = NULL, bool bZEnablePass = true, bool b3DTextPass = false);

		/** HeadOn UI needs to be drawn between BeginPaint and EndPaint, which is usually automatically when the first and last head on UI is called. 
		* Hence, one usually do not need to call this method. 
		*/
		static bool BeginPaint(SceneState* pSceneState, bool b3DTextPass, bool bZEnablePass);
		static void EndPaint(SceneState* pSceneState, bool b3DTextPass);

	private:
		/**
		* get the head on 3d data struct. it will create on first use. 
		*/
		HeadOn3DData* GetHeadOn3DData(int nIndex=0);
		/** whether there is data at index*/
		inline bool HasDataAt(int nIndex=0) {return ((int)(m_data.size())>nIndex);}
	private:
		std::vector<HeadOn3DData> m_data;
	};

}
