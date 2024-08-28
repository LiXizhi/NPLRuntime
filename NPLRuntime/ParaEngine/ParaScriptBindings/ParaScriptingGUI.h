//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2007 ParaEngine Tech Studio, All Rights Reserved.
// Date:	2005.4
// Description:	API for GUI objects
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingCommon.h"
#include "ParaScriptingGlobal.h"
#include "2dengine/GUIBase.h"
#if defined (USE_FLASH_MANAGER) && defined(USE_DIRECTX_RENDERER)
#include "ParaScriptingFlashPlayer.h"
#endif
#ifdef USE_DIRECTX_RENDERER
#include "ParaScriptingBrowserManager.h"
#include "ParaScriptingHTMLBrowser.h"
#endif
#include "ParaScriptingPainter.h"
#include "ParaScriptingGraphics.h"
#include "Framework/InputSystem/VirtualKey.h"

namespace ParaScripting
{
	using namespace std;
	using namespace luabind;
	using namespace ParaEngine;

	/**
	* @ingroup ParaUI
	* it represents a GUI object.

	@par Class Properties

	- ("text",&ParaUIObject::GetText,&ParaUIObject::SetText1)
	- ("id",&ParaUIObject::GetID,&ParaUIObject::SetID)
	- ("PasswordChar",&ParaUIObject::GetPasswordChar,&ParaUIObject::SetPasswordChar)
	- ("name",&ParaUIObject::GetName,&ParaUIObject::SetName)
	- ("enabled",&ParaUIObject::GetEnabled,&ParaUIObject::SetEnabled)
	- ("highlightstyle",&ParaUIObject::GetHighlightStyle,&ParaUIObject::SetHighlightStyle)
	- ("autosize",&ParaUIObject::GetAutoSize,&ParaUIObject::SetAutoSize)
	- ("visible",&ParaUIObject::GetVisible,&ParaUIObject::SetVisible)
	- ("candrag",&ParaUIObject::GetCanDrag,&ParaUIObject::SetCanDrag)
	- ("scrollable",&ParaUIObject::GetScrollable,&ParaUIObject::SetScrollable)
	- ("readonly",&ParaUIObject::GetReadOnly,&ParaUIObject::SetReadOnly)
	- ("position",&ParaUIObject::GetPosition,&ParaUIObject::SetPosition)
	- ("parent",&ParaUIObject::GetParent,&ParaUIObject::SetParent)
	- ("background",&ParaUIObject::GetBGImage,&ParaUIObject::SetBGImage1)
	- ("color",&ParaUIObject::GetColor,&ParaUIObject::SetColor)
	- ("button",&ParaUIObject::GetBtnImage,&ParaUIObject::SetBtnImage1)
	- ("font",&ParaUIObject::GetFontString,&ParaUIObject::SetFontString1)
	- ("type",&ParaUIObject::GetType)
	- ("shadow",&ParaUIObject::GetUseTextShadow,&ParaUIObject::SetUseTextShadow)
	- ("textscale",&ParaUIObject::GetTextScale,&ParaUIObject::SetTextScale)
	- ("script",&ParaUIObject::ToScript)
	- ("ismodified",&ParaUIObject::IsModified)
	- ("animstyle",&ParaUIObject::GetAnimationStyle,&ParaUIObject::SetAnimationStyle)
	- ("receivedrag",&ParaUIObject::GetReceiveDrag,&ParaUIObject::SetReceiveDrag)
	- ("wordbreak",&ParaUIObject::GetWordbreak,&ParaUIObject::SetWordbreak)
	- ("itemheight",&ParaUIObject::GetItemHeight,&ParaUIObject::SetItemHeight)
	- ("multiselect",&ParaUIObject::GetMultipleSelect,&ParaUIObject::SetMultipleSelect)
	- ("tooltip",&ParaUIObject::GetToolTip,&ParaUIObject::SetToolTip)
	- ("scrollbarwidth",&ParaUIObject::GetScrollbarWidth,&ParaUIObject::SetScrollbarWidth)
	- ("fastrender",&ParaUIObject::GetFastRender,&ParaUIObject::SetFastRender)
	- ("lifetime",&ParaUIObject::GetLifeTime,&ParaUIObject::SetLifeTime)
	- ("zdepth",&ParaUIObject::GetZDepth,&ParaUIObject::SetZDepth)
	- ("value",&ParaUIObject::GetValue,&ParaUIObject::SetValue)
	- ("fixedthumb",&ParaUIObject::GetFixedThumb,&ParaUIObject::SetFixedThumb)
	- ("thumbsize",&ParaUIObject::GetThumbSize,&ParaUIObject::SetThumbSize)
	- ("tooltip",&ParaUIObject::GetToolTip,&ParaUIObject::SetToolTip)
	- ("canvasindex",&ParaUIObject::GetCanvasIndex,&ParaUIObject::SetCanvasIndex)
	- ("zorder",&ParaUIObject::GetZOrder,&ParaUIObject::SetZOrder)
	- ("x",&ParaUIObject::GetX,&ParaUIObject::SetX)
	- ("y",&ParaUIObject::GetY,&ParaUIObject::SetY)
	- ("depth",&ParaUIObject::GetDepth,&ParaUIObject::SetDepth)
	- ("width",&ParaUIObject::Width,&ParaUIObject::SetWidth)
	- ("height",&ParaUIObject::Height,&ParaUIObject::SetHeight)
	- ("rotation",&ParaUIObject::GetRotation,&ParaUIObject::SetRotation)
	- ("scalingx",&ParaUIObject::GetScalingX,&ParaUIObject::SetScalingX)
	- ("scalingy",&ParaUIObject::GetScalingY,&ParaUIObject::SetScalingY)
	- ("translationx",&ParaUIObject::GetTranslationX,&ParaUIObject::SetTranslationX)
	- ("translationy",&ParaUIObject::GetTranslationY,&ParaUIObject::SetTranslationY)
	- ("colormask",&ParaUIObject::GetColorMask,&ParaUIObject::SetColorMask)
	- ("spacing",&ParaUIObject::GetSpacing,&ParaUIObject::SetSpacing)
	- ("popup",&ParaUIObject::GetPopUp,&ParaUIObject::SetPopUp)
	- ("onframemove",&ParaUIObject::GetOnFrameMove,&ParaUIObject::OnFrameMove)
	- ("onclick",&ParaUIObject::GetOnClick,&ParaUIObject::OnClick)
	- ("onchange",&ParaUIObject::GetOnChange,&ParaUIObject::OnChange)
	- ("onkeydown",&ParaUIObject::GetOnKeyDown,&ParaUIObject::OnKeyDown)
	- ("onkeyup",&ParaUIObject::GetOnKeyUp,&ParaUIObject::OnKeyUp)
	- ("ondoubleclick",&ParaUIObject::GetOnDoubleClick,&ParaUIObject::OnDoubleClick)
	- ("ondragbegin",&ParaUIObject::GetOnDragBegin,&ParaUIObject::OnDragBegin)
	- ("ondragend",&ParaUIObject::GetOnDragEnd,&ParaUIObject::OnDragEnd)
	- ("ondragmove",&ParaUIObject::GetOnDragOver,&ParaUIObject::OnDragOver)
	- ("onmousedown",&ParaUIObject::GetOnMouseDown,&ParaUIObject::OnMouseDown)
	- ("onmouseup",&ParaUIObject::GetOnMouseUp,&ParaUIObject::OnMouseUp)
	- ("onmousemove",&ParaUIObject::GetOnMouseMove,&ParaUIObject::OnMouseMove)
	- ("onmousewheel",&ParaUIObject::GetOnMouseWheel,&ParaUIObject::OnMouseWheel)
	- ("onmousehover",&ParaUIObject::GetOnMouseHover,&ParaUIObject::OnMouseHover)
	- ("onmouseenter",&ParaUIObject::GetOnMouseEnter,&ParaUIObject::OnMouseEnter)
	- ("onmouseleave",&ParaUIObject::GetOnMouseLeave,&ParaUIObject::OnMouseLeave)
	- ("onselect",&ParaUIObject::GetOnSelect,&ParaUIObject::OnSelect)
	- ("onfocusin",&ParaUIObject::GetOnFocusIn,&ParaUIObject::OnFocusIn)
	- ("onfocusout",&ParaUIObject::GetOnFocusOut,&ParaUIObject::OnFocusOut)
	- ("onmodify",&ParaUIObject::GetOnModify,&ParaUIObject::OnModify)
	- ("ondestroy",&ParaUIObject::GetOnDestroy,&ParaUIObject::OnDestroy)
	- ("onsize",&ParaUIObject::GetOnSize,&ParaUIObject::OnSize)
	*/
	class PE_CORE_DECL ParaUIObject
	{
	public:
		CGUIBase::WeakPtr_type m_pObj;

		ParaUIObject();
		ParaUIObject(CGUIBase* obj);

		/**
		* check if the object is valid
		* @return: true is the object is a valid GUI object.
		*/
		bool IsValid() const;
		/**
		* Attach a child GUI object to this object
		* This function is only for container objects;
		* @param pChild: the child object to be attached
		*/
		void AddChild(ParaUIObject pChild);

		/** get the attribute object associated with an object. */
		ParaAttributeObject GetAttributeObject();
		/** for API exportation*/
		void GetAttributeObject_(ParaAttributeObject& output);

		/**
		* Add a text item for listbox
		* @param text: the text string to be added.
		*/
		void AddTextItem(const char* text);
		/**
		* Attach this object to the root of the screen GUI.
		*/
		void AttachToRoot();

		void SetName(const char* szName);
		/**
		* Get the name of this object
		*/
		string GetName()const;

		/** get child by name*/
		ParaUIObject GetChild(const char* name);

		/** get child by index.Index start from 0. Use GetChildCount() for total child count. */
		ParaUIObject GetChildAt(int index);

		/** get the total number of child UI objects.*/
		int GetChildCount();

		/** get id of this object. please note that this is a child id, not a globally unique id.
		* the id is only available when this object is attached to a parent. And the ID will change if this object changes its parent.
		* In all other circumstances, the id uniquely identify this object in its parent. One can call GetChildByID() from its parent control to get this object.
		* _Note_: ID is assigned by its parent when this control is attached to a parent control (or parent changes)
		* it ensures that ChildID is unique among all sibling children of the parent control during the lifetime of the parent.
		*/
		int GetID() const;

		/** this function is used internally. never call this unless you known why. */
		void SetID(int nID);

		/** get a child node by its id
		* @param nChildID: child ID usually obtained by GetID() method.
		* @return: return the child object found. it may return invalid object if not found.
		*/
		ParaUIObject GetChildByID(int nChildID);

		/** get the first child node whose name is name. Since a name may not be unique among its sibling children. One is advised to use GetChildByID() instead.
		* @param name: child name usually obtained by GetName() method.
		* @return: return the child object found. it may return invalid object if not found.
		*/
		ParaUIObject GetChildByName(const char* name);


		/**
		* Set if a control is enabled
		* @param bEnabled: the new value
		*/
		void SetEnabled(bool bEnabled);
		bool GetEnabled() const;

		/** Set/Get whether the use text shadow */
		void SetUseTextShadow(bool bUseTextShadow);
		bool GetUseTextShadow() const;

		/** set the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border.
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		void SetTextScale(float fScale);
		/** get the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border.
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		float GetTextScale();

		void SetHighlightStyle(const char* style);
		const char* GetHighlightStyle()const;

		/** Get the UI control color for the current state.*/
		string GetColor()const;

		/** set the UI control color for the current state.
		* @param strColor:"255 0 0" or "255 0 0 128"
		*/
		void SetColor(const char* strColor);

		/** set z depth in the range [0,1], where 0 is closest to screen. if this is -1(default), zdepth is automatically determined. Otherwise it will force the z depth.  */
		void SetZDepth(float fDepth);

		/** get z depth */
		float GetZDepth();

		/**
		* Set if a control is auto resize
		* @param bAutosize: the new value
		*/
		void SetAutoSize(bool bAutosize);
		bool GetAutoSize() const;

		/** get the text line size in pixels, supposing the current font and text will be rendered in a single line. */
		void GetTextLineSize(int* width, int* height);

		/**
		* Set if a control is visible
		* @param bVisible: the new value
		*/
		void SetVisible(bool bVisible);
		bool GetVisible() const;


		bool GetScrollable() const;

		string GetType() const;
		/**
		* Get the resource of this control
		*/
		//ParaUIResource GetResource();

		void SetReceiveDrag(bool bReceiveDrag);
		bool GetReceiveDrag()const;

		ParaUIObject GetParent()const;
		void SetParent(ParaUIObject parent);

		/** get field by name.
		*/
		object GetField(const char* sFieldname, const object& output);

		/** set field by name
		@param sFieldname: field name
		@param input: input value. if field type is vectorN, input is a table with N items.*/
		void SetField(const char* sFieldname, const object& input);

		/** call field by name. This function is only valid when The field type is void.
		It simply calls the function associated with the field name. */
		void CallField(const char* sFieldname);

		/**
		* set as the default button of the container. please note each container can only have one default button.
		* so as one set a default button, it will automatically unset old default button in the parent container.
		* when enter key is pressed in an edit box, the default button will be clicked.
		*/
		void SetDefault(bool bDefaultButton);

		/**
		* Set a count down timer in frames. when it is zero, the object will be automatically deleted
		* If LifeTime is negative, the object will be permanent.
		* @param nSeconds: how many seconds the window left to be alive.
		*/
		void SetLifeTime(int nSeconds);
		/**
		* Get a count down timer in frames. when it is zero, the object will be automatically deleted
		* If LifeTime is negative, the object will be permanent.
		* @return: return how many seconds the window left to be alive.
		*/
		int GetLifeTime()const;

		/**
		Get text of a control;
		*/
		string GetText() const;
		/**
		* Show text in the window. The text will fill the entire window. If one want to
		* position text in a window, it must create another window to contain the text,
		* then use the window to position the text.
		* @param strText: text to display (must be in utf8)
		* @param strColor: color of the object. such as  "255 255 255".
		* @param strFontAssetName: the font asset name, it can be "", in which case the default font will be used.
		*/
		void SetText3(const char* strText, const char* strColor, const char* strFontAssetName);
		void SetText1(const object& strText);
		void SetText(const char* strText);

		/** basically this is the same as the SetText method, except that it will automatically translate the text from one language to another.
		* please note that this method does not solve all localization problems, but it allows for quick machine translation .
		* @param strText: utf8 encoded text
		* @param sConverter: it should be one of the supported converter name.
		*   a most common usage is "c_S2T" where simplified Chinese character is translated to traditional Chinese. The asset file "locale/c_simpl_to_trad.txt" is used.
		*/
		void SetTextAutoTranslate(const char* strText, const char* sConverter);

		/**
		* this function is only supported in the "editbox" control, not the "text" or "IMEEditbox"
		* The PasswordChar property specifies the character displayed in the edit box. For example, if you want asterisks displayed in the password box,
		* specify * for the PasswordChar property in the Properties window. Then, regardless of what character a user types in the text box, an asterisk is displayed.
		* @remark: Security Note: Using the PasswordChar property on a text box can help ensure that other people will not be able to determine a user's password
		* if they observe the user entering it. This security measure does not cover any sort of storage or transmission of the password that can occur due to
		* your application logic. Because the text entered is not encrypted in any way, you should treat it as you would any other confidential data.
		* Even though it does not appear as such, the password is still being treated as a plain-text string (unless you have implemented some additional security measure).
		* @param PasswordChar such as '*'
		*/
		void SetPasswordChar(string PasswordChar);
		/** get password char */
		string GetPasswordChar() const;

		/**
		 * Set a container to be a top-level control or set a top-level control back to normal container.
		 **/
		void SetTopLevel(bool value);
		/**
		* set the background image of the control.
		* @param szBackground: the texture asset file name with the rectangle information
		*  the format of szBackground is filename[; left top width height][:left top toright to bottom]. if it is "" or width or height is zero, the texture is not drawn or is fully transparent.
		*  e.g. "texture/whitedot.png", "texture/whitedot.png;0 0 64 64", "texture/whitedot.png:10 10 10 10", "texture/whitedot.png;0 0 64 64;10 10 10 10"
		*  [:left top toright to bottom] is used to specify the inner rect of the center tile in a nine texture element UI object
		* @note: GUI also supports texture bound with dynamic width and height. such as "Texture/anyfile.dds;0 0 -1 -1",
		* where -1 is replaced with the dynamic width or height when needed. This is specially useful for specifying width
		* and height of a HTTP remote texture where the dimension is not known immediately.
		* things in [] are optional input;if not specified, the rectangle contains the whole picture;
		*/
		void SetBGImage1(const object& szBackground);
		void SetBGImageStr(const char* szBackground);
		void SetBGImage5(const char* TextFilename, int left, int top, int width, int height);
		void SetBGImage(ParaAssetObject pTextureEntity);
		void SetBGImageAndRect(ParaAssetObject pTextureEntity, int left, int top, int width, int height);
		// internally used
		void SetBGImage_(TextureEntity* pTextureEntity, RECT* rect);
		// internally used
		void SetBGImage2(const char* TextFilename, RECT* rect);

		string GetBGImage()const;

		void SetBtnImage2(const char* TextFilename, RECT* rect);
		void SetBtnImage1(const object& szButton);
		string GetBtnImage()const;

		/**
		* drawing the background using nine element.
		* @note: Right now, this is only valid on container, but we will support it for all other controll.
		* @param TextFilename: the background texture to use. it will be divided into 3x3 tiles when drawing the control
		* @param left, top, toRight, toBottom: the inner tile position relative to the dimension of TextFilename texture. If all are 0, it will be set back to one element background
		*/
		void SetNineElementBG(const char* TextFilename, int left, int top, int toRight, int toBottom);

		/**
		* this method is only valid if the control is edit box or imeedit box
		* if the text is longer than the edit box, the returned value is the index of the first visible character in the edit box
		* Normally, this is 0 if the text can be fully contained in the edit box.
		*/
		int GetFirstVisibleCharIndex();

		/**
		* return Caret position in characters
		*/
		int GetCaretPosition();

		/**
		* Set Caret position in characters
		* @param nCharacterPos: in characters
		*/
		void SetCaretPosition(int nCharacterPos);

		/**
		* return the text size in Unicode character count.
		*/
		int GetTextSize();

		/** get the prior word position */
		void GetPriorWordPos(int nCP, int PriorIn, int& Prior);

		/** get the next word position */
		void GetNextWordPos(int nCP, int NextIn, int& Next);

		/** Character position to X,Y in pixel*/
		void CPtoXY(int nCP, bool bTrail, int XIn, int YIn, int& X, int& Y);

		/** X,Y in pixel to character position*/
		void XYtoCP(int nX, int nY, int CPIn, int nTrailIn, int& CP, int& nTrail);

		/**
		* Get the page size of the scroll bar
		* Page size is the size of how many items a page contains. The control will scroll a page size if we click on the empty space of the track of the scroll bar
		*/
		void SetPageSize(int pagesize);

		/** how many pixels to scroll each step */
		void SetStep(int nDelta);

		/** how many pixels to scroll each step */
		int GetStep();

		/**
		* set the page size of a container or a scroll bar
		* @param nBegin: the start value of the range
		* @param nEnd: the end value of the range
		*/
		void SetTrackRange(int nBegin, int nEnd);

		void GetTrackRange(int& nBegin, int& nEnd);

		/** get the animation style of this object. Different GUI object may have different style definitions.
		* for "button" object: 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25%
		* @return: 0 always means no animation.
		* 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25%
		* 11-15 means the same as 1-5, except that the normal state alpha is the same as the highlighted state.
		* 21-25 means the same as 11-15, except that the button animation will not stop in the highlighted state.
		* 31-39 is clock-wise rotation, the larger the faster
		* 41-49 is counter-clock-wise rotation, the larger the faster
		*/
		int GetAnimationStyle() const;

		/** set the animation style of this object. Different GUI object may have different style definitions.
		* for "button" object: 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25%
		* @param nStyle: 0 always means no animation.
		* 1 is gradually enlarge 5% when getting focus.2 is 10%, 3 is 15%, 4 is 20%, 5 is 25%
		* 11-15 means the same as 1-5, except that the normal state alpha is the same as the highlighted state.
		* 21-25 means the same as 11-15, except that the button animation will not stop in the highlighted state.
		* 31-39 is clock-wise rotation, the larger the faster
		* 41-49 is counter-clock-wise rotation, the larger the faster
		*/
		void SetAnimationStyle(int nStyle);

		/**
		* Set if a container is scrollable
		* @param bScrollable: the new scrollable value
		*/
		void SetScrollable(bool bScrollable);

		/**
		* get the absolute position of the control in screen coordinate
		* e.g. local x,y = obj:GetAbsPosition();
		* @param &x screen x
		* @param &y screen y
		* @param &width width
		* @param &height height
		* @param &z depth, usually not used
		*/
		void GetAbsPosition(float& x, float& y, float& width, float& height, float& z)const;


		/** set rect position of the control.
		@param pos: in format "x y width height"
		*/
		void SetPosition(const char* pos);

		/** get rect position of the control*/
		string GetPosition()const;

		/** reposition the control using the same parameter definition used when control is created.
		* see ParaUI::CreateUIObject() for parameter definition. */
		void Reposition(const char* alignment, int left, int top, int width, int height);


		/**
		* Gets and Sets the value of a control
		* Controls that have value are CGUIScrollBar, CGUISlider
		*/
		int GetValue()const;
		void SetValue(int value);
		/**
		* When the user clicked on the window, the script file or function will be activated.
		* The following parameters are passed as global variables.
		*  id = int; -- the id of the triggering GUI window
		*  mouse_button = ["left"|"right"|"middle"];
		*  mouse_x = number;
		*  mouse_y = number;
		* @param strScriptName: the NPL script file to activate.
		*/
		void OnClick(const object& strScriptName);
		string GetOnClick()const;

		/**
		* called when the size of the window is changed.
		*  id = int; -- the id of the triggering GUI window
		*/
		void OnSize(const object& strScriptName);
		string GetOnSize()const;

		/**
		* called when the window is destroyed. Please note that GetUIObject() will return invalid object.
		*/
		void OnDestroy(const object& strScriptName);
		string GetOnDestroy()const;

		/**
		* The framework calls this member function when a window is being activated or deactivated.
		*  id = int; -- the id of the triggering GUI window
		*  param1: Specifies whether the window is being activated or deactivated. It can be one of the following values:
		*  - 0  The window is being deactivated.
		*  - 1  The window is being activated by a mouse click.
		*  - 2  The window is being activated through some method other than a mouse click.
		*/
		void OnActivate(const object& strScriptName);
		string GetOnActivate()const;

		/**
		* The following parameters are passed as global variables.
		*  id = int; -- the id of the triggering GUI window
		*  mouse_button = ["left"|"right"|"middle"];
		*  mouse_x = number;
		*  mouse_y = number;
		* @param strScriptName
		*/
		void OnDoubleClick(const object& strScriptName);
		string GetOnDoubleClick()const;
		/**
		* When the user pressed a key while the mouse cursor is inside the window, a script file or function will be activated.
		* The following parameters are passed as global variables.
		*  id = int; -- the id of the triggering GUI window
		*  keystring = string;--it is filled with the key character. e.g. "\r", "a", etc. It may contain multiple characters, if the user is typing fast or Multi-byte such as Chinese Character is entered.
		*
		* @param strScriptName: the NPL script file to activate.
		*/
		void OnKeyDown(const object& strScriptName);
		string GetOnKeyDown()const;

		void OnKeyUp(const object& strScriptName);
		string GetOnKeyUp()const;

		void OnDragBegin(const object& strScriptName);
		string GetOnDragBegin()const;

		void OnDragEnd(const object& strScriptName);
		string GetOnDragEnd()const;

		void OnDragOver(const object& strScriptName);
		string GetOnDragOver()const;
		void OnMouseMove(const object& strScriptName);
		string GetOnMouseMove()const;
		void OnMouseHover(const object& strScriptName);
		string GetOnMouseHover()const;
		void OnMouseEnter(const object& strScriptName);
		string GetOnMouseEnter()const;
		void OnMouseLeave(const object& strScriptName);
		string GetOnMouseLeave()const;
		void OnMouseDown(const object& strScriptName);
		string GetOnMouseDown()const;
		void OnMouseUp(const object& strScriptName);
		string GetOnMouseUp()const;
		void OnMouseWheel(const object& strScriptName);
		string GetOnMouseWheel()const;
		void OnFrameMove(const object& strScriptName);
		string GetOnFrameMove()const;
		void OnTouch(const object& strScriptName);
		string GetOnTouch()const;
		void OnDraw(const object& strScriptName);
		string GetOnDraw()const;
		void OnInputMethod(const object& strScriptName);
		string GetOnInputMethod()const;

		/**
		* Set/Get cursor file when mouse is over it. If empty, the parent cursor file is used.
		**/
		void				SetCursor(const object& szCursorFile);
		std::string GetCursor() const;
		/** Set cursor by file and hot spot.
		*/
		void				SetCursorEx(const char* szCursorFile, int nHotSpotX, int nHotSpotY);

		/** usually the editbox will call this handle, "virtual_key" contains the last key stroke.
		* Usually application may check if it is an enter key for special actions.*/
		void OnChange(const object& strScriptName);
		string GetOnChange()const;
		void OnSelect(const object& strScriptName);
		string GetOnSelect()const;
		void OnFocusIn(const object& strScriptName);
		string GetOnFocusIn()const;
		void OnFocusOut(const object& strScriptName);
		string GetOnFocusOut()const;

		void OnModify(const object& strScriptName);
		string GetOnModify()const;
		void SetCanDrag(bool bCanDrag);
		bool GetCanDrag() const;

		void SetFontString(const char* font);
		void SetFontString1(const object& font);
		void SetFontString3(const char* fontname, DWORD fontsize, bool isbold);


		string ToScript()const;
		string GetFontString()const;
		//make this object get keyboard focus, if it can get keyboard focus
		void Focus();
		//make this object lost keyboard focus, if it has focus
		void LostFocus();
		bool GetReadOnly()const;
		void SetReadOnly(bool readonly);
		int GetX()const;
		int GetY()const;
		float GetDepth()const;
		void SetDepth(float depth);
		void SetX(int x);
		void SetY(int y);
		void SetWidth(int width);
		void SetHeight(int height);
		int Width()const;
		int Height()const;
		void SetSize(int width, int height);

		/** set the rotation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		void SetRotation(float fRot);

		/** Get the rotation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		float GetRotation() const;

		/** rotation origin offset from the center of the UI object. */
		void SetRotOriginOffset(float x, float y);

		/** rotation origin offset from the center of the UI object. */
		void GetRotOriginOffset(float* x, float* y) const;

		/** set the scaling of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		void SetScaling(float x, float y);

		/** Get the scaling of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		void GetScaling(float* x, float* y) const;

		void SetScalingX(float x);
		float GetScalingX() const;
		void SetScalingY(float y);
		float GetScalingY() const;

		/** set the translation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		void SetTranslation(float x, float  y);

		/** Get the translation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		void GetTranslation(float* x, float* y) const;

		void SetTranslationX(float x);
		float GetTranslationX() const;
		void SetTranslationY(float y);
		float GetTranslationY() const;

		/** color mask, no matter what state the control is in.  "r g b a", such as "255 255 255 255" */
		void SetColorMask(const char* strColor);
		string GetColorMask()const;

		/** automatically animate the child nodes according to this object's rotation, scaling, translation and color mask values.
		*/
		void ApplyAnim();


		/** Spacing between the text and the edge of border */
		void SetSpacing(int nSpacing);

		/** Spacing between the text and the edge of border */
		int GetSpacing() const;

		//for scrollbar and short-cut for container 
		int GetScrollbarWidth()const;
		void SetScrollbarWidth(int width);

		//for scrollbar
		bool GetFixedThumb()const;
		void SetFixedThumb(bool bFixed);

		int GetThumbSize()const;
		void SetThumbSize(int size);

		void SetBindingObj(ParaUIObject obj);//set linked object of a scrollbar

		void SetFastRender(bool fastrender);
		bool GetFastRender()const;

		bool GetWordbreak()const;
		void SetWordbreak(bool wordbreak);

		int GetItemHeight()const;
		void SetItemHeight(int itemheight);

		bool GetMultipleSelect()const;
		void SetMultipleSelect(bool multiple);
		/** Attach the control to the 3D object.
		This function will internally call AttachToRoot() if it has not been called previously
		One can call this function multiple times move the attached GUI object from one 3D object to another.
		@param obj: this must be a global object
		@remark: The visible property of this GUI object is controlled internally as below:
		- if the 3d object does not exist, the GUI object is invisible.
		- if the 3d object is not visible in the current camera frustum, the GUI object is invisible.
		- if the 3d object is visible in the current camera frustum, the GUI object is visible.
		*/
		void AttachTo3D(const ParaObject obj);
		/**
		* save as AttachTo3D, except that the 3D object's name is specified.
		@param s3DObjectName: this must be the name of a global object.
		@remark: The visible property of this GUI object is controlled internally as below:
			- if the 3d object does not exist, the GUI object is invisible.
			- if the 3d object is not visible in the current camera frustum, the GUI object is invisible.
			- if the 3d object is visible in the current camera frustum, the GUI object is visible.
		*/
		void AttachTo3D_(const char* s3DObjectName);

		string GetToolTip()const;
		void SetToolTip(const object& tooltip);

		void RemoveAll();

		void RemoveItem(int index);

		void SetPopUp(int popup);
		int GetPopUp()const;


		ParaUIFont CreateFont(const char* name, const char* fontname);
		ParaUITexture CreateTexture(const char* name, const char* texturename);
		/** get font by name. The prefered way is to get by index.
		* @param name: this is usually "text". */
		ParaUIFont GetFont(const char* name);
		/** get font by index */
		ParaUIFont GetFont_(int nIndex);

		/** try to automatically adjust the size of this object to contain all its content. */
		void DoAutoSize();

		/**
		* Get the texture object associated with the current control A control may be associated with multiple textures.
		* @param name: different controls have set of textures. it can be one of the following predefined string.
			- "background": the background texture, this is the most commonly used texture and is available in all controls.
			- "track": the track texture in a scroll bar. it is available in "scrollbar" control.
			- "up_left": the up arrow texture in scroll bar. it is available in "scrollbar" control.
			- "down_left": the down arrow texture in scroll bar. it is available in "scrollbar" control.
			- "thumb": the thumb button texture in the scroll bar. it is available in "scrollbar" control.
		* @return
		*/
		ParaUITexture GetTexture(const object& name);


		/** bring to front among the same z order */
		void BringToFront();

		/** bring to back among the same z order */
		void BringToBack();

		/** z order affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0. */
		int GetZOrder()const;

		/** z order affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0.
		* if this control has a parent it will cause the parent to sort all children again. */
		void SetZOrder(int nOrder);

		bool IsModified()const;
		/**
		* Set the active layer for setting resource;
		* @param layer: name of the layer
		*				We have three layers "artwork", "background" and "overlay"
		*				"artwork" layer is the primary layer. By default, we render a control using this layer
		*				"overlay" layer is a layer on top of the "artwork" layer.
		*				"background" layer is on the bottom of the "artwork" layer.
		*				All three layers are the same.
		* If not implicitly set, the default active layer is the "artwork" layer. But when a user wants to set something, remember to call this function to ensure that the active layer is correct.
		*/
		void SetActiveLayer(const char* layer);

		/**
		* return whether a given layer exist in the resource. One needs to call SetActiveLayer() in order to create a layer.
		* @param layer: it can be one of the three layers "artwork", "background" and "overlay". Typically, all controls contains the "artwork" layer.
		*/
		bool HasLayer(const char* layer);

		/**
		* Set the current state for setting resource:
		* @param statename: name of the state
		*					We have four states "normal", "highlight", "pressed", "disabled"
		*					"normal" state is the default state.
		*					"highlight" state is the highlighted state. Some controls will change their state to "highlight" automatically when mouse enters them.
		*					"pressed" state is when a button or something else is pressed.
		*					"disabled" state is when a control is disabled.
		* If not implicitly set, the default state is the "normal" state. But when a user wants to set something, remember to call this function to ensure that the current state is correct.
		*/
		void SetCurrentState(const char* statename);

		/**
		* Clones a state to the current state
		* @param statename: name of the state
		*					We have four states "normal", "highlight", "pressed", "disabled"
		*					"normal" state is the default state.
		*					"highlight" state is the highlighted state. Some controls will change their state to "highlight" automatically when mouse enters them.
		*					"pressed" state is when a button or something else is pressed.
		*					"disabled" state is when a control is disabled.
		*/
		void CloneState(const char* statename);
		void ActivateScript(const char* scripttype);

		/**
		* the window's client(child) area that must be recalculated and redrawn
		* this function is obsoleted, call UpdateRect() instead
		*/
		void InvalidateRect();

		/**
		* recalculated the window's client rect
		*/
		void UpdateRect();

	protected:
		string GenGetImage(int index)const;
		string GenGetFont(int index)const;
	private:
		void GetEventScript(DWORD type, string& out)const;
		void SetEventScript(DWORD type, const char* strScriptName);

	};
	/**
	* @ingroup ParaUI
	* ParaUI namespace contains a list of HAPI functions to create user interface controls, such as
	* windows, buttons, as well as event triggers.
	* The main difference between the two is that (1) 2D GUI object are not tested against view frustum,
	* instead it's controlled by visibility tag automatically or through user input. (2) 2D GUI object
	* generally does not obey the physical law of 3D world. (3) GUI object are generally specified by screen
	* coordinates, instead of 3D position. (4) GUI object may be frequently created and discarded.
	* If a object is discarded, so will its child objects. E.g. if one delete a window, all buttons, sensors, etc
	* attach to it will also be discarded.
	*/
	class PE_CORE_DECL ParaUI
	{
	public:
		/**
		* Play a sound file
		* @param strObjectName: the sound object name
		*/
		static void PlaySound(const char* strSoundAssetName, bool bLoop);

		/**
		* stop playing a sound file. one can use this function to stop playing looping sound
		* such as a background music.
		* @param strObjectName: the sound object name
		*/
		static void StopSound(const char* strSoundAssetName);

		/**
		* delete a GUI object as well as all its child objects, by it name.
		* If there are several objects who have the same id, only the last attached one is deleted.
		* @param strObjectName: the object name
		*/
		static void Destroy(const char* strObjectName);
		/**
		* delete a GUI object as well as all its child objects, by it name.
		* If there are several objects who have the same id, only the last attached one is deleted.
		* @param nID: id.
		*/
		static void Destroy1(int nID);

		/** destroy an UI object */
		static void DestroyUIObject(ParaUIObject& obj);

		/**
		* delete at the end of frame.
		*/
		static void PostDestroy(const char* strObjectName);

		/**
		* Get GUI object by name or ID
		*
		* @param NameOrID:
		*	if it is string, we will get the first GUI object that matches the name. If name is "root", the root object is returned.
		*	if it is number , we will get the object by its ID. NameOrID is then the id property value or result of GetID() from a UI object.  If this is 0, the root object is returned.
		*/
		static ParaUIObject GetUIObject_any(const object& NameOrID);
		static ParaUIObject GetUIObject(const char* Name);

		/**
		* Get the first GUI object at the given coordinates
		* @params x: x coordinate
		* @params y: y coordinate
		*/
		static ParaUIObject GetUIObjectAtPoint(int x, int y);

		/**
		* Get the first GUI object at the given coordinates in 3d world space.
		* This function is mostly used in WebXR mode where GUI root is displayed in a fixed 3d plane in world space.
		* in 2d mode, GUI root is supposed to be in the near plane of the camera. and rayX, rayY, rayZ is supposed to be the camera eye position in world space.
		* @param params: {rayX, rayY, rayZ, dirX, dirY, dirZ, maxDistance, filterFunc, hitDist = output}
		*/
		static ParaUIObject Pick(const object& params);

		/**
		* get the current mouse position.
		* e.g. local x,y = ParaUI.GetMousePosition();
		* @param &x screen x
		* @param &y screen y
		*/
		static void GetMousePosition(float& x, float& y);

		static void SetMousePosition(float x, float y);

		/* check if a given key is currently in pressed state. Please note that there may be time delays since the scripting interface is called at regularly interval.
		* e.g. ParaUI.IsKeyPressed(DIK_SCANCODE.DIK_LCONTROL) or ParaUI.IsKeyPressed(DIK_SCANCODE.DIK_LSHIFT) or ParaUI.IsKeyPressed(DIK_SCANCODE.DIK_LMENU)
		* @param nDikScanCode: DirectInput keyboard scan codes.
		*/
		static bool IsKeyPressed(int nDikScanCode);

		/* check if a given mouse button is currently in pressed state.
		* @param nButton: 0 is left, 1 is right, 2 is middle.
		this is just traditional mouse key mapping.
		enum MOUSE_KEY_STD{
			LEFT_BUTTON = 0,
			RIGHT_BUTTON = 1,
			MIDDLE_BUTTON = 2
		};
		*/
		static bool IsMousePressed(int nButton);

		/**
		* Create a GUI object based on the default template object.
		* All default template object is defined in "script/config.lua", which will be loaded when GUI engine is loaded. One can change template object at runtime
		* by GetDefaultObject(). @see GetDefaultObject()
		* Although it is possible to create many objects with the same name, we do not advice you to do so.
		* @param strType: type of the new object. It can be "container", "button", "scrollbar", "editbox", "imeeditbox","slider", "video", "3dcanvas", "listbox", "painter" and "text".
		*				"container" is the only type of control that can contain other objects
		* @param strObjectName: the new object's name
		* @param alignment: can be one of the following strings or nil or left out entirely:
		*	- "_lt" align to left top of the screen
		*	- "_lb" align to left bottom of the screen
		*	- "_ct" align to center of the screen
		*	- "_ctt": align to center top of the screen
		*	- "_ctb": align to center bottom of the screen
		*	- "_ctl": align to center left of the screen
		*	- "_ctr": align to center right of the screen
		*	- "_rt" align to right top of the screen
		*	- "_rb" align to right bottom of the screen
		*	- "_mt": align to middle top
		*	- "_ml": align to middle left
		*	- "_mr": align to middle right
		*	- "_mb": align to middle bottom
		*	- "_fi": align to left top and right bottom. This is like fill in the parent window.
		*
		*	the layout is given below:\n
		*	_lt _mt _rt	\n
		*	_ml _ct _mr \n
		*	_lb _mb _rb \n
		* @param x: screen coordinate x, for _m? alignments, the meaning is different, see remark
		* @param y: screen coordinate y, for _m? alignments, the meaning is different, see remark
		* @param width: screen coordinate width or right, depending on alignment mode, for _m? alignments, the meaning is different, see remark
		* @param height: screen coordinate height or bottom, depending on alignment mode, for _m? alignments, the meaning is different, see remark
		* @remark: we design _m? alignment types.
		*    _mt: x is coordinate from the left. y is coordinate from the top, width is the coordinate from the right and height is the height
		*    _mb: x is coordinate from the left. y is coordinate from the bottom, width is the coordinate from the right and height is the height
		*    _ml: x is coordinate from the left. y is coordinate from the top, width is the width and height is the coordinate from the bottom
		*    _mr: x is coordinate from the right. y is coordinate from the top, width is the width and height is the coordinate from the bottom
		*/
		static ParaUIObject CreateUIObject(const char* strType, const char* strObjectName, const char* alignment, int x, int y, int width, int height);

		/**
		* get the top level control at level 0
		*/
		static ParaUIObject GetTopLevelControl();


		/**
		* get the default template object from which all sub-sequent controls of the same type are cloned(created).
		* one can modify the template object at runtime to change of the theme of all controls created subsequently.
		* All default template object is defined in "script/config.lua", which will be loaded when GUI engine is loaded
		* @param strType: It can be "container", "button", "scrollbar", "editbox", "imeeditbox","slider" and "text".
		* @return the template object is returned.
		*/
		static ParaUIObject GetDefaultObject(const char* strType);
		/** @ obsoleted. Set Mouse Cursor appearance*/
		static void SetCursorFont(const char* fontname, const char* strColor, DWORD transparency);
		/**	@obsoleted. Set Mouse Cursor appearance */
		static void SetCursorTexture(const char* texturename, const char* strColor, DWORD transparency);
		/** @obsoleted. Set Mouse Cursor appearance	*/
		static void SetCursorText(const char* strText);
		/**
		* Set the current cursor to use. One can call very often, since it will does nothing with identical cursor file and hot spot.
		* Typically, hardware supports only 32x32 cursors and, when windowed, the system might support only 32x32 cursors.
		* @param szCursor cursor file name: The contents of this texture will be copied and potentially format-converted into an internal buffer from which the cursor is displayed. The dimensions of this surface must be less than the dimensions of the display mode, and must be a power of two in each direction, although not necessarily the same power of two. The alpha channel must be either 0.0 or 1.0.
		* @param XHotSpot [in] X-coordinate offset (in pixels) that marks the center of the cursor. The offset is relative to the upper-left corner of the cursor. When the cursor is given a new position, the image is drawn at an offset from this new position determined by subtracting the hot spot coordinates from the position.
		* @param YHotSpot [in] Y-coordinate offset (in pixels) that marks the center of the cursor. The offset is relative to the upper-left corner of the cursor. When the cursor is given a new position, the image is drawn at an offset from this new position determined by subtracting the hot spot coordinates from the position.
		*/
		static void SetCursorFromFile(const char* szCursor, int XHotSpot, int YHotSpot);
		/** same as above, with hot spot (0,0) */
		static void SetCursorFromFile_(const char* szCursor);
		/*
		* get the current cursor file name.
		* @return: the cursor file is returned.
		*/
		static const char* GetCursorFile();

		/** whether to use system cursor. If this is true, d3d hardware cursor is not shown, even you have loaded an cursor icon using SetCursorFromFile.*/
		static void SetUseSystemCursor(bool bUseSystem);

		/** get whether to use system cursor*/
		static bool GetUseSystemCursor();

		/** to NPL script.*/
		static string ToScript();

		/**
		* save current layout to file
		* @param *filename
		* @return
		*/
		static bool SaveLayout(const char* filename);

		/**
		* whether to enable edit mode for all controls.
		* @param bDesign
		*/
		static void SetDesignTime(bool bDesign);

		/**
		* show cursor
		* @param bShow
		*/
		static void ShowCursor(bool bShow);

		/**
		* Lock Mouse so that mouse move will not change the mouse position. this is useful when user is changing camera view during mouse drag operation.
		* @param bLock: true to lock
		*/
		static void LockMouse(bool bLock);

		/**
		* check whether Mouse is locked. When mouse is locked, mouse move will not change the mouse position. this is useful when user is changing camera view during mouse drag operation.
		*/
		static bool IsMouseLocked();

		/** clear all UI objects.*/
		static void ResetUI();

		/** This function opens or closes the IME programmtically.
		* In most cases, we should never programmatically open or closes IME, instead the user usually pressed Ctrl+Space to change it.
		* however, in some rare cases, such as we are opening a windowed mode flash window, and wants to disable IME programmatically.
		* @param bOpen: true to open.
		*/
		static void SetIMEOpenStatus(bool bOpen);

		/** Check if IME status is opened. */
		static bool GetIMEOpenStatus();


		/** set the UI scaling. This can be useful to render 1024*768 to a 800*600 surface; we can set to fScalingX to 800/1024 and fScalingY to 600/768
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called.
		* @param fScalingX: x defaults to 1.0
		* @param fScalingY: y defaults to 1.0
		*/
		static void SetUIScale(float fScalingX, float fScalingY);

		/** the minimum screen size. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		* @param nWidth: the new width.
		* @param nHeight: the new height.
		* @param bAutoUIScaling: usually set to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		*/
		static void SetMinimumScreenSize(int nWidth, int nHeight, bool bAutoUIScaling);

		/** the maximum screen size. if the backbuffer is larger than this, we will use automatically use UI scaling
		 * for example, if maximum width is 1024, and backbuffer it 1600, then m_fUIScalingX will be automatically set to 1024/1600.
		 * @param nWidth: the new width.
		 * @param nHeight: the new height.
		 * @param bAutoUIScaling: usually set to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		 */
		static void SetMaximumScreenSize(int nWidth, int nHeight, bool bAutoUIScaling);

		/** add an receiver to the current receiver list during an drag operation.
		* call this function on an dragable UI object's Begin Drag event handler.
		* @param sName: name. if this is "root", the dragging object can always to reattached.
		*/
		static void AddDragReceiver(const char* sName);


		/**
		* How tooltip is displayed
		* @param behavior  "normal" or "flashing". default is "normal"
		*/
		static void SetToolTipBehavior(const char* behavior);

		/**
		* Set Highlight Param. this is usually called in the start up configuration file.
		* @param szEffectName
		* @param szParamName
		* @param szParamValue
		* @return
		*/
		static bool SetHighlightParam(const char* szEffectName, const char* szParamName, const char* szParamValue);

#ifdef USE_DIRECTX_RENDERER
#ifdef USE_FLASH_MANAGER
		/**
		* get flash player pointer by name. this function does not create any player if there is no player with the given name.
		*/
		static ParaFlashPlayer GetFlashPlayer(const char* sFileName);

		/**
		* create get flash player pointer by name. this function will create any player if there is no player with the given name.
		*/
		static ParaFlashPlayer CreateFlashPlayer(const char* sFileName);

		/**
		* get flash player pointer by index. this function does not create any player if there is no player at the given index.
		*/
		static ParaFlashPlayer GetFlashPlayer1(int nIndex);
#endif
#endif
	};
}
