#pragma once

#include <vector>

#include "GUIBase.h"
namespace ParaEngine
{
	class CGUIContainer;

	class CGUIButton : public CGUIBase
	{
	public:
		CGUIButton();
		virtual ~CGUIButton();
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CGUIButton; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CGUIButton"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CGUIButton, GetOnRenderScript_s, const char**)	{ *p1 = cls->GetOnRenderScript().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIButton, SetOnRenderScript_s, const char*)	{ cls->SetOnRenderScript(p1); return S_OK; }

		virtual bool MsgProc(MSG *event);
		virtual string ToScript(int option=0);
		virtual void Reset();
		virtual HRESULT Render(GUIState* pGUIState, float fElapsedTime);
		virtual HRESULT OnRenderExternal(GUIState* pGUIState, float fElapsedTime);

		virtual bool ActivateScript(const string code){return false;};
		void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		void UpdateRects();

		/**
		* Set the text of this object
		* @param szText 
		*/
		virtual void SetTextA(const char* szText);
		/**
		* Set the text of this object
		* The control internally store everything in Unicode.
		* @param wszText 
		*/
		virtual void SetText(const char16_t* wszText);
		/**
		* get the text of the control
		* @param szText [out] buffer to receive the text
		* @param nLength size of the input buffer
		* @return return the number of bytes written to the buffer. If nLength is 0, this function returns the total number of characters of the text
		*/
		virtual int GetTextA(std::string& out);
		/**
		* get the text of the control 
		* The control internally store everything in Unicode. So it is faster this version than the acsii version.
		* @return 
		*/
		virtual const char16_t* GetText(){return m_szText.data();};

		virtual IObject* Clone()const;
		virtual void Clone(IObject* pobj)const;
		virtual const IType* GetType()const{return m_type;}
		/**
		* Initialize the event mapping for the default CGUIButton class
		* All its inheritances should have a StaticInit() for each kind of class.
		*/
		static void StaticInit();
		
		/** get the animation style of this object. 
		* @return: 0 always means no animation. 
		* 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25% 
		* 11-15 means the same as 1-5, except that the normal state alpha is the same as the highlighted state.
		* 21-25 means the same as 11-15, except that the button animation will not stop in the highlighted state.
		*/
		virtual int GetAnimationStyle();
		/** set the animation style of this object. Different GUI object may have different style definitions.
		* @param nStyle: 0 means no animation. 
		* 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25% 
		* 11-15 means the same as 1-5, except that the normal state alpha is the same as the highlighted state.
		* 21-25 means the same as 11-15, except that the button animation will not stop in the highlighted state.
		* 31-39 is clock-wise rotation, the larger the faster
		* 41-49 is counter-clock-wise rotation, the larger the faster 
		*/
		virtual void SetAnimationStyle(int nStyle);

		/** set whether this is the default button.
		* @param bDefault: true to enable. please note each container can only have one default button. 
		* so as one set a default button, it will automatically unset old default button in the parent container. 
		*/
		void SetDefaultButton(bool bDefault);

		/** whether this is the default button */
		bool IsDefaultButton() {return m_bIsDefaultButton;};

		/** set the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		virtual void SetTextScale(float fScale);
		/** get the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		virtual float GetTextScale();

		/**
		* Default handler of OnMouseEnter event
		* OnMouseEnter is triggered whenever a mouse enters the rectangle of a control. 
		* It's called automatically by the default event system. 
		*/
		virtual bool		OnMouseEnter();

		/**
		* Default handler of OnMouseLeave event
		* OnMouseLeave is triggered whenever a mouse leaves a the rectangle of a control and enters a new control
		* It's called automatically by the default event system before the OnMouseEnter event of the new control.
		*/
		virtual bool		OnMouseLeave();

		/** whether it is pressed. */
		virtual bool IsPressed(){return m_bPressed;} 

		/** padding between the background and content */ 
		virtual void SetPadding( int nPadding);

		/** padding between the background and content */ 
		virtual int GetPadding();

		/** get on render script */
		const std::string& GetOnRenderScript();

		/** set on render script */
		void SetOnRenderScript(const char* sScript);

	protected:
		static const IType* m_type;
		bool				m_bPressed;
		bool				m_bIsDefaultButton;
		std::u16string		m_szText;

		short m_nPadding;
		short m_nAnimationStyle;
		int m_nAnimationData;
		

		/** the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border. 
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		float	m_text_scale;
	};
}
