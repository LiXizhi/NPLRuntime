#pragma once

#include <vector>
#include <string>
#include <deque>
#include "math/ParaPoint.h"
#include "util/ParaMemPool.h"
#include "GUIEvent.h"
#include "IObjectDrag.h"
#include "Type.h"
#include "IAttributeFields.h"
#include "GUIPosition.h"
#include "GUIState.h"
#include "PaintEngine/PaintDevice.h"
#include <stdint.h>

namespace ParaEngine
{
	using namespace std;

	class CBaseObject;
	class CEventBinding;
	class SimpleScript;
	class CRenderTarget;

	typedef FixedSizedAllocator<CGUIBase*>	DL_Allocator_CGUIBase_Ptr;
	typedef std::deque<CGUIBase*, DL_Allocator_CGUIBase_Ptr>	GUIBase_List_Type;

	class CGUIBase;
	class CSceneObject;
	class CGUISensor;
	class CGUIResource;
	class CGUIContainer;
	struct GUITextureElement;
	struct GUIFontElement;
	struct TouchEvent;

	struct AssetEntity;

	/** base object for all 2D GUI objects
	* (1) 2D GUI object are not tested against view frustum, instead it is controlled by
	* visibility tag automatically or through user input. (2) 2D GUI object generally
	* does not obey the physical law of 3D world. (3) GUI object are generally specified
	* by screen coordinates, instead of 3D position. (4) GUI object may be frequently created
	* and discarded. They may also have a fixed life time, as in the 2D sprite animation
	* of a magic effect.
	*
	* Base class for all GUI elements:
	* So the best way to do a GUI is to add two classes of object (1) status objects (2) GUI sensor object.
	* - Status object are solely used to render annotations to the 3D world object, or the screen.
	* - GUI sensor object however has no display element, it merely triggers POL neuron file,
	*   once the user clicked or typed in something.
	*/
	class CGUIBase : public IAttributeFields, public IObjectDrag, public CPaintDevice
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, CGUIBase> WeakPtr_type;

		virtual const IType* GetType()const { return m_type; };
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CGUIBase; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CGUIBase"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** convert to object of a given type.
		* @param nObjectType: such as values in ATTRIBUTE_CLASSID_TABLE, In most cases, we query ATTRIBUTE_CLASSID_CBaseObject
		*/
		virtual void* QueryObject(int nObjectType);
		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CGUIBase, GetName_s, const char**)	{ *p1 = cls->GetName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetName_s, const char*)	{ cls->SetName(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTextScale_s, float*)	{ *p1 = cls->GetTextScale(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTextScale_s, float)	{ cls->SetTextScale(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetUseTextShadow_s, bool*)	{ *p1 = cls->GetUseTextShadow(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetUseTextShadow_s, bool)	{ cls->SetUseTextShadow(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTextShadowQuality_s, int*)	{ *p1 = cls->GetTextShadowQuality(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTextShadowQuality_s, int)	{ cls->SetTextShadowQuality(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTextShadowColor_s, int*)	{ *p1 = cls->GetTextShadowColor(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTextShadowColor_s, int)	{ cls->SetTextShadowColor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTextOffsetX_s, int*)	{ *p1 = cls->GetTextOffsetX(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTextOffsetX_s, int)	{ cls->SetTextOffsetX(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTextOffsetY_s, int*)	{ *p1 = cls->GetTextOffsetY(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTextOffsetY_s, int)	{ cls->SetTextOffsetY(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetEnabled_s, bool*)	{ *p1 = cls->GetEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetEnabled_s, bool)	{ cls->SetEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetAlwaysMouseOver_s, bool*)	{ *p1 = cls->GetAlwaysMouseOver(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetAlwaysMouseOver_s, bool)	{ cls->SetAlwaysMouseOver(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetDepth_s, float*)	{ *p1 = cls->GetDepth(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetDepth_s, float)	{ cls->SetDepth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetLifeTime_s, int*)	{ *p1 = cls->GetLifeTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetLifeTime_s, int)	{ cls->SetLifeTime(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetX_s, int*)	{ *p1 = cls->GetX(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetX_s, int)	{ cls->SetX(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetY_s, int*)	{ *p1 = cls->GetY(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetY_s, int)	{ cls->SetY(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetWidth_s, int*)	{ *p1 = cls->GetWidth(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetWidth_s, int)	{ cls->SetWidth(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetHeight_s, int*)	{ *p1 = cls->GetHeight(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetHeight_s, int)	{ cls->SetHeight(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetRotation_s, float*)	{ *p1 = cls->GetRotation(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetRotation_s, float)	{ cls->SetRotation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetScaling_s, Vector2*)	{ cls->GetScaling(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetScaling_s, Vector2)	{ cls->SetScaling(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTranslation_s, Vector2*)	{ cls->GetTranslation(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTranslation_s, Vector2)	{ cls->SetTranslation(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetRotOriginOffset_s, Vector2*)	{ cls->GetRotOriginOffset(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetRotOriginOffset_s, Vector2)	{ cls->SetRotOriginOffset(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetColorMask_s, DWORD*)	{ *p1 = cls->GetColorMask(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetColorMask_s, DWORD)	{ cls->SetColorMask(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetText_s, const char**)	{ 
			static std::string sStr;
			sStr.clear();
			cls->GetTextA(sStr);
			*p1 = sStr.c_str(); return S_OK;
		}
		ATTRIBUTE_METHOD1(CGUIBase, SetText_s, const char*)	{ cls->SetTextA(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetSpacing_s, int*)	{ *p1 = cls->GetSpacing(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetSpacing_s, int)	{ cls->SetSpacing(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetPadding_s, int*)	{ *p1 = cls->GetPadding(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetPadding_s, int)	{ cls->SetPadding(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetZOrder_s, int*)	{ *p1 = cls->GetZOrder(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetZOrder_s, int)	{ cls->SetZOrder(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetCursor_s, const char**)	{ *p1 = cls->GetCursor().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetCursor_s, const char*)	{ cls->SetCursor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetIndex_s, int*)	{ *p1 = cls->GetIndex(); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetID_s, int*)	{ *p1 = cls->GetID(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetID_s, int)	{ cls->SetID(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetVisible_s, bool*)	{ *p1 = cls->GetVisible(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetVisible_s, bool)	{ cls->SetVisible(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetVisibleRecursive_s, bool*)	{ *p1 = cls->GetVisibleRecursive(); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetHighlightStyle_s, int*)	{ *p1 = cls->GetHighlightStyle(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetHighlightStyle_s, int)	{ cls->SetHighlightStyle(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetAnimationStyle_s, int*)	{ *p1 = cls->GetAnimationStyle(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetAnimationStyle_s, int)	{ cls->SetAnimationStyle(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsPressed_s, bool*)	{ *p1 = cls->IsPressed(); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsClickThrough_s, bool*)	{ *p1 = cls->IsClickThrough(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetClickThrough_s, bool)	{ cls->SetClickThrough(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, CanHaveFocus_s, bool*)	{ *p1 = cls->CanHaveFocus(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetCanHaveFocus_s, bool)	{ cls->SetCanHaveFocus(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsDirty_s, bool*)	{ *p1 = cls->IsDirty(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetDirty_s, bool)	{ cls->SetDirty(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsOwnerDraw_s, bool*)	{ *p1 = cls->IsOwnerDraw(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetOwnerDraw_s, bool)	{ cls->SetOwnerDraw(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsUVWrappingEnabled_s, bool*)	{ *p1 = cls->IsUVWrappingEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, EnableUVWrapping_s, bool)	{ cls->EnableUVWrapping(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, HasKeyFocus_s, bool*)	{ *p1 = cls->HasKeyFocus(); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetUsePointTextureFiltering_s, bool*)	{ *p1 = cls->GetUsePointTextureFiltering(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetUsePointTextureFiltering_s, bool)	{ cls->SetUsePointTextureFiltering(p1); return S_OK; }

		ATTRIBUTE_METHOD(CGUIBase, ApplyColorMask_s)	{ cls->ApplyColorMask(); return S_OK; }
		ATTRIBUTE_METHOD(CGUIBase, ApplyAnim_s)	{ cls->ApplyAnim(); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsSelfPaintEnabled_s, bool*)	{ *p1 = cls->IsSelfPaintEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, EnableSelfPaint_s, bool)	{ cls->EnableSelfPaint(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsAutoClearBackground_s, bool*)	{ *p1 = cls->IsAutoClearBackground(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetAutoClearBackground_s, bool)	{ cls->SetAutoClearBackground(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsMouseCaptured_s, bool*)	{ *p1 = cls->IsMouseCaptured(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetMouseCaptured_s, bool)	{ (p1 ? cls->CaptureMouse() : cls->ReleaseMouseCapture()); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsInputMethodEnabled_s, bool*)	{ *p1 = cls->IsInputMethodEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetInputMethodEnabled_s, bool)	{ cls->SetInputMethodEnabled(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetCompositionPoint_s, Vector2*)	{ auto pt = cls->GetCompositionPoint(); *p1 = Vector2((float)pt.x(), (float)pt.y()); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetCompositionPoint_s, Vector2)	{ cls->SetCompositionPoint(QPoint((int)p1.x, (int)p1.y)); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, IsNonClientTestEnabled_s, bool*)	{ *p1 = cls->IsNonClientTestEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, EnableNonClientTest_s, bool)	{ cls->EnableNonClientTest(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIBase, GetTouchTranslationAttFlag_s, int*) { *p1 = cls->GetTouchTranslationAttFlag(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIBase, SetTouchTranslationAttFlag_s, int) { cls->SetTouchTranslationAttFlag(p1); return S_OK; }
	public:
		virtual CPaintEngine * paintEngine() const;

		void DeleteThis();

		/** get the parent container */
		CGUIContainer* GetParent(){ return m_parent; }
		/** get the parent container which has a self painted painter device. It will return null if it does not belong to any painter device. */
		CGUIContainer* GetPainterWindow();
		/** if control contains its own painting device. i.e. self painted as a window. */
		inline bool IsPainterWindow(){
			return IsSelfPaintEnabled();
		}
		
		virtual int GetChildCount();

		/** return true, if this control can be clicked. In mobile device we will use the finger size to touch a control
		which has Click event. Generally edit box and button with onclick event */
		virtual bool HasClickEvent();

		/** return true if current control is a parent of pChild. */
		bool IsAncestorOf(CGUIBase * pChild);

		/** return true if pParent is a predecessor of this object. */
		bool HasParent(CGUIContainer* pParent);

		/** set the parent container */
		void SetParent(CGUIContainer* parent){ m_parent = parent; }

		/** get the child index in the parent object. this function returns -1 if no parent found. */
		int GetIndex();

		/** whether control is enabled.*/
		bool GetEnabled(){ return m_bIsEnabled; }

		/** set whether the control is enabled or not*/
		void SetEnabled(bool bEnabled){ m_bIsEnabled = bEnabled; }

		bool IsInputMethodEnabled() const;
		void SetInputMethodEnabled(bool val);

		/** set need update */
		void SetNeedUpdate(bool bNeedUpdate = true) { m_bNeedUpdate = bNeedUpdate; }
		/** whether this control needs update. 
		* @param bRecursive: if true, this function will return true if any of its visible child node needs update recursively. 
		*/
		bool IsNeedUpdate(bool bRecursive = false);

		/** set whether make mouse over to true as long as it contains the mouse point, no matter it has mouse focus or not.
		* default to false.
		*/
		void SetAlwaysMouseOver(bool bAlwaysMouseOver){ m_bAlwaysMouseOver = bAlwaysMouseOver; }

		/** get whether make mouse over to true as long as it contains the mouse point, no matter it has mouse focus or not.
		* default to false.
		*/
		bool GetAlwaysMouseOver(){ return m_bAlwaysMouseOver; }

		/** get pointer to children list. It may return NULL, if the control can not have children. */
		virtual GUIBase_List_Type*	GetChildren(){ return NULL; };

		/** whether it is pressed. */
		virtual bool IsPressed(){ return false; }

		//new
		/**
		* Initialize the event mapping for the default CGUIBase class
		* All its inheritances should have a StaticInit() for each kind of class.
		*/
		static void			StaticInit();

		/**
		* Set the text of this object
		* @param szText
		*/
		virtual void SetTextA(const char* szText){};
		/**
		* Set the text of this object
		* The control internally store everything in Unicode.
		* @param wszText
		*/
		virtual void SetText(const char16_t* wszText){};
		/**
		* get the text of the control
		* @param szText [out] buffer to receive the text
		* @param nLength size of the input buffer
		* @return return the number of bytes written to the buffer. If nLength is 0, this function returns the total number of characters of the text
		*/
		virtual int GetTextA(std::string& out);;
		/**
		* get the text of the control
		* The control internally store everything in Unicode. So it is faster this version than the acsii version.
		* @return
		*/
		virtual const char16_t* GetText();

		/**
		* Resets the objects states. Descendants should have their own reset function;
		*/
		virtual void		Reset();
		// -- Base object functions
		/** check if the object has any events */
		//	bool HasEvent(){return !m_objEvents.empty();}
		/** check if the object has a certain event handler
		* @etype: script type.
		*/
		bool				HasEvent(int etype);

		/**
		Clear all events in the event buffer (m_objEvents), not including interpreted events such as m_pMouseEvent and m_pKeyEvent
		*/
		void				ClearAllEvent();

		/**
		Clear specific kind of events in the event buffer (m_objEvents), including interpreted events such as m_pMouseEvent and m_pKeyEvent
		@param etype: Type of events to clear. Can be GUI_EVENT_TYPE::KEYBOARD or GUI_EVENT_TYPE::MOUSE
		*/
		void				ClearEvent(int etype);

		/** draw calls between BeginDepthTest() and EndDepthTest() are rendered with depth enabled,
		* such as for relative to 3d objects. Nested called are allowed and detected.
		*/
		void BeginDepthTest(GUIState* pState = NULL);
		void EndDepthTest(GUIState* pState = NULL);
		
		/**
		* Gets the absolute position of a CGUIPosition structure.
		* @param pIn: Pointer to the input CGUIPosition structure.
		* @param pOut: Pointer to the output CGUIPosition structure.
		* @see CGUIPosition for more details.
		*/
		virtual void		GetAbsolutePosition(CGUIPosition* pOut, const CGUIPosition* pIn);

		/** set the 3D depth. this is usually automatically set when object is attached to 3d object.
		* it the object contains children. it will also affect all children automatically.
		* @param fDepth: a value of [0,1).  Default to 0, which is closest to the screen.
		*/
		virtual void	SetDepth(float fDepth);

		/** Get the 3D depth.
		*/
		virtual float GetDepth();


		bool HasFocus() const;

		/** whether this control has key focus. */
		virtual bool HasKeyFocus();

		/**
		the procedure that handles all the events. It automatically calls some default event handlers.
		@return: return true if we want to tell it's caller to suppress further handling process.
		Otherwise, return false
		By default, the MsgProc() in CGUIBase returns false.
		*/
		virtual bool		MsgProc(MSG *event);

		/**
		 * Set/get how many frames the control will survive. -1 is permanent
		 **/
		virtual void		SetLifeTime(int lifttime){ m_nLifeTimeCountDown = lifttime < 0 ? -1 : lifttime; }
		virtual int			GetLifeTime()const{ return m_nLifeTimeCountDown; }

		/**
		* Activate the given code of a type of event
		* The activated script will send to the script buffer in the root control and wait for the script engine to execute it.
		* The function will return immediately. So the script execution is asynchronous.
		* @code: the script to be activated. It is generate by the GUI engine.
		* @etype: script type.
		* @remark: Sometimes we have user-defined script codes initially. We will concatenate parameter code and user-defined script code together.
		* The parameter code will execute before user-defined script code.
		* @return: always return false.
		*/
		virtual bool		ActivateScript(const string &code, int etype);

		/**
		* Activate the given code of a type of event
		* The activated script will send to the script buffer in the root control and wait for the script engine to execute it.
		* The function will return immediately. So the script execution is asynchronous.
		* @precode: the script to be activated before the user-defined script code. It is generate by the GUI engine.
		* @postcode: the script to be activated after the user-defined script code. It is generate by the GUI engine.
		* @etype: script type.
		* @remark: Sometimes we have user-defined script codes initially. We will concatenate parameter code and user-defined script code together.
		* Unlike another version of ActivateScript(), this one concatenate precode and subcode before and after the user-defined script code.
		* @return: always return false.
		*/
		virtual bool		ActivateScript(const string &precode, const string &postcode, int etype);

		/**
		* Updates the drawing rectangles of a control, it needs update
		* It is overridden by each type of controls. It should be called whenever the location and size of the control changes.
		*/
		virtual void		UpdateRects();

		/** refresh parent if control is already attached. This function is usually called after the position or size of this control is changed.
		*/
		void		UpdateParentRect();

		/** adds a rectangle to the specified window's update region. The update region represents the portion of the
		* window's client(child) area that must be recalculated and redrawn
		* @param lpRect:[in] Pointer to a RECT structure that contains the client coordinates of the rectangle to be added to
		* the update region. If this parameter is NULL, the entire client(child) area is added to the update region.
		* @return true if succeed.
		*/
		virtual bool InvalidateRect(const RECT* lpRect);
		RECT GetClippingRect(GUIState* pGUIState);

		/** try to automatically adjust the size of this object to contain all its content. */
		virtual bool DoAutoSize();

		/**
		 * Begin() is called before Render() in render time
		 **/
		virtual void		Begin(GUIState* pGUIState, float fElapsedTime);
		/**
		* End() is called after Render() in render time
		**/
		virtual void		End(GUIState* pGUIState, float fElapsedTime);
		/**
		* Default render function of the control.
		* All descendants of CGUIBase should override this function.
		* @param pGUIState: a pointer to the GUIState object;
		* @return: return S_OK if success;
		*/
		virtual HRESULT			Render(GUIState* pGUIState, float fElapsedTime){ return S_OK; };

		/** perform rendering on current render state. */
		virtual HRESULT			DoRender(GUIState* pGUIState, float fElapsedTime);

		/** draw the object */
		void Draw(GUIState* pGUIState = NULL, float fElapsedTime = 0.f);

		/** fill the clipping region with default color, transparent color or background color. */
		virtual void FillClippingRegion(GUIState* pGUIState);
		virtual HRESULT			DoSelfPaint(GUIState* pGUIState, float fElapsedTime);
		/** owner draw handler. It just calls in to the scripting interface for user provided draw functions.
		* return true if something is drawn, or false where we will still use the system drawing.
		*/
		virtual bool			OnOwnerDraw(GUIState* pGUIState);

		//it may not be useful.
		//virtual void Framemove( double dTimeDelta ){};

		/// used as KEY for batch rendering, no use now, return resource
		//virtual AssetEntity* GetPrimaryAsset(){return NULL;};				

		/** weather we can capture the mouse input. */
		inline bool CanCaptureMouse() { return m_bCanCaptureMouse; };
		void SetCanCaptureMouse(bool bCanCapture);

		//decide whether a control can have keyboard focus
		virtual bool		CanHaveFocus();
		virtual void		SetCanHaveFocus(bool bCanHaveFocus);

		//finds whether the given point is in the rectangle, the input rectangle should be in absolute position
		virtual BOOL		ContainsPoint(const RECT &pos, const POINT &pt)const;
		virtual BOOL		ContainsPoint(const POINT &pt)const;
		/**
		* whether the given point is in this control.
		* @param x usually the current mouse position
		* @param y usually the current mouse position
		* @return true
		*/
		virtual BOOL		IsPointInControl(int x, int y);

		/**
		 * Bunch of functions for setting size and location
		 **/
		virtual void		SetLocation(int x, int y);
		virtual void		SetSize(int width, int height);
		virtual void		SetWidth(int width);
		virtual int			GetWidth()const{ return m_position.GetWidth(); }
		virtual void		SetHeight(int height);
		virtual int			GetHeight()const{ return m_position.GetHeight(); }
		virtual void		SetPosition(int left, int top, int right, int bottom);
		void SetX(int nX);
		int GetX();
		void SetY(int nY);
		int GetY();

		/** reposition the control using the same parameter definition used when control is created.
		* see InitObject() for parameter definition. */
		virtual void		Reposition(const char* alignment, int left, int top, int width, int height);

		/** set the rotation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual void		SetRotation(float fRot);

		/** Get the rotation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual float		GetRotation();

		/** set the scaling of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual void		SetScaling(const Vector2& in);

		/** Get the scaling of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual void		GetScaling(Vector2* pOut);

		/** set the translation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual void		SetTranslation(const Vector2& in);

		/** Get the translation of the control around the center of the UI plus rotation origin offset. it only affects the drawing rect but not the mouse sensor rect.
		* it is usually used for visual effect, so there is no need to update or calculate client rect.*/
		virtual void		GetTranslation(Vector2* pOut);

		/** rotation origin offset from the center of the UI object. */
		virtual void		SetRotOriginOffset(const Vector2& in);

		/** rotation origin offset from the center of the UI object. */
		virtual void		GetRotOriginOffset(Vector2* pOut);

		/** set the color mask of the control. default to 0xffffffff */
		virtual void		SetColorMask(DWORD dwColor);

		/** get the color mask of the control. default to 0xffffffff */
		virtual DWORD		GetColorMask();
		/** apply color mask to child control recursively */
		virtual void ApplyColorMask();

		/** automatically animate the child nodes according to this object's rotation, scaling, translation and color mask values.
		* @param pvOrigin: center of the screen space position. If NULL, it is the center of the current object.
		* @param pdwColor: color mask. if NULL, it is the current object's color mask.
		*/
		virtual void ApplyAnim(const Vector2* pvOrigin = NULL, const float* pfRadian = NULL, const Vector2* pvScaling = NULL, const Vector2* pvTranslation = NULL, const DWORD* pdwColor = NULL);
		virtual void ApplyAnimSelective(const Vector2* pvOrigin = NULL, const float* pfRadian = NULL, const Vector2* pvScaling = NULL, const Vector2* pvTranslation = NULL, const DWORD* pdwColor = NULL);

		/** Spacing between the text and the edge of border */
		virtual void SetSpacing(int nSpacing) { m_nSpacing = nSpacing; m_bNeedUpdate = true; }

		/** Spacing between the text and the edge of border */
		virtual int GetSpacing() { return m_nSpacing; }

		/** padding between the background and content */
		virtual void SetPadding(int nPadding) {}

		/** padding between the background and content */
		virtual int GetPadding() { return 0; }

		/** set the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border.
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		virtual void SetTextScale(float fScale);
		/** get the text scale the text scale, default to 1.f. if we have text scale between 1.1-1.5 and shadow to true, the text will have an alpha border.
		This is great for rendering text in 3d scene with a boarder using just normal system font. */
		virtual float GetTextScale();

		//set location/size/width/height/position without updating the parent's child rect
		virtual void		SetLocationI(int x, int y);
		virtual void		SetSizeI(int width, int height);
		virtual void		SetWidthI(int width);
		virtual void		SetHeightI(int height);
		virtual void		SetPositionI(int left, int top, int right, int bottom);
		virtual void		SetPositionI(const CGUIPosition& position);

		virtual CGUIPosition* GetPosition(){ return &m_position; }

		/** whether the object is drawn by the system or by user provided draw event. default to false, which is drawn by the system. */
		bool IsOwnerDraw() const { return m_bOwnerDraw; }
		void SetOwnerDraw(bool val) { m_bOwnerDraw = val; }

		/**
		 * Set/Get whether the control receives drag
		 **/
		void				SetReceiveDrag(bool bReceiveDrag){ m_bReceiveDrag = bReceiveDrag; };
		bool				GetReceiveDrag()const{ return m_bReceiveDrag; };

		/** Set/Get whether the use text shadow */
		void				SetUseTextShadow(bool bUseTextShadow){ m_bUseTextShadow = bUseTextShadow; };
		bool				GetUseTextShadow() const{ return m_bUseTextShadow; };

		//valied value:0,4,8
		void				SetTextShadowQuality(uint8_t shadowQuality){ m_textShadowQuality = shadowQuality; }
		uint8_t				GetTextShadowQuality() const{ return m_textShadowQuality; };

		void				SetTextShadowColor(uint32_t shadowColor){ m_textShadowColor = shadowColor; }
		uint32_t			GetTextShadowColor()const { return m_textShadowColor; }

		void				SetTextOffsetX(int32_t x){ m_textOffsetX = x; }
		int32_t				GetTextOffsetX()const { return m_textOffsetX; }

		void				SetTextOffsetY(int32_t y){ m_textOffsetY = y; }
		int32_t				GetTextOffsetY()const { return m_textOffsetY; }
		/**
		 * brings the control to the front. The control is moved to the front of the z-order. If the control is a child of another
		 * control, the child control is moved to the front of the z-order. BringToFront does not make a control a top-level control.
		 */
		virtual void		BringToFront();
		/**
		 * Sends the control to the back of the z-order.
		 * The control is moved to the back of the z-order. If the control is a child of another control,
		 * the child control is moved to the back of the z-order
		 **/
		virtual void		SendToBack();

		/** z order affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0. */
		virtual int			GetZOrder()const{ return m_nZOrder; };

		/** z order affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0.
		* if this control has a parent it will cause the parent to sort all children again. */
		void				SetZOrder(int nOrder);

		/** set z depth in the range [0,1], where 0 is closest to screen. if this is -1(default), zdepth is automatically determined. Otherwise it will force the z depth.  */
		void SetZDepth(float fDepth);

		/** get z depth */
		float GetZDepth();

		/**
		 * Set/Get the hotkey
		 **/
		void				SetHotkey(UINT nHotkey) { m_nHotkey = nHotkey; }
		UINT				GetHotkey() const{ return m_nHotkey; }

		/**
		 * Set/Get name of the control
		 **/
		void				SetName(const char* szName);
		const string &		GetName()const{ return m_sIdentifer; };

		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		/**
		* Set/Get cursor file when mouse is over it. If empty, the parent cursor file is used.
		**/
		void				SetCursor(const char* szCursorFile, int nHotSpotX = -1, int nHotSpotY = -1);
		const std::string&		GetCursor(int* pnHotSpotX = 0, int* pnHotSpotY = 0) const;

		/** get id of this object. please note that this is a child id, not a globally unique id.
		* the id is only available when this object is attached to a parent. And the ID will change if this object changes its parent.
		* In all other circumstances, the id uniquely identify this object in its parent. One can call GetChildByID() from its parent control to get this object.
		* _Note_: ID is assigned by its parent when this control is attached to a parent control (or parent changes)
		* it ensures that ChildID is unique among all sibling children of the parent control during the lifetime of the parent.
		*/
		int					GetID()const{ return m_nID; };
		/** this function is used internally. never call this unless you known why. */
		void				SetID(int nID){ m_nID = nID; };

		/** get a child node by its id
		* @param nChildID: child ID usually obtained by GetID() method.
		* @return: return the child object found. it may return NULL if not found.
		*/
		virtual CGUIBase*			GetChildByID(int nChildID){ return NULL; };

		/** get the first child node whose name is name. Since a name may not be unique among its sibling children. One is advised to use GetChildByID() instead.
		* @param name: child name usually obtained by GetName() method.
		* @return: return the child object found. it may return NULL if not found.
		*/
		virtual CGUIBase*			GetChildByName(const string& name){ return NULL; };

		/**
		* save as AttachTo3D, except that the 3D object's name is specified.
		@param s3DObjectName: this must be the name of a global object.
		@remark: The visible property of this GUI object is controlled internally as below:
		- if the 3d object does not exist, the GUI object is invisible.
		- if the 3d object is not visible in the current camera frustum, the GUI object is invisible.
		- if the 3d object is visible in the current camera frustum, the GUI object is visible.
		*/
		void AttachTo3D(const char* s3DObjectName);
		void AttachTo3D(CBaseObject* p3DObject);

		enum EnumTexElementPredefined
		{
			TexElement_White = 0,
			TexElement_Black,
			TexElement_Tranparent,
			TexElement_Last,
		};
		/*get predefined texture elements*/
		static GUITextureElement* GetPredefinedTextureElement(EnumTexElementPredefined nID = TexElement_White);
		static void ClearPredefinedTextureElement();
		/**
		* set visible
		* @param visible
		*/
		virtual void		SetVisible(bool visible);
		/**
		* whether visible
		* @return
		*/
		virtual bool		GetVisible()const{ return m_bIsVisible; }

		/** only return true, if all its parent are visible. */
		virtual bool		GetVisibleRecursive();

		virtual void		SetHighlightStyle(int style){ m_eHighlight = style; }
		virtual int			GetHighlightStyle()const{ return m_eHighlight; }
		/**
		 * Get the texture and font resource of this control
		 **/
		virtual GUITextureElement* GetTextureElement(const char *texturename);
		/** get font element by name*/
		virtual GUIFontElement* GetFontElement(const char *fontname);
		/** get font element by index */
		virtual GUIFontElement* GetFontElement(int nIndex);

		//if you want to erase an event script, use script=NULL;
		void				SetEventScript(int nEvent, const SimpleScript *script);
		const SimpleScript* GetEventScript(int nEvent)const;

		/**
		 * Some controls has scrollbars or other controls bound. This function is used for these
		 * bound controls to tell this control what has changed.
		 **/
		virtual void		UpdateScroll(int nXDelta, int nYDelta){};
		/* only root uses these four functions
		Most controls are designed to be device independent.
		*/
		virtual HRESULT		InitDeviceObjects(){ return 0; };	// device independent
		virtual HRESULT		RestoreDeviceObjects(){ m_bNeedUpdate = true; return 0; }; // device dependent
		virtual HRESULT		InvalidateDeviceObjects(){ return 0; };
		virtual HRESULT		DeleteDeviceObjects(){ return 0; };

		virtual string		ToScript(int option = 0);
		/**
		It is the default object initializing interface between script and the engine. By default, it creates an
		object with the default settings. The classes inherited from base class should override this function.
		*/
		virtual void		InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);

		//gain input focus of the current object
		virtual void		Focus();
		virtual void		LostFocus();
		//some default on events
		/**
		* Default handler of OnFocusIn event
		* OnFocusIn is triggered whenever the control gets input focus.
		* It's called automatically by the default event system.
		*/
		virtual bool		OnFocusIn();

		/**
		* Default handler of OnFocusOut event
		* OnFocusOut is triggered whenever the control losts input focus.
		* It's called automatically by the default event system.
		*/
		virtual bool		OnFocusOut();
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

		/**
		* Default handler of OnMouseLeave event
		* OnMouseWheel event is triggered when the mouse wheel is rotated.
		* @param Delta: delta value of the wheel
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnMouseWheel(int Delta, int X, int Y);

		/**
		* Default handler of OnMouseMove event
		* OnMouseMove event is triggered when the mouse moves on a control.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnMouseMove(int MouseState, int X, int Y);

		/**
		* Default handler of OnMouseHover event
		* OnMouseHover event is triggered when the mouse stays for a while. The default duaration is 0.2 seconds.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnMouseHover(int MouseState, int X, int Y);
		/**
		This is the default OnMouseDown Event handler.
		It constructs a script string and tries to activate it.
		@param MouseState: combination of GUI_MOUSE_STATE
		@param X,Y: position of the mouse;
		@see CGUIEvent::GUI_MOUSE_STATE for the mouse state values.
		*/
		virtual bool		OnMouseDown(int MouseState, int X, int Y);
		/**
		This is the default OnMouseUp Event handler.
		It constructs a script string and activates it.
		@param MouseState: Indicates which mouse button is released. It can be one of the following mouse state values:
		- LBUTTON_UP: Left mouse button is up.
		- MBUTTON_UP: Middle mouse button is up.
		- RBUTTON_UP: Right mouse button is up.
		@param X,Y: position of the mouse;
		@see CGUIEvent::GUI_MOUSE_STATE for the mouse state values.
		*/
		virtual bool		OnMouseUp(int MouseState, int X, int Y);

		/** touch event */
		virtual bool		OnTouch(const TouchEvent& touch);

		/**
		* Default handler of OnModify event.
		* OnModify event is triggered whenever the user changes the contain of edit box, changes position of a slider,
		*/
		virtual bool		OnModify();
		virtual void 		SetModified(){};
		/** Default handler of OnSize event.*/
		virtual bool		OnSize();

		/**
		 * Some controls has select event. such as listbox
		 **/
		virtual bool		OnSelect();

		/**
		* Some controls has confirm event. such as editbox, listbox
		**/
		virtual bool OnChange(const char* code = NULL);


		virtual bool		OnKeyDown();

		virtual bool		OnKeyUp();

		/** this function is called every frame
		* @param fDeltaTime: time elapsed since last frame move.
		*/
		virtual bool		OnFrameMove(float fDeltaTime);

		/**
		* Default OnClick event handler. It activates corresponding scripts.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnClick(int MouseState, int X, int Y);
		/**
		* Default OnDoubleClick event handler. It activates corresponding scripts.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnDoubleClick(int MouseState, int X, int Y);

		virtual bool		OnDragOver(int MouseState, int X, int Y);

		/**
		* The framework calls this member function when a window is being activated or deactivated.
		* @param nState: Specifies whether the window is being activated or deactivated. It can be one of the following values:
		*  - 0  The window is being deactivated.
		*  - 1  The window is being activated by a mouse click.
		*  - 2  The window is being activated through some method other than a mouse click.
		*/
		virtual bool		OnActivate(int nState);

		virtual void MakeActivate(int nState);

		/** called after the control is destroyed. */
		virtual bool		OnDestroy();

		/** get this or the parent object which has WM_ACTIVATE event. May return NULL if not such a parent*/
		CGUIBase* GetWindow();

		/**
		* Default OnDragBegin handler.
		* It does the following:
		* 1. detaches the control and updates its rectangles.
		* 2. updates the dragging information in root and sets the dragging structure to default value
		* 3. activates corresponding scripts.
		* This should be called before the descendant's own handle function.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnDragBegin(int MouseState, int X, int Y);
		/**
		* Default OnDragEnd handler.
		* It does the following:
		* 1. updates the position of the dragging control
		* 2. attaches it to the destination control.
		* 3. Does user defined clean-ups or the default clean-up
		* 4. activates corresponding scripts.
		* This should be called after the descendant's own handle function.
		* @param MouseState: combination of GUI_MOUSE_STATE
		* @param X,Y: position of the mouse;
		*/
		virtual bool		OnDragEnd(int MouseState, int X, int Y);

		//inherited from IObjectDrag
		virtual void		BeginDrag(int nEvent, int x, int y, int z);
		virtual void		EndDrag(int nEvent, int x, int y, int z);

		virtual void		SetCandrag(bool bCandrag){ m_bCandrag = bCandrag; };
		virtual bool		GetCandrag()const{ return m_bCandrag; };

		virtual BOOL		IsOnObject(int x, int y, int z);

		/** get the animation style of this object. Different GUI object may have different style definitions.
		* @return: 0 always means no animation. */
		virtual int GetAnimationStyle(){ return 0; };
		/** set the animation style of this object. Different GUI object may have different style definitions.
		* @param nStyle: 0 always means no animation. */
		virtual void SetAnimationStyle(int nStyle){};

		/** whether this control will leak all events to its parent.
		* Some special (transparent) container may be defined as click through, so that mouse event will leak to 3d. */
		inline bool IsClickThrough(){ return m_bClickThrough; };
		void SetClickThrough(bool bClickThrough);


		/** if true we will enable uv wrapping. uv wrapping is disabled by default. */
		virtual bool IsUVWrappingEnabled();

		/** set whether we will enable uv wrapping. uv wrapping is disabled by default. */
		virtual void EnableUVWrapping(bool bUVWrapping);

		/** this function is called to handle window's latest WM_CHAR message, when the control got key focus. */
		virtual int OnHandleWinMsgChars(const std::wstring& sChars);

		void SendInputMethodEvent(const std::wstring& sChars);

		/** get composition point. default to bottom of the current bounding box. */
		virtual QPoint  GetCompositionPoint();
		virtual void SetCompositionPoint(const QPoint& point);

		virtual bool IsNonClientTestEnabled();
		virtual void EnableNonClientTest(bool val);

		/** whether to use point texture filtering for all ui images rendered. */
		bool GetUsePointTextureFiltering();
		void SetUsePointTextureFiltering(bool bUse);
		/** whether content is dirty, mostly used for self painted object */
		bool IsDirty() const;
		/** if any child is also dirty. */
		bool IsDirtyRecursive();
		void SetDirty(bool val);

		/**
		* return true if it is scrollable.
		* @return
		*/
		virtual bool IsScrollable();

		/** whether it or any of its parent is scrollable or has mouse wheel event handler.
		*/
		bool IsScrollableOrHasMouseWheelRecursive();

		/** if NULL, it means that the object(including all of its child objects) is allowed to paint on any paint device.
		Otherwise, we will only paint this object and its children when the current paint device is same as the one specified.
		*/
		CPaintDevice* GetForcedPaintDevice() const { return m_pForcedPaintDevice; }
		void SetForcedPaintDevice(CPaintDevice* val) { m_pForcedPaintDevice = val; }

		/** whether this object is allowed to paint on the given device. By default, object is allowed to paint on any device.
		unless SetForcedPaintDevice is called.
		*/
		virtual bool CanPaintOnDevice(CPaintDevice* val);

		/** whether this object will paint on its own render target. by default this false. 
		* standard GUI and ownerDraw GUI can coexist on SelfPainted parent device.
		* GUIContainer's "SelfPaint" can be turned on/off anytime anywhere. Once on, all its children including itself is painted using a private render target.
		*/
		void EnableSelfPaint(bool bEnable);
		bool IsSelfPaintEnabled();

		/** whether to automatically clear the background to fully transparent when doing self paint on its own render target. */
		bool IsAutoClearBackground() const;
		void SetAutoClearBackground(bool val);

		/** get GUI painter */
		CPainter* GetPainter(GUIState* pGUIState = NULL);

		/**
		* touch translation for a given GUI container. Default to 0xff, where all possible touch translations are enabled.
		*/
		enum TouchTranslationAttributeFlag
		{
			// long hold to translate to right button operation
			touch_translate_rightbutton = 0x0000001,
			// click and drag to translate to scroll operation when the container is scrollable or has mouse wheel event
			touch_translate_scroll = 0x0000002,
			// some gestures like pinch with two figures to translate to mouse wheel. 
			touch_translate_gestures = 0x0000004,
		};

		uint32_t GetTouchTranslationAttFlag()  const
		{
			return m_touchTranslateAttFlag;
		}

		void SetTouchTranslationAttFlag(uint32_t dwAttr)
		{
			m_touchTranslateAttFlag = dwAttr;
		}

		/** if match any of the given attributes */
		bool IsMatchTouchTranslationAttribute(uint32_t attFlags) const
		{
			return ((m_touchTranslateAttFlag & attFlags) > 0);
		}

		/** all attributes as specified in dwMask must match the value of attFlags*/
		bool IsMatchTouchTranslationAttributes(uint32_t dwMask, uint32_t attFlags) const
		{
			return ((m_touchTranslateAttFlag & dwMask) == attFlags);
		}

		/** if match all of the given attributes */
		bool IsMatchTouchTranslationAttributeAll(uint32_t attFlags) const
		{
			return ((m_touchTranslateAttFlag & attFlags) == attFlags);
		}

	protected:
		/**
		* internally it calls m_resource->DrawSprite. it will apply rotation if necessary..
		* @param prcWindow: the window screen position. If NULL it will retrieve from rect 0.
		*/
		HRESULT DrawElement(GUITextureElement* pElement, RECT *prcDest, RECT* prcWindow = NULL, GUIState* pGUIState = NULL);

		/**
		* internally it calls m_resource->DrawText. it will apply rotation if necessary..
		* @param prcWindow: the window screen position. If NULL it will retrieve from rect 0.
		*/
		HRESULT DrawText(const char16_t* strText, GUIFontElement* pElement, RECT* prcDest, RECT* prcWindow = NULL, bool bShadow = false, int nCount = -1, int shadowQuality = 0, Color shadowColor = 0, GUIState* pGUIState = NULL);

		/** draw a rect of the given color */
		HRESULT DrawRect(Color color, RECT *prcDest, RECT* prcWindow = NULL, GUIState* pGUIState = NULL);

		/** a render target will be created with the same name as this object. */
		virtual CRenderTarget* CreateGetRenderTarget(bool bCreateIfNotExist = true);

		virtual bool IsMouseCaptured();
		virtual void CaptureMouse();
		virtual void ReleaseMouseCapture();
	public:
		CGUIBase(void);
		virtual ~CGUIBase(void);

		//The following four function inherited from the IObject interface.
		/**
		* call this function to safely release this asset. If there is no further reference to this object,
		* it will actually delete itself (with "delete this"). So never keep a pointer to this class after you
		* have released it. A macro like SAFE_RELEASE() is advised to be used.
		*/
		virtual int		Release();
		virtual void		Clone(IObject* pobj)const;
		virtual IObject*	Clone()const;
		virtual bool		Equals(const IObject *obj)const;



		static void InflateRect(RECT* lprc, int dx, int dy);
		static void OffsetRect(RECT* lprc, int dx, int dy);
		static BOOL UnionRect(RECT& lprcDst, const RECT& lprcSrc1, const RECT& lprcSrc2);
		static inline int RectWidth(const RECT &rc) { return ((rc).right - (rc).left); }
		static inline int RectHeight(const RECT &rc) { return ((rc).bottom - (rc).top); }
		static bool RectIntersect(const RECT &rect1, const RECT &rect2);
		static bool IntersectRect(RECT* lprcDst, const RECT &rect1, const RECT &rect2);
		static BOOL IsRectEmpty(const RECT& lprc);
		static void SetRectEmpty(RECT& rc);
		static BOOL PtInRect(const RECT& pos, const POINT &pt);
		static BOOL SetRect(RECT* pRc, int xLeft, int yTop, int xRight, int yBottom);

		/** check if there is any IME string that the user has typed since last call.
		* @return true if there is one.
		*/
		virtual bool FetchIMEString();

		friend class CGUIContainer;
		friend class CGUIRoot;

	public:
		/**
		* Resource object of this control.
		* @see CGUIResource for more details.
		*/
		CGUIResource*		m_objResource;
	protected:
		
		/// parent container
		CGUIContainer*	m_parent;

		/// Virtual key code for this control's hot key
		UINT			m_nHotkey;

		/// global ID of this object. Assigned when object is created. It ensures that ID is unique.
		int				m_nID;

		/** weather we can capture the mouse */
		bool				m_bCanCaptureMouse : 1;

		/// Control can have input focus
		bool				m_bCanHasFocus : 1;

		/// Control currently has focus
		bool				m_bHasFocus : 1;
				
		/// Mouse currently over the control
		bool				m_bMouseOver : 1;

		/// If the control can receive drag
		bool				m_bReceiveDrag : 1;

		/** whether this control will leak all events to its parent.
		* Some special (transparent) container may be defined as click through, so that mouse event will leak to 3d */
		bool				m_bClickThrough : 1;

		/**
		Only when a GUI is enable is it LifeTimeCountDown at work and is sensed.
		This leads to the conclusion that a disabled object is always rendered,
		however it does not receiving any event.*/
		bool				m_bIsEnabled : 1;
		/** whether to use text shadow */
		bool				m_bUseTextShadow : 1;
		/** whether position need update */
		bool				m_bNeedUpdate : 1;
		/** make mouse over to true as long as it contains the mouse point, no matter it has mouse focus or not.*/
		bool				m_bAlwaysMouseOver : 1;
		/// decide if this control is visible. If it's not visible, it cannot receive mouse event, but can receive keyboard event in case it has focus.
		bool				m_bIsVisible : 1;
		/** if true we will enable uv wrapping. uv wrapping is disabled by default. */
		bool				m_bIsUVWrappingEnabled : 1;
		/** default to false for all UI images rendered. */
		bool m_bUsePointTextureFiltering : 1;

		/** whether the object is drawn by the system or by user provided draw event. default to false, which is drawn by the system. */
		bool m_bOwnerDraw : 1;
		/** whether content is dirty. */
		bool m_bDirty : 1;
		/** whether to automatically clear the background to fully transparent when doing self paint on its own render target. */
		bool m_bAutoClearBackground : 1;
		/** whether IME is enabled for this control. default to false. */
		bool m_bInputMethodEnabled : 1;
		
		/** if NULL, it means that the object(including all of its child objects) is allowed to paint on any paint device.
		Otherwise, we will only paint this object and its children when the current paint device is same as the one specified.
		*/
		CPaintDevice* m_pForcedPaintDevice;

		uint8_t m_textShadowQuality;
		uint32_t m_textShadowColor;
		int32_t m_textOffsetX;
		int32_t m_textOffsetY;

		//the only event object
		CGUIEvent*			m_event;
		// bitwise field of TouchTranslationAttributeFlag, default to 0xff.
		uint32_t m_touchTranslateAttFlag;

		/// unit name used in the scripting language
		std::string				m_sIdentifer;
		/// the cursor to be displayed when mouse over it. If this is empty, the parent cursor is used. 
		std::string				m_sCursorFile;
		/** default to -1, which is using current hardware value.  */
		int16	m_nCursorHotSpotX;
		/** default to -1, which is using current hardware value. */
		int16	m_nCursorHotSpotY;

		/// gui position 
		CGUIPosition		m_position;
		/** this affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0. */
		int					m_nZOrder;

		/**
		Number of frames left, before this object is removed from the GUI pool. Set this
		value to negative to make it permanent, unless explicitly deleted.
		*/
		int					m_nLifeTimeCountDown;

		///HighlightStyle Enumeration
		int					m_eHighlight;
		/** rotation around the center of the UI plus rotation offset*/
		float				m_fRotation;
		/** rotation origin offset from the center of the UI object. Only for animation.*/
		Vector2			m_vRotOriginOffset;
		/** scaling, default to 1.f, 1.f. Only for animation.*/
		Vector2			m_vScaling;
		/** translation relative to left, top, default to 0.f, 0.f. Only for animation.*/
		Vector2			m_vTranslation;
		/** the color mask of the control. default to 0xFFFFFFFF. */
		DWORD				m_dwColorMask;

		/** Spacing between the text and the edge of border */
		int			m_nSpacing;

		/** this should be a value in the range [0,1). If it is negative, such as -1.f, this value is then not used. Default to -1.f. */
		float		m_fForceZDepth;

		IType* m_type;
	};

	extern void DeleteGUIObject(CGUIBase * pObj);
}