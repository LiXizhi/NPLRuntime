#pragma once
#include "GUIBase.h"

// Minimum scroll bar thumb size
#define SCROLLBAR_MINTHUMBSIZE 8

namespace ParaEngine
{
	enum GUI_SCROLL_TYPE{
		NONE = 0,
		HSCROLL = 1,
		VSCROLL = 2
	};

	/** mostly associated with a CGUIContainer
	m_objResource->m_objArtwork->DrawingRects[0]: track rectangle;
	m_objResource->m_objArtwork->DrawingRects[1]: up/left button rectangle;
	m_objResource->m_objArtwork->DrawingRects[2]: down/right button rectangle;
	m_objResource->m_objArtwork->DrawingRects[3]: thumb button rectangle;
	*/
	class CGUIScrollBar : public CGUIBase
	{
	public:
		CGUIScrollBar();
		virtual ~CGUIScrollBar();

		//virtual bool	MsgProc(CGUIEvent *event=NULL);
		virtual bool	MsgProc(MSG *event);

		virtual HRESULT Render(GUIState* pGUIState, float fElapsedTime);
		virtual void    UpdateRects();
		virtual void	InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		virtual bool 	OnMouseLeave();
		virtual bool	OnChange(const char* code = NULL);

		/**
		* Set the track range of the scroll bar.
		* @param nStart: the start value
		* @param nEnd: the end value
		*/
		void SetTrackRange(int nStart, int nEnd);

		/**
		* Get the track position of the scroll bar.
		* @return: the current track position
		*/
		int GetTrackPos() { return m_nPosition; }

		/**
		* Get the track start value of the scroll bar.
		* @return: the current start value
		*/
		int GetTrackStart(){ return m_nStart; };
		/**
		* Get the track end value of the scroll bar.
		* @return: the current end value
		*/
		int GetTrackEnd(){ return m_nEnd; };

		/** track position*/
		void SetValue(int nValue) { SetTrackPos(nValue); };

		/** track position*/
		int  GetValue() const { return m_nPosition; };

		/**
		* Set the track position of the scroll bar.
		* @param nPosition: the new track position
		*/
		void SetTrackPos(int nPosition) { m_nPosition = nPosition; Cap(); UpdateThumbRect(); }

		/**
		* Get the page size of the scroll bar
		* Page size is the size of how many items a page contains. The control will scroll a page size if we click on the empty space of the track of the scroll bar
		* @return: the page size of the scroll bar
		*/
		int GetPageSize() { return m_nPageSize; }

		/**
		* Set the page size of the scroll bar
		* Page size is the size of how many items a page contains. The control will scroll a page size if we click on the empty space of the track of the scroll bar
		* @param nPageSize: the new page size of the scroll bar
		*/
		void SetPageSize(int nPageSize) { m_nPageSize = nPageSize; Cap(); UpdateThumbRect(); }

		/** how many pixels to scroll each step */
		void SetStep(int nDelta){ m_nDelta = nDelta; }

		/** how many pixels to scroll each step */
		int GetStep(){ return m_nDelta; }

		/**
		* Scroll the scroll bar
		* @param nDelta: Scroll by nDelta items (plus or minus)
		*/
		void Scroll(int nDelta);

		/**
		* Ensure that an item is displayed, scroll if necessary
		* @param nIndex: the item to be displayed
		*/
		void ShowItem(int nIndex);

		void SetFixedThumb(bool bFixed);
		bool GetFixedThumb()const{ return m_bFixedThumb; }

		void SetThumbSize(int size){ m_nThumbSize = SCROLLBAR_MINTHUMBSIZE > size ? SCROLLBAR_MINTHUMBSIZE : size; m_bNeedUpdate = true; }
		int GetThumbSize()const { return m_nThumbSize; }

		void SetLinkedObject(CGUIBase* obj){ m_Linked = obj; }

		void					SetScrollbarWidth(int width);
		int						GetScrollbarWidth()const;
		// 		virtual void SetSize(int width, int height );
		// 		virtual void SetWidth(int width );
		// 		virtual void SetHeight(int height );

		static void StaticInit();
		virtual void Clone(IObject* pobj)const;
		virtual IObject *Clone()const;
		virtual const IType* GetType()const{ return m_type; }

	protected:
		static const IType* m_type;
		// ARROWSTATE indicates the state of the arrow buttons.
		// CLEAR            No arrow is down.
		// CLICKED_UP       Up arrow is clicked.
		// CLICKED_DOWN     Down arrow is clicked.
		// HELD_UP          Up arrow is held down for sustained period.
		// HELD_DOWN        Down arrow is held down for sustained period.
		enum ARROWSTATE { CLEAR, CLICKED_UP, CLICKED_DOWN, HELD_UP, HELD_DOWN, CLICKED, HELD };

		/** update thumb rect */
		void UpdateThumbRect();

		/** update nine element thumb rects (4, 4+8) according to original calcuated thumb rect at index 3*/
		void UpdateThumbRectNineElement();

		void Cap();  // Clips position at boundaries. Ensures it stays within legal range.

		bool m_bShowThumb;
		int m_nPosition;  // Position of the first displayed item
		int m_nOldPosition;
		int m_nPageSize;  // How many items are displayable in one page
		int m_nStart;     // First item
		int m_nEnd;       // The index after the last item
		int m_nDelta;	  // How many items are scrolled in one click
		//		POINT m_LastMouse;// Last mouse position
		ARROWSTATE m_Arrow; // State of the arrows
		DWORD m_dArrowTS;  // Timestamp of last arrow event.
		int m_ScrollType;
		bool m_bFixedThumb;//if the thumb has fixed size, default is true
		CGUIBase* m_Linked;
		int m_nThumbSize;//thumb size if m_bFixedThumb=true, default is 20;
	};
}