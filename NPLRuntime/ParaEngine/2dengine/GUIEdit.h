#pragma once
#include "PEtypes.h"
#include "GUIBase.h"
#include "GUIUniBuffer.h"
#include "GUIIMEDelegate.h"

namespace ParaEngine
{
	struct TextureEntity;
	struct AssetEntity;
	struct GUIFontElement;


	//-----------------------------------------------------------------------------
	// EditBox control
	//-----------------------------------------------------------------------------
	class CGUIEditBox : public CGUIBase
// #ifdef PARAENGINE_MOBILE
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
		, public GUIIMEDelegate
#endif
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CGUIEditBox;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CGUIEditBox"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CGUIEditBox, GetCaretColor_s, DWORD*)	{*p1 = cls->GetCaretColor(); return S_OK;}
		ATTRIBUTE_METHOD1(CGUIEditBox, SetCaretColor_s, DWORD)	{cls->SetCaretColor(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CGUIEditBox, GetSelectedBackColor_s, DWORD*)	{*p1 = cls->GetSelectedBackColor(); return S_OK;}
		ATTRIBUTE_METHOD1(CGUIEditBox, SetSelectedBackColor_s, DWORD)	{cls->SetSelectedBackColor(p1); return S_OK;}
        
        ATTRIBUTE_METHOD1(CGUIEditBox, GetEmptyTextColor_s, DWORD*)    { *p1 = cls->GetEmptyTextColor(); return S_OK; }
        ATTRIBUTE_METHOD1(CGUIEditBox, SetEmptyTextColor_s, DWORD)    { cls->SetEmptyTextColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIEditBox, GetEmptyText_s, const char**)	{ *p1 = cls->GetEmptyText().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, SetEmptytext_s, const char*)	{ cls->SetEmptytext(p1); return S_OK; }

// #ifdef PARAENGINE_MOBILE
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
		ATTRIBUTE_METHOD1(CGUIEditBox, SetMoveViewWhenAttachWithIME_s, bool) { cls->setMoveViewWhenAttachWithIME(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, GetIsUseFloatEditBox_s, bool*) { *p1 = cls->IsUseFloatEditBox(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, SetIsUseFloatEditBox_s, bool) { cls->SetIsUseFloatEditBox(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, GetMaxWordLength_s, int*) { *p1 = cls->GetMaxWordLength(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, SetMaxWordLength_s, int) { cls->SetMaxWordLength(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, GetConfirmType_s, const char**) { *p1 = cls->GetConfirmType(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, SetConfirmType_s, const char*) { cls->SetConfirmType(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, GetInputType_s, const char**) { *p1 = cls->GetInputType(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIEditBox, SetInputType_s, const char*) { cls->SetInputType(p1); return S_OK; }
#endif

	public:
		CGUIEditBox();
		virtual ~CGUIEditBox();

		//virtual bool MsgProc(CGUIEvent *event=NULL);
		virtual bool MsgProc(MSG *event);

		void OnSelectStart();

		virtual void UpdateRects();
		virtual bool CanHaveFocus() { return (m_bIsVisible && m_bIsEnabled && m_bCanHasFocus); }
		virtual HRESULT Render(GUIState* pGUIState ,float fElapsedTime);
		virtual bool OnFocusIn();
		virtual bool OnFocusOut();
		virtual bool OnMouseEnter();
		virtual bool OnMouseLeave();
		virtual bool OnChange(const char* code=NULL);
		virtual void InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		virtual string ToScript(int option=0);
		virtual void Reset();

		/** return true, if this control can be clicked. In mobile device we will use the finger size to touch a control
		which has Click event. Generally edit box and button with onclick event */
		virtual bool HasClickEvent();

		/**
		* set the text. 
		* @param szText 
		* @param bSelected whether to set the selection only. This is usually false.
		*/
		void SetTextA( LPCSTR szText, bool bSelected );
		virtual void SetModified();
		/**
		* Set the text of this object
		* @param szText 
		*/
		virtual void SetTextA(const char* szText) { SetTextA(szText, false);};
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
		virtual const char16_t* GetText(){ return m_Buffer.GetBuffer(); };

		/**
		* if the text is longer than the edit box, the returned value is the index of the first visible character in the edit box
		* Normally, this is 0 if the text can be fully contained in the edit box.
		*/
		int GetFirstVisibleCharIndex(){return m_nFirstVisible;};


		/** get the prior word position */
		void GetPriorWordPos( int nCP, int *pPrior );

		/** get the next word position */
		void GetNextWordPos( int nCP, int *pNext );

		/** Character position to X,Y in pixel*/
		HRESULT CPtoXY( int nCP, BOOL bTrail, int *pX, int *pY, bool absolute = false);

		/** X,Y in pixel to character position*/
		HRESULT XYtoCP( int nX, int nY, int *pCP,int *pnTrail, bool absolute = false);

		void GetTextLineSize(int* width, int* height);

		/**
		* return Caret position in characters
		*/
        virtual int GetCaretPosition();
		
		/**
		* Set Caret position in characters
		* @param nCharacterPos: in characters
		*/
		void SetCaretPosition(int nCharacterPos);

		/** whether caret is hidden. this may only be true on IME mode.*/
		virtual bool IsHideCaret() { return false;};

		/**
		* return the text size in Unicode character count. 
		*/
		int GetTextSize();
		
		HRESULT GetTextCopy( std::u16string& out );
		void ClearText();
		virtual void SetTextColor( Color Color ); // Text color
		void SetSelectedTextColor( Color Color );  // Selected text color
		void SetSelectedBackColor( Color Color );  // Selected background color
		Color GetSelectedBackColor();
		void SetEmptyTextColor( Color Color ); // Empty text color
		Color GetEmptyTextColor();
		void SetCaretColor( Color Color );  // Caret color
		Color GetCaretColor();
		void SetBorderWidth( int nBorder ) { m_nBorder = nBorder; UpdateRects(); }  // Border of the window
		const std::string& GetEmptyText();
		void SetEmptytext(const char* utf8Text);

		void SetTextFloatArray( const float *pNumbers, int nCount );
		bool GetReadOnly()const{return m_bReadOnly;	}
		void SetReadOnly(bool readonly){m_bReadOnly=readonly;}

		virtual IObject* Clone()const;
		virtual void Clone(IObject* pobj)const;
		virtual const IType* GetType()const{return m_type;}
		/**
		* Initialize the event mapping for the default CGUIEdit class
		* All its inheritances should have a StaticInit() for each kind of class.
		*/
		static void StaticInit();
		bool IsModified()const{return m_bIsModified;}

		/** this function is called to handle window's latest WM_CHAR message, when the control got key focus. */
		virtual int OnHandleWinMsgChars(const std::wstring& sChars);
		virtual int OnSetEditBoxText(const std::wstring& sChars);

		/**
		* The PasswordChar property specifies the character displayed in the text box. For example, if you want asterisks displayed in the password box, 
		* specify * for the PasswordChar property in the Properties window. Then, regardless of what character a user types in the text box, an asterisk is displayed. 
		* @remark: Security Note: Using the PasswordChar property on a text box can help ensure that other people will not be able to determine a user's password 
		* if they observe the user entering it. This security measure does not cover any sort of storage or transmission of the password that can occur due to 
		* your application logic. Because the text entered is not encrypted in any way, you should treat it as you would any other confidential data. 
		* Even though it does not appear as such, the password is still being treated as a plain-text string (unless you have implemented some additional security measure). 
		* @param PasswordChar such as '*'
		*/
		void SetPasswordChar(char PasswordChar);
		char GetPasswordChar();

		/** Flag to indicate whether caret is currently visible */
		void SetCaretVisible(bool bVisible);

		/** Flag to indicate whether caret is currently visible */
		bool IsCaretVisible();
        
        const RECT& GetCaretRect() const;

		bool IsMultipleLine() { return m_bMultipleLine; }

		//Whether to use an independent input box that follows the keyboard suspension
		bool IsUseFloatEditBox(){return m_bIsUseFloatEditBox;}
		void SetIsUseFloatEditBox(bool val){m_bIsUseFloatEditBox=val;}

        //maximum limit of input
		int GetMaxWordLength(){return m_nMaxWordLength;}
		void SetMaxWordLength(int val){m_nMaxWordLength=val;};

		//Type of input method's OK button,such as : "done","next","search","go","send"
		const char * GetConfirmType(){return m_sComfirmType.c_str();}
		void SetConfirmType(const char *val){m_sComfirmType=val;}

#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
		// ime delegate implementation. 
	public:
		virtual bool attachWithIME() override;
		virtual bool detachWithIME() override;

		void setMoveViewWhenAttachWithIME(bool bMove);

		//Type of filter for input characters,such as :"text","email","number","phone","password"
		const char *GetInputType(){ return m_sInputType.c_str(); }
		void SetInputType(const char *val){ m_sInputType = val; }
	protected:
		virtual bool canAttachWithIME() override;
		virtual void didAttachWithIME() override;
		virtual bool canDetachWithIME() override;
		virtual void didDetachWithIME() override;
		virtual void deleteBackward() override;
		virtual const std::string& getContentText() override;
		virtual const std::u16string& getContentUTF16Text() override;

	private:
		bool	m_bMoveViewWhenAttachWithIME;
#endif
	protected:
		static const IType* m_type;
		void PlaceCaret( int nCP );
		void DeleteSelectionText();
		void ResetCaretBlink();
		void CopyToClipboard();
		void PasteFromClipboard();
		//index =-1 means insert into the current caret
		void InsertChar( WCHAR wChar, int index=-1);
		void InsertCharA( CHAR Char, int index=-1);

		CUniBuffer	m_Buffer;     // Buffer to hold text
		int			m_nBorder;      // Border of the window
		
		//RECT     m_rcText;       // Bounding rectangle for the text
		//RECT     m_rcRender[9];  // Convenient rectangles for rendering elements
		double		m_dfBlink;      // Caret blink time in milliseconds
		double		m_dfLastBlink;  // Last timestamp of caret blink
		bool		m_bCaretOn;     // Flag to indicate whether caret is currently visible
		int			m_nCaret;       // Caret position, in characters
		bool		m_bInsertMode;  // If true, control is in insert mode. Else, overwrite mode.
		int			m_nSelStart;    // Starting position of the selection. The caret marks the end.
		int			m_nFirstVisible;// First visible character in the edit control
		Color	m_SelBkColor;   // Selected background color
		Color	m_CaretColor;   // Caret color
		Color   m_EmptyTextColor; // Emptytext color
		bool		m_bMultipleLine;// if allow multiple line, false by default
		bool		m_bIsModified;
		// Mouse-specific
		bool		m_bMouseDrag;       // True to indicate drag in progress
		bool		m_bReadOnly;

		// key 
		double		m_last_keytime;
		DWORD		m_last_key;//for holding key	
		DWORD		m_key_state; //0:not pressed; 1:pressed; 2:holding a key
		/** The PasswordChar, by default it is '\0'. */
		char m_PasswordChar;
		/** text to show when text is empty: this is usually something like "click to enter text" */
		std::u16string m_empty_text;
		std::string m_empty_text_utf8;
        
        RECT       m_rcCaret;

        bool 		m_bIsUseFloatEditBox;
        int 		m_nMaxWordLength;
        std::string m_sComfirmType;
        std::string m_sInputType;
	};
}
