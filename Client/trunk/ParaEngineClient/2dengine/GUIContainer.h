#pragma once

#include "GUIBase.h"
#include <list>

namespace ParaEngine
{
	class CGUIButton;
	class CGUIScrollBar;
	struct GUITextureElement;

	/**
	m_objResource->m_objArtwork->DrawingRects[0]: client rectangle;
	m_objResource->m_objArtwork->DrawingRects[1]: container rectangle, including scrollbars
	m_objResource->m_objArtwork->DrawingRects[2]: container rectangle, including scrollbars, including borders
	m_objResource->m_objArtwork->DrawingRects[3-11]: [optional] for nine element containers.
	*/
	class CGUIContainer: public CGUIBase
	{
	public:
		enum GUI_POPUP_STYLE{
			Popup_None=0,
			Popup_Autohide=1,
			Popup_Autodelete=2
		};

		CGUIContainer();
		virtual ~CGUIContainer();
		ATTRIBUTE_DEFINE_CLASS(CGUIContainer);

		virtual int GetChildCount();

		/**
		* Gets the control's children
		* @return: a pointer to the children list;
		*/
		virtual GUIBase_List_Type*		GetChildren(){ return &m_children;};

		/** get a child node by its id 
		* @param nChildID: child ID usually obtained by GetID() method. 
		* @return: return the child object found. it may return NULL if not found. 
		*/
		virtual CGUIBase*			GetChildByID(int nChildID);
		
		/** get the first child node whose name is name. Since a name may not be unique among its sibling children. One is advised to use GetChildByID() instead. 
		* @param name: child name usually obtained by GetName() method. 
		* @return: return the child object found. it may return NULL if not found. 
		*/
		virtual CGUIBase*			GetChildByName(const string& name);

		/**
		* add a child object. Please note that this function does not check if the child is already a children of this node. 
		* @param pChild: the child object to attach. 
		* @param nIndex: if this is -1, child is attached to end of the child list. 
		*/
		void AddChild(CGUIBase* pChild, int nIndex = -1);

		/**
		* Clear all the control's children
		*/
		void					DestroyChildren();

		/**
		* Sort children by z-order. it will just move child in min distance to make them in order. 
		* @return true if at least one child's position is changed. 
		*/
		bool					SortChildrenByZOrder();

		/** z order affect the order of rendering. The parent control sort and render child control from small z value to larger z value. default value is 0. 
		* if this is a top level control it will add 1000 to zorder. 
		*/
		virtual int			GetZOrder()const;

		/** set the 3D depth. this is usually automatically set when object is attached to 3d object. 
		* it the object contains children. it will also affect all children automatically. 
		* @param fDepth: a value of [0,1).  Default to 0, which is closest to the screen.
		*/
		virtual void	SetDepth(float fDepth);

		/**
		* Set key focus of this control. 
		* It will also change all related key focus and triggers OnFocusIn and OnFocusOut event
		* @param control: the new focused control
		*/
		void					SetKeyFocus(CGUIBase* control);
		/**
		* Set mouse focus of this control. 
		* It will also change all related mouse focus and triggers OnMouseEnter and OnMouseLeave event
		* @param control: the new focused control
		*/
		void					SetMouseFocus(CGUIBase* control);

		/**
		* Gets the direct key focus of the current control. It searches its children but not all its descendant.
		* @return: a pointer to the focused control
		* @remark: It's different from the GetUIKeyFocus() in CGUIRoot. 
		*/
		virtual CGUIBase*		GetKeyFocus(){return m_pKeyFocus;};
		/**
		* Gets the direct mouse focus of the current control. It searches its children but not all its descendant.
		* @return: a pointer to the focused control
		* @remark: It's different from the GetUIMouseFocus() in CGUIRoot. 
		*/
		virtual CGUIBase*		GetMouseFocus(){return m_pMouseFocus;}
		/**
		* Get the scroll type of the control
		* @return: a combination of GUI_SCROLL_TYPE
		* @see GUI_SCROLL_TYPE for more details
		*/
		int						GetScrollType();

		/**
		* Set the scroll type of the control
		* @param etype: a combination of GUI_SCROLL_TYPE
		* @see GUI_SCROLL_TYPE for more details
		*/
		void					SetScrollType(int etype);

		/**
		* return true if it is scrollable.
		* @return 
		*/
		virtual bool IsScrollable();

		/**
		* set scrollable
		* @param bScrollable 
		*/
		void SetScrollable(bool bScrollable);

		/**
		* Reset the scroll bar. 
		*/
		void ResetScrollbar();

		/**
		* Gets the direct object at the given point. It searches its first level children but not all its descendant.
		* Please note that click-through object are also returned. Disabled and invisible controls are ignored. 
		* @return: a pointer to the control(maybe this)
		* @remark: It's different from the GetUIObjectAtPoint() in CGUIRoot. 
		*/
		virtual CGUIBase*		GetObjectAtPoint(int x,int y);
		/** 
		* Gets the deepest non-click-through object at the given point. It searches all of its children recursively.
		* @return: a pointer to the control(maybe this)
		*/
		virtual CGUIBase*		GetObjectAtPointRecursive(int x,int y);
		/**
		* Render a mask of the container to ensure none of its descendants is rendered outside its boundary.
		* Please note that click-through object are NOT returned. Disabled and invisible controls are ignored. 
		* @param pGUIState: a pointer to the GUIState object;
		* @param rcWindow: the rectangle which all drawings should lie inside it
		* @return: return S_OK if success;
		*/
		//virtual HRESULT			RenderMask(GUIState* pGUIState, RECT rcWindow );

		/**
		* Do some preparations before render. 
		* @param pGUIState: a pointer to the GUIState object;
		*/
		virtual void			Begin(GUIState* pGUIState,float fElapsedTime);

		/**
		* Render function of the control. 
		* @param pGUIState: a pointer to the GUIState object;
		* @return: return S_OK if success;
		*/
		virtual HRESULT			Render(GUIState* pGUIState ,float fElapsedTime);

		/**
		* Do some fininsh-ups after render. 
		* @param pGUIState: a pointer to the GUIState object;
		*/
		virtual void			End(GUIState* pGUIState,float fElapsedTime);
		
		virtual bool			OnFocusOut();
		void					SetFastRender(bool fastrender){m_bFastRender=fastrender;}
		bool					GetFastRender(){return m_bFastRender;}

		/**
		* Tell the container how many items are scrolled in both X and Y direction. 
		* It's usually called by its scroll bars in order to update the container's information.
		* @param nXDelta: The horizontal scroll bar scrolls by nXDelta items (plus or minus) 
		* @param nYDelta: The vertical scroll bar scrolls by nYDelta items (plus or minus) 
		*/
		virtual void			UpdateScroll(int nXDelta,int nYDelta);
		virtual void			UpdateRects();
		/** adds a rectangle to the specified window's update region. The update region represents the portion of the 
		* window's client(child) area that must be recalculated and redrawn 
		* @param lpRect:[in] Pointer to a RECT structure that contains the client coordinates of the rectangle to be added to 
		* the update region. If this parameter is NULL, the entire client(child) area is added to the update region. 
		* @return true if succeed.
		*/
		virtual bool InvalidateRect(const RECT* lpRect);
		void					UpdateScrollSize();
		/**
		* this function will update the client rectangle of this object.
		* Usually a child object need to inform a parent object to update its client area, whenever a child object size changes.
		* if a container's child rectangle is bigger than the container itself, scroll bar may be displayed.
		* @param pos the position of the child object specified in any coordinate system.
		*/
		virtual void			UpdateClientRect(const CGUIPosition& pos,bool unupdate=false);

		virtual string			ToScript(int option=0);
		virtual void			InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		virtual HRESULT			RestoreDeviceObjects();; // device dependent
		virtual bool			MsgProc(MSG *event);

		virtual void			NextKeyFocus();

		/** click the default button inside this container. If no default button is found, this function will return false. */
		virtual bool			ActivateDefaultButton();

		/** get the default button inside this container. If no default button is found, this function will return NULL. */
		virtual CGUIButton*		GetDefaultButton();

 		/**
		* Get the texture element.
		* This is an overloaded function to provide container class specific functionality. 
		* @param texturename: name of texture. Extra textures are support here. They are "vscrollbar.track", "vscrollbar.upbutton",
		* "vscrollbar.downbutton", "vscrollbar.thumb", "hscrollbar.track", "hscrollbar.leftbutton",
		* "hscrollbar.rightbutton", "hscrollbar.thumb"
		*/
		virtual GUITextureElement* GetTextureElement(const char *texturename);

		void					SetScrollbarWidth(int width);
		int						GetScrollbarWidth()const{return m_nSBWidth;}
		void					Clone(IObject* pobj)const;
		IObject					*Clone()const;
		virtual const IType* GetType()const{return m_type;}

		static void				StaticInit();
		
		/**
		 * Bring the obj to the front of z-order if the obj is child of this container          
		 **/
		virtual void			BringToFront(CGUIBase* obj);
		/**
		* Send the obj to the back of z-order if the obj is child of this container          
		**/
		virtual void			SendToBack(CGUIBase* obj);

		/**
		 * If set invisible, the control will make itself not on top.          
		 **/
		virtual void			SetVisible(bool visible);
		/**
		 * If set dragable, the control will make itself not on top.                 
		 **/
		virtual void			SetCandrag(bool bCandrag);
		/**
		* Sets the container as the top-level control.
		* If you call the SetTopLevel method of a container and pass in a value of true, the container will be the top container 
		* which receive all messages. You can only pass a value of true when the container is attached to root. After you pass a 
		* value of false, the container will behave as a normal container.
		* @remark: If the container is the top-level control, set it invisible or set it dragable will make it no longer the top-level control.
		*  If CanDrag or visible property is false, you can't set it top-level control.
		* @param value: true to set the control as the top-level control; otherwise, false
		**/
		virtual void			SetTopLevel(bool value);
		virtual bool			GetTopLevel()const{return m_bIsTop;}

		//obsolete
		virtual void			SetPopUp(int popup);
		//obsolete
		virtual int				GetPopUp()const{return m_nPopupStyle;}

		virtual void			BeginUpdate(){m_bBatching=true;};
		virtual void			EndUpdate(){m_bBatching=false;};
		
		/** get composition point. default to bottom of the current bounding box. */
		virtual QPoint  GetCompositionPoint();

		/** if set to (0,0) it will use the bounding bottom. */
		virtual void SetCompositionPoint(const QPoint& point);

		/** whether this object is allowed to paint on the given device. By default, object is allowed to paint on any device.
		unless SetForcedPaintDevice is called.
		*/
		virtual bool CanPaintOnDevice(CPaintDevice* val);

		/**
		* get the vertical or horizontal scroll bar. 
		* @param nVerticalHorizontal : if 0, vertical scroll bar is returned; if 1 horizontal scroll bar object is returned.
		* @return scroll bar object linked to this object. It may be NULL.
		*/
		CGUIScrollBar*			GetScrollBar(int nVerticalHorizontal=0);

		/** get the index of the given child. */
		int GetChildIndex(CGUIBase* pChild);

		virtual bool IsNonClientTestEnabled();
		virtual void EnableNonClientTest(bool val);

	protected:
		/** a render target will be created with the same name as this object. */
		virtual CRenderTarget* CreateGetRenderTarget(bool bCreateIfNotExist = true);

	protected:
		/// whether it can automatically scroll
		bool					m_bScrollable;
		static const IType* m_type;
		/// Vertical scrollbar
		CGUIScrollBar*		m_VScroll;
		/// Horizontal scrollbar
		CGUIScrollBar*		m_HScroll;
		/// child object
		GUIBase_List_Type	m_children;
		/// one of its children control which has Key focus
		CGUIBase*			m_pKeyFocus;
		/// one of its children control where the mouse located
		CGUIBase*			m_pMouseFocus;
		/// Scroll type
		int					m_ScrollType;
		/** only used when self paint is enabled, when we will render into our own render target. */
		WeakPtr m_renderTarget;

		//vector<CGUIBase*>	m_objDrawingControls;
		//int					m_nRefValue;	//stores it's own reference value for stencil buffer
		//DWORD				tempRef;
		int					m_nBorder;
		int					m_nMargin;
		int					m_nSBWidth;	//width of the scrollbars
		bool				m_bFastRender;//whether to speed up rendering
		RECT				m_oldRect;//the old rectangle of the scissor rectangle
		RECT				m_clientRect;//the rect that contains all children
		QPoint				m_compositionPoint;
		int					m_nPopupStyle;//obsolete
		bool				m_bIsTop;
		bool				m_bBatching;//if a Batching operation is on the way
		bool				m_bNeedCalClientRect;//if the child rect needs to be recalculate
		/** default to false. if true, we will set GUIRoot's non-client attribute. */
		bool				m_bEnableNonClientTest;
		friend class CGUIRoot;
	};
}
