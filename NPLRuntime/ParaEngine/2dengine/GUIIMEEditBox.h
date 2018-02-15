#pragma once
#include "GUIEdit.h"

#ifdef USE_DIRECTX_RENDERER
#include "GrowableArray.h"
#include "GUIIME.h"

namespace ParaEngine
{
	class CGUIBase;
	struct TextureEntity;
	struct AssetEntity;
	struct GUIFontElement;
	/**
	* IME-enabled EditBox control
	*/
	class CGUIIMEEditBox : public CGUIEditBox, public CGUIIME
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CGUIIMEEditBox;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CGUIIMEEditBox"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

	public:
		CGUIIMEEditBox( );
		virtual ~CGUIIMEEditBox();

		static void StaticInit();
		virtual HRESULT Render(GUIState* pGUIState,float fElapsedTime );
		
		virtual bool MsgProc(MSG *event);
		virtual void UpdateRects();
		virtual bool OnFocusIn();
		virtual bool OnFocusOut();
		virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);

		virtual bool IsHideCaret();

		virtual void PostRender(GUIState *pGUIState, float fElapsedTime);
		virtual void RenderCandidateReadingWindow( GUIState* pGUIState , bool bReading ,float fElapsedTime);
		virtual void RenderComposition( GUIState* pGUIState ,float fElapsedTime );
		virtual void RenderIndicator( GUIState* pGUIState ,float fElapsedTime );

		virtual IObject* Clone()const;
		virtual void Clone(IObject *pobj)const;
		virtual const IType* GetType()const{return m_type;}

		/** check if there is any IME string that the user has typed since last call. 
		* @return true if there is one. 
		*/
		virtual bool FetchIMEString();

		void SendKey( BYTE nVirtKey );
	protected:
		static const IType* m_type;
		// Color of various IME elements
//		Color       m_ReadingColor;        // Reading string color
		Color       m_ReadingWinColor;     // Reading window color
//		Color       m_ReadingSelColor;     // Selected character in reading string
		Color       m_ReadingSelBkColor;   // Background color for selected char in reading str
//		Color       m_CandidateColor;      // Candidate string color
		Color       m_CandidateWinColor;   // Candidate window color
//		Color       m_CandidateSelColor;   // Selected candidate string color
		Color       m_CandidateSelBkColor; // Selected candidate background color
//		Color       m_CompColor;           // Composition string color
		Color       m_CompWinColor;        // Composition string window color
		Color       m_CompCaretColor;      // Composition string caret color
//		Color       m_CompTargetColor;     // Composition string target converted color
		Color       m_CompTargetBkColor;   // Composition string target converted background
//		Color       m_CompTargetNonColor;  // Composition string target non-converted color
		Color       m_CompTargetNonBkColor;// Composition string target non-converted background
//		Color       m_IndicatorImeColor;   // Indicator text color for IME
//		Color       m_IndicatorEngColor;   // Indicator text color for English
		Color       m_IndicatorBkColor;    // Indicator text background color

		// Edit-control-specific data
		int            m_nIndicatorWidth;     // Width of the indicator symbol
		RECT           m_rcIndicator;         // Rectangle for drawing the indicator button
	};
}
#else
namespace ParaEngine
{
	class CGUIBase;
	struct TextureEntity;
	struct AssetEntity;
	struct GUIFontElement;
	/**
	* IME-enabled EditBox control
	*/
	class CGUIIMEEditBox : public CGUIEditBox
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CGUIIMEEditBox; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CGUIIMEEditBox"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }

		static void Initialize(){};
		static void Uninitialize(){}; 
	};
}
#endif