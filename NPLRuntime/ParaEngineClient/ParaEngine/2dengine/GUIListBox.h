#pragma once

#include "GUIContainer.h"
#include "GUIScript.h"

namespace ParaEngine
{
	struct GUIListItem {
		u16string text;
		bool selected;
		GUI_SCRIPT onclick;		
		CGUIBase *advObj;
	};

	/**
	m_objResource->m_objArtwork->DrawingRects[0]: client rectangle;
	m_objResource->m_objArtwork->DrawingRects[1]: container rectangle, including scrollbars
	m_objResource->m_objArtwork->DrawingRects[2]: container rectangle, including scrollbars, including borders
	m_objResource->m_objArtwork->DrawingRects[3]: text rectangle
	*/
	class CGUIListBox:public CGUIContainer
	{
	public:
		CGUIListBox();
		virtual ~CGUIListBox();

		/**
		* Enable or disable multiselect of the listbox
		* @param multiselect: multiselect state
		*/
		void SetMultiSelect(bool multiselect){m_bMultiSelect=multiselect;};
		/**
		* Get multiselect state of the listbox
		* @return: multiselect state
		*/
		bool GetMultiSelect(){return m_bMultiSelect;};

		/**
		* Set height of each item.
		* Users don't need to explicitly set height of each item. It will adjust to fit the initial font size.
		* @param height: the height of each item, in pixel
		*/
		void SetItemHeight(int height);
		/**
		* Get the height of each item
		* @return: the height of each item, in pixel
		*/
		int GetItemHeight(){return m_ItemHeight;}

		/**
		* Get the selected item.
		* If it is multiselect listbox, this returns one of the selected items.
		* @return: the index of the selected item;
		*/
		int GetSelected(){return m_nSelected;};
		/**
		* Check if an item at certain index is selected.
		* Users can use this function to check if an item is selected, useful when the listbox is multiselect listbox
		* @return: return true if the item is selected;
		*/
		bool  GetSelected(int index);
	
		/**
		* get the text of the control
		* @param szText [out] buffer to receive the text
		* @param nLength size of the input buffer
		* @return return the number of bytes written to the buffer. If nLength is 0, this function returns the total number of characters of the text
		*/
		virtual int GetTextA(std::string& out);
		int GetTextA(int index, std::string& out);
		/**
		* get the text of the control 
		* The control internally store everything in Unicode. So it is faster this version than the ascii version.
		* @return 
		*/
		virtual const char16_t* GetText();;
		const char16_t* GetText(int index);

		/**
		* Add a GUIListItem item to the listbox.
		* The item is copied into a new item in the listbox.
		* @param listitem: the item to be added into the list, 
		*/
		void AddItem(const GUIListItem& listitem);

		/**
		* Add a text item into the listbox
		* @param szText: the input text string.
		*/
		void AddTextItemA(const char* szText);

		/**
		* Insert an item into the listbox
		* The inserted item will be at the position given by parameter index
		* @param index: the position where the item is inserted to
		* @param listitem: the item to be inserted.
		*/
		void InsertItem(int index,const GUIListItem* listitem);
		/**
		* Delete an item at given place
		* @param index: the index of the item to be deleted.
		*/
		void DeleteItem(int index);

		/**
		* Delete all item in the list box
		*/
		void RemoveAll();
		/**
		* Get the item at the given index
		* @param index: the index of the item we want to get
		* @return: return the item we get
		*/
		GUIListItem* GetItem(int index);

		/**
		* Set Wordbreak of the listbox.
		* @param wordbreak: the new value of Wordbreak
		* @remark: Wordbreak specifies whether the text in the items will automatically breaks to a new line
		*	If it is true, the text will breaks to a new line if the line exceed the right boundary.
		*   If it is false, the text that exceed the right boundary will be clipped.
		*/
		void SetWordBreak(bool wordbreak);
		/**
		* Get Workbreak of the list item
		* @return: the Wordbreak of the listbox
		*/
		bool GetWordBreak(){return m_bWordBreak;};

		/**
		* Select a specific item
		* @param index: the index of the item you want to select
		*/
		void SelectItem(int index);
		/**
		* decide if the items can be dragged inside the listbox
		*/
		bool GetItemCanDrag(){return m_bItemCanDrag;}
		void SetItemCanDrag(bool candrag){m_bItemCanDrag=candrag;}
		
		/**
		* Get number of items in the CGUIListBox
		*/
		int Size(){return (int)m_items.size();}

		virtual void			UpdateScroll(int nXDelta,int nYDelta);
		virtual HRESULT			Render(GUIState* pGUIState ,float fElapsedTime);

		virtual void			InitObject(const char * strObjectName, const char * alignment, int x, int y, int width, int height);
		//virtual bool			MsgProc(CGUIEvent *event=NULL);
		virtual bool			MsgProc(MSG *event);
		virtual void			UpdateRects();
		
		virtual IObject* Clone()const;
		virtual void Clone(IObject *pobj)const;
		virtual const IType* GetType()const{return m_type;}

		static void StaticInit();
		///events
	protected:
		static const IType* m_type;
		//if allow select multiple items
		bool m_bMultiSelect;
		//the index of the selected item, it is the first selected item if m_bMultiSelect is true; if nothing selected, it is -1;
		int m_nSelected;
		int m_ItemHeight;
		mutable list<GUIListItem> m_items;
		map<int,GUIListItem*> m_itemmap;
		int m_deltaX,m_deltaY;
		bool m_bItemCanDrag;
		bool m_bAllowAdvObject;
		bool m_bDrag;       // Whether the user is dragging the mouse to select
		bool m_bWordBreak;
		Color m_SelBkColor;   // Selected background color
		int m_nSelStart;
	};
}