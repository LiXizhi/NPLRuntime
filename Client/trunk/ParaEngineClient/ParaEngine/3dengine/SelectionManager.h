#pragma once
#include <vector>
#include <list>
#include "IRefObject.h"

namespace ParaEngine
{
	using namespace std;
	class CBaseObject;
	class CGUIBase;

	/**
	* currently selected item. only a weak reference is kept for the selected object.
	*/
	struct SelectedItem
	{
		IAttributeFields::WeakPtr_type m_pObjectRef;
		enum ObjectBaseType{
			TYPE_CBASEOBJECT,
			TYPE_CGUIBASE,
			TYPE_UNKNOWN,
		}m_nItemType;

		bool m_bSelected;

	public:
		SelectedItem() :m_bSelected(false), m_nItemType(TYPE_UNKNOWN){};
		SelectedItem(CBaseObject* pObj) :m_pObjectRef((IAttributeFields*)pObj), m_bSelected(false), m_nItemType(TYPE_CBASEOBJECT){};
		SelectedItem(CGUIBase* pObj) :m_pObjectRef((IAttributeFields*)pObj), m_bSelected(false), m_nItemType(TYPE_CGUIBASE){};

		inline IAttributeFields* GetObject(){
			return m_pObjectRef.get();
		};

		CBaseObject* GetAs3DObject(){
			return Is3DObject() ? (CBaseObject*)m_pObjectRef.get() : NULL;
		};

		CGUIBase* GetAs2DObject(){
			return (Is2DObject()) ? (CGUIBase*)m_pObjectRef.get() : NULL;
		};

		/** whether it is still a valid object. */
		operator bool() const
		{
			return m_pObjectRef;
		}

		inline bool Is3DObject(){ return m_nItemType == TYPE_CBASEOBJECT && m_pObjectRef; };
		inline bool Is2DObject(){ return m_nItemType == TYPE_CGUIBASE && m_pObjectRef; };
	};

	/**
	* a group of selected object
	*
	*/
	class CSelectionGroup : public IAttributeFields
	{
	public:
		CSelectionGroup() : m_bSelected(false), m_nMaxItemNumber(1), m_bRemoveFromBack(false), m_bExclusiveSelect(true) {};
		virtual ~CSelectionGroup(){};

		ATTRIBUTE_DEFINE_CLASS(CSelectionGroup);

		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

	public:
		/** get item count. */
		int GetItemCount() { return (int)(m_items.size()); }

		/** get item at give index. must ensure index is valid. */
		SelectedItem& GetItem(int nIndex) { return m_items[nIndex]; }

		/** default to true. if true, an object can only be in one group only. */
		bool IsExclusive(){ return m_bExclusiveSelect; }
	public:
		/** all items in the selection group. */
		vector<SelectedItem> m_items;

		/** whether the entire group is selected.default value is false */
		bool m_bSelected;

		/** maximum number of objects in the group. default value is 1 */
		int m_nMaxItemNumber;

		/** if true, objects exceeding the max item number will be removed from the back of
		* the item list, otherwise it will be removed from the front. default value is false*/
		bool m_bRemoveFromBack;

		/** default to true. if true, an object can only be in one group only. */
		bool m_bExclusiveSelect;
	};

	/**
	* A pool of currently selected objects. This is a singleton class.
	* Object may be selected into different groups. Although, there are no limit to group number, better keep it smaller than 16 groups.
	* Selected objects may be displayed or highlighted differently.
	* When objects are deleted from the scene. It will be deleted from the selection manager automatically.
	*/
	class CSelectionManager : public IRefObject
	{
	public:
		CSelectionManager(void);
		virtual ~CSelectionManager(void);
		ATTRIBUTE_DEFINE_CLASS(CSelectionManager);

		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		/**
		* get the singleton instance of this class
		* @return this always returns valid pointer.
		*/
		static CSelectionManager* GetSingleton();
	public:
		/** This is called after another object deleted this object from its reference list.
		* If this object needs to know, it can override it.
		* @param rm:
		* @remark: one can call DeleteReference in the OnRefDeleted, to delete bi-directional reference.
		* By default, a bi-directional reference is deleted in this function
		*/
		virtual void OnRefDeleted(IRefObject* rm);

		/**
		* remove a given object in all selections.
		* @param pObject pointer to the object to delete
		*/
		void RemoveObject(CBaseObject* pObject);
		void RemoveObject(CGUIBase* pObject);

		/** get the number occurrences of a given object.
		*/
		int GetObjectCount(CBaseObject* pObject);

		/**
		* Add a new object to a given group. An object may belong to multiple groups.
		* @param pObject pointer to the object to add
		* @param nGroupID which group the should be added to. be default it is added to group 0.
		* group ID must be smaller than 32.
		*/
		void AddObject(CBaseObject* pObject, int nGroupID = 0);
		void AddObject(CGUIBase* pObject, int nGroupID = 0);

		/**
		* get the nItemIndex object in the nGroupID group.
		* @param nGroupID from which group the object is get
		* @param nItemIndex the index of the item to be retrieved.
		* @param pOut [out] selected item is returned.
		* @return true if there is a object at that slot.
		*/
		bool GetObject(int nGroupID, int nItemIndex, SelectedItem* pOut);

		/**
		* get the total number item in the given group.  This function can be used with GetObject()
		* to iterate through all objects in any group.
		* @param nGroupID group ID.
		* @return
		*/
		int GetItemNumInGroup(int nGroupID);

		/**
		* select the entire group.
		* @param nGroupID
		* @param bSelect true to select, false to de-select.
		*/
		void SelectGroup(int nGroupID, bool bSelect);
		/**
		* Clear a given group so that there are no objects in it.
		* @param nGroupID ID of the group. If ID is -1, all groups will be deleted.
		*/
		void ClearGroup(int nGroupID);

		/**
		* set the maximum number of objects in the group.
		* @param nGroupID group ID
		* group ID must be smaller than 32.
		* @param nMaxItemsNumber the number to set. default value is 1
		*/
		void SetMaxItemNumberInGroup(int nGroupID, int nMaxItemsNumber);

		/**
		* set the maximum number of objects in the group.
		* @param nGroupID group ID,which ID must be smaller than 32.
		* @return the maximum number in the given group
		*/
		int GetMaxItemNumberInGroup(int nGroupID);

		/**
		* return the given group if it is non-empty.
		* @param nGroupID group to get
		* @return NULL if the group is empty.
		*/
		CSelectionGroup* GetGroup(int nGroupID);

		/**
		* create a new group if it is not created before
		* @param nGroupID
		* @return the new group or existing group is returned.
		*/
		CSelectionGroup* CreateGroup(int nGroupID);

		/** get the group count. Please note, this is only the max group index plus 1. So GetGroup() may return NULL if empty for index smaller than GroupCount. */
		int GetGroupCount(){ return (int)(m_groups.size()); }

	protected:

		/**
		* it will delete object when there are more objects than its capacity.
		* @param nGroupID
		*/
		void UpdateGroup(int nGroupID);

	public:
		vector< ref_ptr<CSelectionGroup> > m_groups;
	};

}
