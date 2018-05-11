#pragma once
#include "GUIBase.h"
#include "GUIContainer.h"
#include "GUIScript.h"
#include "GUIToolTip.h"
#include "SpriteRenderer.h"
#include "util/mutex.h"
#include <vector>
#include <string>
#include <map>
namespace ParaEngine
{
	using namespace std;
	/// @def how many key strokes the GUI can cached before processing
#ifndef KEYBUFFER_SIZE
#define KEYBUFFER_SIZE			15 
#endif
	class CGUIIMEEditBox;
	class CGDIGraphics;
	class CTouchGesturePinch;
	struct  AssetEntity;
	typedef std::vector<MSG> GUIMsgEventList_type;
	struct TouchEvent;
	struct MouseEvent;
	struct AccelerometerEvent;
	class TouchEventSession;

	/**
	* When rendering scene, root scene and the root GUI are rendered in sequence.
	* We see that root scene is still the dominant entity in the ParaEngine. What
	* GUI objects do is making annotations to the scene as we are traversing the
	* scene. Some biped head-up display may cause creation of new GUI objects
	* (with a small life time count down, possibly one frame). Neuron files may
	* also cause the creation of new GUI object, such as the triggering of a dialog
	* neuron file will cause a dialog box screen annotation to be created and a few
	* GUI sensors as buttons in the dialog. A again the triggering of the sensor by
	* mouse click may cause the previous neuron (dialog box )reach a new state,
	* which in turn, determines the response to the user's choice in the dialog box.
	*
	* 2004.5.21:  Key stroke sensor GUI added, the AISimulator will support it automatically.
	* A key buffer is added in CGUIRoot. The ParaShell is responsible to add key to
	* the key buffer by calling AddKeyStroke(), the pick function in CGUIRoot will
	* now handle not only mouse but also key strokes, key triggered object is added
	* to the GUIState of listTriggeredObjects. HAPI of creating such sensor is also added.
	*
	* call AdvanceGUI() to render the GUI
	*/
	class CGUIRoot : public CGUIContainer
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CGUIRoot; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CGUIRoot"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CGUIRoot, GetUIScale_s, Vector2*)	{ float x, y; cls->GetUIScale(&x, &y); p1->x = x; p1->y = y; return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetUIScale_s, Vector2)		{ cls->SetUIScale(p1.x, p1.y); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetBackBufferSize_s, Vector2*)	{ cls->GetBackBufferSize(&(p1->x), &(p1->y)); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetMousePosition_s, Vector2*)	{ int x, y; cls->GetMousePosition(&x, &y); p1->x = (float)x; p1->y = (float)y; return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetMousePosition_s, Vector2)		{ cls->SetMousePosition((int)(p1.x), (int)(p1.y)); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetHasIMEFocus_s, bool*)	{ *p1 = cls->HasIMEFocus(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetHasIMEFocus_s, bool)	{ cls->SetHasIMEFocus(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetEnableIME_s, bool*)	{ *p1 = cls->GetEnableIME(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetEnableIME_s, bool)	{ cls->SetEnableIME(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetUseSystemCursor_s, bool*)	{ *p1 = cls->GetUseSystemCursor(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetUseSystemCursor_s, bool)	{ cls->SetUseSystemCursor(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetFingerSizePixels_s, int*)	{ *p1 = cls->GetFingerSizePixels(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetFingerSizePixels_s, int)	{ cls->SetFingerSizePixels(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, GetFingerStepSizePixels_s, int*)	{ *p1 = cls->GetFingerStepSizePixels(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetFingerStepSizePixels_s, int)	{ cls->SetFingerStepSizePixels(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, SendKeyDownEvent_s, int) { cls->SendKeyDownEvent(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SendKeyUpEvent_s, int) { cls->SendKeyUpEvent(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, IsMouseButtonSwapped_s, bool*) { *p1 = cls->IsMouseButtonSwapped(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetMouseButtonSwapped_s, bool) { cls->SetMouseButtonSwapped(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, SendInputMethodEvent_s, const char*) { cls->SendInputMethodEvent(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, SetMinimumScreenSize_s, Vector2)		{ cls->SetMinimumScreenSize((int)(p1.x), (int)(p1.y)); return S_OK; }
		
		ATTRIBUTE_METHOD1(CGUIRoot, IsMouseCaptured_s, bool*)	{ *p1 = cls->IsMouseCaptured(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetCaptureMouse_s, bool)	{ cls->SetCaptureMouse(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CGUIRoot, IsNonClient_s, bool*)	{ *p1 = cls->IsNonClient(); return S_OK; }
		ATTRIBUTE_METHOD1(CGUIRoot, SetIsNonClient_s, bool)	{ cls->SetIsNonClient(p1); return S_OK; }
	public:
		ParaEngine::GUIState& GetGUIState() { return m_stateGUI; }
		CDirectMouse* GetMouse() { return m_pMouse; }
		CDirectKeyboard* GetKeyboard() { return m_pKeyboard; }
		/** set whether we will capture the mouse for the main window. */
		void SetCaptureMouse(bool bCapture);
		bool IsMouseCaptured();
		
		bool PushEvent(const MSG& msg);
		bool handleTouchEvent(const TouchEvent& touch);
		bool handleNonClientTest(const MouseEvent& mouseEvent);
		bool handleAccelerometerEvent(const AccelerometerEvent& accelerator);
		bool handleGesturePinch(CTouchGesturePinch& pinch_gesture);
		void TranslateMousePointInTouchEvent(TouchEvent &touch);

		/** set the UI scaling. This can be useful to render 1024*768 to a 800*600 surface; we can set to fScalingX to 800/1024 and fScalingY to 600/768
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called.
		* @param fScalingX: x defaults to 1.0
		* @param fScalingY: y defaults to 1.0
		* @param bEnsureMinimumScreenSize: if true, we will readjust UI scaling so that minimum screen size is ensured.
		*/
		void SetUIScale(float fScalingX, float fScalingY, bool bEnsureMinimumScreenSize = true, bool bNotifySizeChange = true);

		/** set the UI scaling. This can be useful to render 1024*768 to a 800*600 surface; we can set to fScalingX to 800/1024 and fScalingY to 600/768
		* @param pfScalingX: out put x scale
		* @param pfScalingY: out put y scale
		*/
		void GetUIScale(float* pfScalingX = NULL, float* pfScalingY = NULL);

		/** get back buffer size*/
		void GetBackBufferSize(float* pWidth, float* pHeight);

		/** the minimum screen size. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		* calling this function will cause OnSize() and UpdateBackbufferSize() to be called. Actually it calls SetUIScale()
		* @param nWidth: the new width.
		* @param nHeight: the new height.
		* @param bAutoUIScaling: default to true. whether we will automatically recalculate the UI scaling accordingly with regard to current backbuffer size.
		*/
		void SetMinimumScreenSize(int nWidth, int nHeight, bool bAutoUIScaling = true);


		/** update backbuffer size, in case it is changed. we will automatically multiple backbuffer size using UI scale to obtain the actual size of GUI state
		* @return true if size has changed.
		*/
		bool UpdateViewport(int nLeft, int nTop, int nWidth, int nHeight, bool bForceUpdate = false);

		/**
		* make the given control the current top level control. if the given control is already a top level control but it not on  level 0, it is moved to level 0.
		* set top level control
		* @param pTopLevelControl: make the given control the current top level control
		*/
		void PushTopLevelControl(CGUIContainer* pTopLevelControl);
		/**
		* Remove the given top level control and make all top level controls below this control one level up.
		* @param pTopLevelControl:
		*/
		void RemoveTopLevelControl(CGUIContainer* pTopLevelControl);

		/** clear all top level controls. possibly only used for GUI resetting.*/
		void ClearAllTopLevelControls();

		/**
		* get the top level control at level 0
		*/
		CGUIContainer* GetTopLevelControl(){ return m_TopLevelCtrls[0]; };

		CGUIBase* GetIMEFocus() const;
		void SetIMEFocus(CGUIBase* val);

		/**
		* Delete a given object according to its type
		* It's a static function.
		* @param pObj: pointer to the object.
		* @remark: Users need to make sure the object is not referenced in other place. We don't set the pointer to NULL.
		*/
		static void DeleteGUIObject(CGUIBase * pObj);

		static void PostDeleteGUIObject(CGUIBase * pObj);
		/**
		* Attach a control to another control.
		* @param pParent: pointer to the parent control. If it is NULL, the child will be attached to the root.
		* @param pChild: pointer to the child control. If it is NULL, nothing is done.
		*/
		void AttachGUIElement(CGUIBase* pParent, CGUIBase* pChild);

		/**
		* Detach a control from its parent.
		* This function automatically cleans all focus related to pChild and updates rectangles of its parent and itself.
		* @param pChild: pointer to the child control. If it is NULL, nothing is done.
		*/
		void DetachGUIElement(CGUIBase* pChild);

		/**
		* Destroy destroy a control and all its descendants from the GUI.
		* @param pObj: pointer to the control to delete. If it is NULL, nothing is done.
		* @return: always return 0.
		*/
		int DestroyGUIElement(CGUIBase* pObj);

		/**
		* Destroy destroy a control with the given name and all its descendants from the GUI.
		* If there are several objects who have the same id, only the last attached one is deleted.
		* @param sID: the name of the control to delete.
		* @return: return 0 if succeed otherwise -1(object not found).
		*/
		int DestroyGUIElement(const char* sID);

		/**
		* Get the first control with the given name
		* @param strObjectName: the name of the control. If this is "root", the root object is returned.
		* @return: the control with the given name. return NULL if not find.
		*/
		CGUIBase* GetUIObject(const char * strObjectName);
		CGUIBase* GetUIObject(const std::string& strObjectName);

		/**
		* get UI object by its ID.
		* @param nID: this is id property value or result of GetID() from a UI object.  If this is 0, the root object is returned.
		* @return: the control with the given id. return NULL if not find.
		*/
		CGUIBase* GetUIObject(int nID);

		/**
		* Gets the control at a given point
		* It finds the top-most control in all controls. It's not the same as GetObjectAtPoint, which finds the control in the direct children of the container
		* @param x,y: absolute coordinates.
		* @remark: It's different from the GetObjectAtPoint() in CGUIContainer.
		*/
		CGUIBase* GetUIObject(int x, int y);

		/**
		* get the default template object from which all sub-sequent controls of the same type are cloned(created).
		* if the default template object does not exist, it will be deleted.
		* one can modify the template object at runtime to change of the theme of all controls created subsequently.
		* All default template object is defined in "script/config.lua", which will be loaded when GUI engine is loaded
		* @param strType: It can be "container", "button", "scrollbar", "editbox", "imeeditbox","slider" and "text".
		* @return the template object is returned. or NULL if does not exists.
		*/
		CGUIBase* GetDefaultObject(const char *strType);

		/**
		* Gets the key focus of all controls. It searches all its descendants.
		* @return: a pointer to the focused control
		* @remark: It's different from the GetKeyFocus() in CGUIContainer.
		*/
		CGUIBase* GetUIKeyFocus();

		/** set UI key focus to a given control. It will automatically refresh the mouse focus hierachy. */
		void SetUIKeyFocus(CGUIBase* control);

		/**
		* Gets the mouse focus of all controls. It searches all its descendant.
		* @return: a pointer to the focused control
		* @remark: It's different from the GetMouseFocus() in CGUIContainer.
		*/
		CGUIBase* GetUIMouseFocus();

		/** set UI mouse focus to a given control. It will automatically refresh the mouse focus hierachy. */
		void SetUIMouseFocus(CGUIBase* control);

		/** set whether to use default mouse cursor or not.*/
		void UseDefaultMouseCursor(bool bUseDefaultMouseCursor);

		/**
		* Dispatch the current events in m_objEvents to its correct receiver. The deeper
		* a control in the hierarchy tree is, the higher priority it has for receiving
		* the event.
		* The two types of events, Mouse event and Keyboard event, are dispatched separately.
		* If there are more than one events for each type. The last one will be dispatched.
		* Return:
		* The successfully dispatched events are deleted from m_objEvents.
		* return value tells which kind of event have been successfully dispatched.
		*/
		int	HandleUserInput();

		/**
		* send a simulated raw key event to ParaEngine.
		* @param nVirtualkey: should be of type EVirtualkey
		*/
		void SendKeyDownEvent(int nVirtualkey);
		void SendKeyUpEvent(int nVirtualkey);
		void SendInputMethodEvent(const char* pStr);

		/** swap left/right mouse button and touch.*/
		bool IsMouseButtonSwapped();
		void SetMouseButtonSwapped(bool bSwapped);

		bool DispatchKeyboardMsg(bool bKeyHandled);

		void DispatchTouchMouseEvent(bool &bMouseHandled);

		//virtual bool MsgProc(CGUIEvent *event=NULL);
		virtual bool MsgProc(MSG *event);
		/**
		* Resets all objects.
		*/
		virtual void Reset();

		/**
		* Clear all the control's children
		*/
		virtual void DestroyChildren();

		/**
		* Delete all contains of the root object
		*/
		void Clear();
		/**
		* Initialize the root object
		*/
		void Initialize();
		/**
		* Render all the contorls
		*/
		void	AdvanceGUI(float fElapsedTime);


		void	AddScript();

		HRESULT OneTimeGUIInit();
		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool &bNoFurtherProcess);

		virtual HRESULT Render(GUIState* pGUIState, float fElapsedTime);
		virtual bool OnClick(int MouseState, int X, int Y);

		virtual HRESULT InitDeviceObjects();	// device independent
		HRESULT RestoreDeviceObjects(UINT nBkbufWidth, UINT nBkbufHeight); // device dependent
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual int	Release();
		virtual string ToScript(int option = 0);

		//to set whether the mouse cursor is in the client area of the program
		void SetMouseInClient(bool bMouseInClient);
		bool GetMouseInClient();
		/** just in case the cursor is not loaded successfully in previous frame*/
		bool CheckLoadCursor();

		///Activates the GUI Root object
		void ActivateRoot();
		///Inactivates the GUI Root object
		void InactivateRoot();
		/** whether the GUI is active or not.
		*/
		bool IsActive();

		bool IsKeyboardProcessed();
		bool IsMouseProcessed();

		/** whether the mouse is in non-client area, we will allow the user to drag the window if this is true.
		* CGUIContainer:SetNonClientArea(true) can be used to specify a non-client area.
		*/
		bool IsNonClient() const;
		void SetIsNonClient(bool val);

		/**
		* get the current mouse cursor position
		* @param nX : in pixels
		* @param nY : in pixels
		*/
		void GetMousePosition(int* nX, int* nY) const;

		void SetMousePosition(int nX, int nY);

		/** touch finger size in pixels. we will automatically click a button when it is within finger size. */
		int GetFingerSizePixels() const;
		void SetFingerSizePixels(int val);
		/** the minimum UI size to test */
		int GetFingerStepSizePixels() const;
		void SetFingerStepSizePixels(int val);

		Matrix4* Get3DViewProjMatrix(){ return &m_matViewProj; };
		void Set3DViewProjMatrix(const Matrix4& mat){ m_matViewProj = mat; };
#ifdef USE_DIRECTX_RENDERER
		DEVMODE *GetDisplayMode(){return &m_displaymode;}
#endif
		/** Never call this unless you know why: Add an object to name map */
		void AddToNameMap(const string& name, CGUIBase* pObj);
		/** Never call this unless you know why: Add an object to id map */
		void AddToIDMap(int nID, CGUIBase* pObj);

		std::map <string, CGUIBase*> m_namemap;
		std::map <int, CGUIBase*> m_idmap;

		//some static function and variables
		static CGUIRoot* GetInstance();
		static CGUIRoot* CreateInstance();
		static void DeleteInstance(CGUIRoot* pThis = NULL);

		/** render the image cursor. this is automatically enabled during movie recording.*/
		void SetRenderImageCursor(bool bEnable);

		/** whether to use system cursor. If this is true, d3d hardware cursor is not shown, even you have loaded an cursor icon for it.*/
		void SetUseSystemCursor(bool bUseSystem);

		/** get whether to use system cursor*/
		bool GetUseSystemCursor();

		virtual const IType* GetType()const{ return m_type; }

		/** [thread safe] whether there is a visible IME virtual CGUIIMEEditBox that has key focus. */
		bool HasIMEFocus();
		void SetHasIMEFocus(bool bHasFocus);

		/** whether to use directX rendered ime windows. in most cases during windowed mode, ime window can be disabled.*/
		bool GetEnableIME();
		void SetEnableIME(bool bHasIME);

		/** confine the cursor in current window.
		* @param bEnable: true to enable and false to release.
		*/
		void EnableClipCursor(bool bEnable);

		/** whether the cursor is clipped. */
		bool IsCursorClipped();

		CGUIBase* GetActiveWindow() const;
		void SetActiveWindow(CGUIBase* val);
	public:
		virtual ~CGUIRoot(void);

		//
		// interface of PaintDevice
		//
		virtual CPaintEngine * paintEngine() const;
		virtual int metric(PaintDeviceMetric metric) const;
		virtual float GetUIScalingX() const;
		virtual float GetUIScalingY() const;
		virtual float GetViewportLeft() const;
		virtual float GetViewportTop() const;

		/* get painter with default device. mostly used for text size calculation, etc. */
		CPainter* GetPainter();
	private:
		/** update the visibility of 3D object. All 3D objects are assumed to be attached to the root object.
		- if the 3d object does not exist, the GUI object is invisible.
		- if the 3d object is not visible in the current camera frustum, the GUI object is invisible.
		- if the 3d object is visible in the current camera frustum, the GUI object is visible.
		*/
		void Update3DObject(float fElapsedTime);

		/** update logical cursor position after UI scaling and backbuffer scaling are applied. */
		void UpdateCursorPosition();

		void TranslateMousePos(int &inout_x, int &inout_y);

		void TranslateTouchEvent(const TouchEvent &touch);

		/** we will simulate finger size, by testing in a 44*44 region around the current touch point. 
		*/
		bool AutoLocateTouchClick(int ui_mouse_x, int ui_mouse_y, int &mouse_x, int &mouse_y);

	public:
		static DWORD WheelScrollLines;
		static DWORD KeyDelay;
		static DWORD KeyRepeat;
		static DWORD MouseDelay;
		static DWORD MouseRepeat;
		static DWORD MouseHover;
		static int MouseThreshold1;
		static int MouseThreshold2;
		static DWORD MouseAcceleration;
		static float MouseSpeed;

		/** GUI root's mouse events. Unlike control events, the root event can have
		* multiple event handlers defined outside. The events will be emptied every GUI cycle.
		Added LXZ: 2006.1.1 */
		//shall delete
		//list <MouseEvent> m_MouseEvents;

		GUIMsgEventList_type m_events;

		CDirectKeyboard*		m_pKeyboard;

		CDirectMouse*			m_pMouse;

		//	GUI_DRAG_AND_DROP		m_pDragObj;

		CGUIBase*			m_IMEFocus;


		CGUIScript				m_scripts;

		bool					m_bDesign;
		///if the gui is active
		bool					m_bActive;
		CGUIToolTip*			m_tooltip;

		/// the top level controls in a array, m_TopCtrl[0] always refers to the top level, m_TopCtrl[1] refers to the top level control below m_TopCtrl[0], etc.
		vector <CGUIContainer*> m_TopLevelCtrls;

	protected:
		static const IType* m_type;
		CGUIRoot(void);

		mutable CPaintEngine *engine;
		bool m_bIsCursorClipped;
		RECT m_rcOldClipRect;
		bool m_bMouseProcessed;
		bool m_bKeyboardProcessed;
		bool m_bMouseCaptured;
		/** please note this can be the 3d scene object. */
		IAttributeFields* m_pLastMouseDownObject;
#ifdef USE_DIRECTX_RENDERER
		DEVMODE m_displaymode;
#endif
		Matrix4 m_matViewProj;
		bool m_bMouseInClient;
		bool m_bWindowChanging;//when window moved or resize, it is set true;
		bool m_bRenderImageCursor;
		bool m_bUseSystemCursor;
		// delete queue id
		vector<int> m_deleteQueue;

		float m_fUIScalingX;
		float m_fUIScalingY;


		float m_fViewportLeft;
		float m_fViewportTop;
		float m_fViewportWidth;
		float m_fViewportHeight;

		/** the minimum screen width. if the backbuffer is smaller than this, we will use automatically use UI scaling
		* for example, if minimum width is 1024, and backbuffer it 800, then m_fUIScalingX will be automatically set to 1024/800.
		*/
		float m_fMinScreenWidth;

		/** the minimum screen height. if the backbuffer is smaller than this, we will use automatically use UI scaling */
		float m_fMinScreenHeight;

		/** whether there is a visible IME virtual CGUIIMEEditBox that has key focus. */
		bool m_bHasIMEFocus;
		/** whether the mouse is in non-client area, we will allow the user to drag the window if this is true. 
		* CGUIContainer:SetNonClientArea(true) can be used to specify a non-client area. 
		*/
		bool m_bIsNonClient;
		/** touch finger size in pixels. we will automatically click a button when it is within finger size. */
		int m_nFingerSizePixels;
		int m_nFingerStepSizePixels;
		/** last touch position in mobile version. */
		int m_nLastTouchX, m_nLastTouchY;
		/** mapping from touch id to ui object. */
		std::map<int, int> m_touch_id_to_ui_obj;

		/// mostly for the ime focus. 
		ParaEngine::mutex m_mutex;
		CPainter* m_pPainter;
		GUIState    m_stateGUI;
		/** last active GUI object object who has onactivate event handler in script. */
		CGUIBase*  m_pActiveWindow;
		
		friend CGUIBase;
	};
}