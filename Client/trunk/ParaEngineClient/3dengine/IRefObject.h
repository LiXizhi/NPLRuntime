#pragma once
#include "IAttributeFields.h"
#include "util/unordered_array.hpp"

namespace ParaEngine
{
	using namespace std;
	class IRefObject;

	typedef DWORD RefMessage;
	enum REF_MESSAGE
	{
		/** when a reference object(caller) is deleted, it will send this message to all its references.
		The default message handler will remove the caller from their reference lists. */
		REFMSG_REF_DELETED = 0x1,
		/** when a reference object add, it will send this message to all its references.*/
		REFMSG_REF_ADDED = 0x2,
	};

	/* Some return codes for references...
	* There would probably be a bunch of these.
	*/
	enum RefResult {
		REF_FAIL,
		REF_SUCCEED,
		REF_DONTCARE,
		REF_STOP,
		REF_INVALID,
		// Return this from NotifRefChanged() when a REFMGS_TAGET_DELETED message is sent
		// if a ReferenceMaker should be removed from the scene when all its references are removed.
		REF_AUTO_DELETE
	};

	/**
	* a referenced object
	*/
	class RefListItem
	{
	public:
		IRefObject*  m_object;
		int m_tag;
		RefListItem( IRefObject*  ref, int nTag):m_object(ref),m_tag(nTag) {}
		RefListItem( IRefObject*  ref):m_object(ref), m_tag(0) {};
		RefListItem():m_object(NULL), m_tag(0) {};

		inline bool operator == (const RefListItem& r){
			return this->m_object == r.m_object;
		}

		inline bool operator == (const IRefObject* r){
			return this->m_object == r;
		}

		bool operator == (const char* r);
	};
	/** reference list */
	typedef unordered_array<RefListItem> RefList;

	/**
	* anything that makes references to other objects. 
	* by default, it create bi-directional references. However, one can override its functions to create one-directional reference.
	* This is unlike parent/child relationship, where reference counting is used. 
	* This is also not weak_reference, it uses event system to ensure that all referenced objects valid without the need to increase/decrease reference count. 
	*/
	class IRefObject : public IAttributeFields
	{
	protected:
		/// This is the list of active references that refer to us.
		RefList m_references;

		/// unit name used in the scripting language
		std::string	m_sIdentifer;		
	public:
		IRefObject() {} 
		virtual ~IRefObject();

		/** get the name or identifier. */
		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		/** gets the unicode name */
		int GetNameW(std::u16string& out);
		/** alias name for GetIdentifier */
		const std::string& GetName();

		/**
		* @param sName 
		* @return NULL if not found.
		*/
		IRefObject* GetRefObjectByName(const char* sName);

		/**
		* get the first reference object by its tag
		*/
		RefListItem* GetRefObjectByTag(int nTag);

		/**
		* @return return false if empty
		*/
		inline bool HasReferences(){ return !(m_references.empty());};
		
		/**
		* add a new reference. 
		* @param maker 
		* @return 
		*/
		RefResult AddReference(IRefObject* maker, int nTag = 0);

		/**
		* delete a reference.
		* @param ref 
		* @return return REF_FAIL if reference not found. otherwise REF_SUCCEED
		*/
		RefResult DeleteReference(IRefObject* ref);

		/** Deletes all references of this object.*/
		RefResult DeleteAllRefs();

		/** Deletes all references whose tag is nTag of this object.
		* @return the number of object deleted. 
		*/
		int DeleteAllRefsByTag(int nTag = 0);

		/** get the ref list*/
		inline RefList& GetRefList() { return m_references; }
		
		/** get the total number of references */
		int GetRefObjNum();

		/**
		* get the referenced object at the given index. 
		* @param nIndex 
		* @return NULL if invalid index.
		*/
		RefListItem* GetRefObject(int nIndex);
		
		/** This is called after another object added this object to its reference list.
		* If this object needs to know, it can override it.
		* @param rm: 
		* @param nTag: default to 0
		* @remark: one can call AddReference in the OnRefAdded, to create bi-directional reference. 
		* By default, a bi-directional reference is created in this function
		*/
		virtual void OnRefAdded(IRefObject* rm, int nTag=0);

		/** This is called after another object deleted this object from its reference list.
		* If this object needs to know, it can override it.
		* @param rm: 
		* @remark: one can call DeleteReference in the OnRefDeleted, to delete bi-directional reference.
		* By default, a bi-directional reference is deleted in this function
		*/
		virtual void OnRefDeleted(IRefObject* rm);

		/** This function is called when the object's last ref is deleted. */
		//virtual RefResult AutoDelete(){ return REF_SUCCEED;};

		// Notify all dependent RefMakers concerned with the message 
		// virtual RefResult NotifyReferences(RefMessage message, bool propagate=true, IRefObject* pCaller=NULL);
	};
}