//----------------------------------------------------------------------
// Interface:	IObjectDrag
// Authors:	Liu Weili
// Date:	2006.4.5
// Revised:	by LiXizhi 2007.11.4
// desc: 
// This interface is intended to provide a common interface for all objects
// which are intended to be dragged in some way. 
// We don't need to know the actual type of the objects through this interface.
//-----------------------------------------------------------------------

#pragma once
#include <string>
namespace ParaEngine
{
	using std::vector;
	class IObjectDrag;
	struct CGUIPosition;
	/*
	This is a structure for drag-and-drop operation.
	It stores necessary information for a dragging operation.
	*/
	struct STRUCT_DRAG_AND_DROP {
		IObjectDrag* pSource;			//where the drag begin, the dragging object's parent
		IObjectDrag* pDestination;		//where to drop
		IObjectDrag* pDragging;		//the dragging object which moves with the mouse cursor

		int nEvent;			//which event triggers the dragging. If mouse,that is which button is pressed.
		int startX, startY; // the absolute screen position when the drag start. 
		int nRelativeX,nRelativeY,nRelativeZ;//relative position of the dragging control's top-left point with respect to  
		// this is used to restore an UI object to its original position. 
		CGUIPosition*		m_pOldPosition;
		// if true, the pDragging is only a dragging candidates and is expect to receive all mouse event if the button is down.
		bool m_bIsCandicateOnly;
		//the mouse position when it starts dragging.
		void (*pCleanUp)();			//call back function when dragging is over in order to do clean up jobs 
		//such as deleting the dragging object or change state of other objects.
		void init();

	public:
		/** set old position */
		void SetOldPosition(const CGUIPosition& vPos);

		/** Get old position */
		CGUIPosition* GetOldPosition();

		/** we should clean up receivers, when a drag begins. However, the drag begin NPL script handlers, one can add receiver names.*/
		void CleanUpReceivers();

		/** add an receiver to the current receiver list.
		* @param sName: name. if this is "root", the dragging object can always to reattached. 
		*/
		void AddReceiver(const char* sName);

		/** whether the current receiver list contains a given name.*/
		bool HasReceiver(const char* sName);

		/** a dragging candidate will receive all mouse event if the mouse button keeps pressed. */
		void SetDraggingCandidate(IObjectDrag* pDragging);
		void UnsetDraggingCandidate();
	private:
		/** the receiver names that can always receives drag.*/
		vector <string> m_receiverNames;
	};

	class IObjectDrag
	{
	public:
		IObjectDrag():m_bCandrag(false) {};
		virtual void SetCandrag(bool bCandrag)=0;
		virtual bool GetCandrag()const=0;
		/**
		 * Default BeginDrag handle function
		 * @param x, y, z: position of where the drag begins 
		 *  x, y, z are screen coordinates. In most cases, z is zero.
		 * @param nEvent: tells which event triggers the begin drag. 
		 * This value will set to the STRUCT_DRAG_AND_DROP::nEvent
		 **/
		virtual void BeginDrag(int nEvent, int x, int y, int z)=0;
		/**
		 * Default EndDrag handle function
		 * @param x, y, z: position of where the drag begins 
		 *  x, y, z are screen coordinates. In most cases, z is zero.
		 * @param nEvent: tells which event triggers the end drag. 
		 * You may test this value against the STRUCT_DRAG_AND_DROP::nEvent 
		 * to see if this end drag is acceptable.
		 **/
		virtual void EndDrag(int nEvent, int x, int y, int z)=0;
		/**
		 * Decide whether the input position is on this object
		 * This function is to help other function to decide if the 
		 * currently pointing position is logically "on" the object.
		 * @param x, y, z: position of where the drag begins 
		 *  x, y, z are screen coordinates. In most cases, z is zero.
		 **/
		virtual BOOL IsOnObject(int x,int y, int z)=0;

		/** restore to a given position. */
		virtual void SetPositionI(const CGUIPosition& position){};

		/**
		 * This structure contains the information about the dragging object.
		 **/
		static STRUCT_DRAG_AND_DROP DraggingObject;

		/** try cancel the drag operation. return true if the drag operation is successfully canceled 
		* to its source location where the drag begins. */
		static bool CancelDrag(STRUCT_DRAG_AND_DROP* pdrag);
	protected:
		bool		m_bCandrag;
	};
}
