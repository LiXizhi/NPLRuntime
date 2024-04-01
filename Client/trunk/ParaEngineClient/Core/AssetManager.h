#pragma once

#include "AssetEntity.h"

namespace ParaEngine
{
	/** 
	* AssetManager manages a set of asset entities of a certain type.
	* IDTYPE must be AssetEntity derived class. ClassImpType is the actually implement of the type. 
	* We can access the singleton via AssetManager<type>::GetInstance()
	*/
	template <class IDTYPE, class ClassImpType = IDTYPE, class ETYPE = AssetEntity>
	class AssetManager : public IAttributeFields
	{
	public:
		typedef AssetManager<IDTYPE, ClassImpType, ETYPE> MyType_t;
		typedef std::map<std::string, ETYPE*> AssetItemsNameMap_t;
		typedef std::map<AssetKey, ETYPE*> AssetItemsSet_t;

		AssetManager()
		{
			static_assert(std::is_convertible<IDTYPE*, AssetEntity*>::value, "Invalid Type for AssetManager!");
			// since asset manager is kind of singleton pattern, we will always set reference count to 1 during creation. 
			addref();
		}
		virtual ~AssetManager()
		{
			Cleanup();
		}

		ATTRIBUTE_DEFINE_CLASS(AssetManager);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName){
			return GetByName(sName);
		};
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0) { return (int)m_items.size(); };
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount() { return 1; };
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0){
			if (nRowIndex < (int)m_items.size())
			{
				auto iter = m_items.begin();
				std::advance(iter, nRowIndex);
				return iter->second;
			}
			return NULL;
		}

		inline static MyType_t& GetInstance() {
			static MyType_t s_instance;
			return s_instance;
		}

		virtual const std::string& GetIdentifier() {
			return m_sName;
		}
		virtual void SetIdentifier(const std::string& sName){
			m_sName = sName;
		}

		/**
		* delete all asset item from memory. They can never be used again. instead,
		* they must be recreated to use.
		*/
		virtual void Cleanup()
		{
			/// clean up items
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
#ifdef _DEBUG
				ETYPE* pAsset =itCurCP->second;
				if(pAsset->GetRefCount()>1){
					OUTPUT_LOG("warning: asset <%s> exits with ref %d\n", pAsset->m_key.c_str(), pAsset->GetRefCount()-1);
				}
#endif
				// normally, it should have 0 reference count at this place. And a delete this operation is performed. 
				itCurCP->second->Release();

				// force deletion regardless of reference count. 
				//(*itCurCP)->Cleanup();
				//delete ((IDTYPE*)(*itCurCP));
			}
			m_items.clear();

			// clean up named references
			m_names.clear();

			m_lowercase_item_maps.clear();
		}

		/**
		* get the lower cased key string. 
		*/
		inline void GetLowerCaseString(std::string& outNameLowered, const AssetKey& key){
			const char A = (char)'A';
			const char Z = (char)'Z';
			const char diff = (char)'a' - A;
			int nLen = (int)key.size();
			outNameLowered = key;
			char c;
			for (int i=0; i<nLen; ++i)
			{
				c = key[i];
				if (c>=A && c<=Z)
					outNameLowered[i] = c + diff;
			}
		}

		/**
		* decrease the reference count on the entity, and if it is negative, the asset will
		* be unloaded.
		* CAUTION: be sure that there is no named asset referencing this asset before calling this function.
		* because the point to the asset will be invalid after this call.So use UnloadAsset(),
		* if one just wants to temporarily remove the asset. 
		* return: return true, if the reference count is negative and that object is deleted from the memory, otherwise false.
		* @note: it also delete name mapping of the entity's key name
		*/
		virtual bool DeleteEntity(ETYPE* entity)
		{
			// remove from the list
			const AssetKey& key = entity->GetKey();

			// remove from lower cased name mapping. 
			std::string sNameLowered;
			GetLowerCaseString(sNameLowered, key);
			typename AssetItemsNameMap_t::iterator iter = m_lowercase_item_maps.find(sNameLowered);
			if( iter != m_lowercase_item_maps.end())
			{
				// remove name mapping
				if(iter->second != entity)
				{
					OUTPUT_LOG("warning: DeleteEntity %s with multiple candidates\n", key.c_str());
				}
				m_lowercase_item_maps.erase(iter);
			}

			// remove anyway
			auto itCur = m_items.find(key);
			if(itCur!=m_items.end())
			{
				m_items.erase(itCur);
			}

			// check references
			if(entity->GetRefCount() > 1) 
			{
				OUTPUT_LOG("warning: you are deleting an entity %s whose has unreleased external references\n", entity->GetKey().c_str());
			}
			// remove name mapping if any
			auto iter1 = m_names.find(key);
			if (iter1 != m_names.end() && iter1->second == entity)
			{
				m_names.erase(iter1);
			}

			// unload anyway.
			entity->UnloadAsset();
			// normally, it should have 0 reference count at this place. And a delete this operation is performed. 
			entity->Release();

			return true;
		}

		/**
		* delete the object by user specified name.
		* CAUTION: be sure that there is no other asset referencing this named asset before calling this function.
		* because the point to the asset will be invalid after this call.So use UnloadAsset(),
		* if one just wants to temporarily remove the asset. 
		*/
		void DeleteByName(const std::string& name)
		{
			auto iter = m_names.find(name);
			if( iter != m_names.end())
			{
				// remove name mapping
				auto pEntity = (*iter).second;
				m_names.erase(iter);

				// delete entity
				DeleteEntity(pEntity);
			}
		}

		/**
		* check if there is a object with a specified name, and return the pointer to it.
		* @return: a pointer to the object is returned if found, otherwise, return NULL.
		*/
		IDTYPE* GetByName(const std::string& name)
		{
			typename AssetItemsNameMap_t::iterator iter = m_names.find(name);
			if( iter != m_names.end())
				return (IDTYPE*)((*iter).second);
			else
				return NULL;
		}

		/**
		* check if there is a object with a specified asset key, and return the pointer to it.
		* @return: a pointer to the object is returned if found, otherwise, return NULL.
		*/
		ETYPE* get(const AssetKey& key)
		{
			// first search using asset key, if not found, we will search the lower cased AssetKey version. 
			typename AssetItemsSet_t::iterator iter = m_items.find(key);
			if(iter != m_items.end())
			{
				return iter->second;
			}
			else
			{
				// search in lower cased 
				std::string sNameLowered;
				GetLowerCaseString(sNameLowered, key);
				typename AssetItemsNameMap_t::iterator iter1 = m_lowercase_item_maps.find(sNameLowered);
				if( iter1 != m_lowercase_item_maps.end())
				{
					// remove name mapping
					return iter1->second;
				}
			}
			return NULL;
		}
	public:

		/** just create the entity instance without adding to the manager. 
		* the caller is responsible to delete the entity and manage device lost. 
		* Use CreateEntity for manager managed entity. 
		*/
		IDTYPE* NewEntity(const AssetKey& key){
			IDTYPE* pEntity = new ClassImpType(key);
			return pEntity;
		}

		/** insert entity to the manager. return true if succeed. */
		bool AddEntity(const std::string& name, IDTYPE* pEntity)
		{
			const AssetKey& key = pEntity->GetKey();
			ETYPE* pEntityOld = get(key);
			if (pEntityOld)
			{
				if (pEntityOld == pEntity)
					return true;
				else
				{
					if (!DeleteEntity(pEntityOld))
					{
						return false;
					}
				}
			}
			
			{
				if (!name.empty())
				{
					// add a named asset reference.
					m_names[name] = pEntity;
				}
				m_items[key] = pEntity;
				pEntity->addref();

				// add a lower cased map
				std::string sNameLowered;
				GetLowerCaseString(sNameLowered, key);
				m_lowercase_item_maps[sNameLowered] = pEntity;
			}
			return true;
		}

		/**
		* Create a new entity object and add it to the manager.
		* an asset entity will only be created if there is no asset which the same key.
		* we will assign a new named asset reference, even if the object has been created before 
		* thus we can assign different names to the same asset object. This is useful in scripting.
		* 
		* @param name: a human readable name associated with the object. 
		*	it can be "", in which case it does not have a human readable name.
		* @param key: the asset key object to be used for creating this asset.
		* @return : pair.first always contains the object pointer whose asset key is key.
		* 	if pair.second is true, the object is newly created; otherwise, it is already been created before.
		`	* @see:	get(std::string name)
		*/
		pair<IDTYPE*, bool> CreateEntity(const string& name, const AssetKey& key)
		{
			ETYPE* pEntity = get(key);
			if(pEntity)
			{
				if( ! name.empty() )
				{
					/// we will assign a new named asset reference, even if the object has been created elsewhere 
					/// thus we can assign different names to the same object
					ETYPE* pOld = GetByName(name);
					if(pOld == NULL)
					{
						// add a named asset reference.
						m_names[name] = pEntity;
					}
				}
				/// return false if the object already exists
				return pair<IDTYPE*, bool>((IDTYPE*)pEntity, false);
			}
			else
			{
				pEntity = NewEntity(key);
				if( ! name.empty() )
				{
					// add a named asset reference.
					m_names[name] = pEntity;
				}
				m_items[key] = pEntity;
				pEntity->addref();

				// add a lower cased map
				std::string sNameLowered;
				GetLowerCaseString(sNameLowered, key);
				m_lowercase_item_maps[sNameLowered] = pEntity;

				return pair<IDTYPE*, bool>((IDTYPE*)pEntity, true);
			}
		}

		
		/**
		* get the entity by its entity key name
		*/
		IDTYPE* GetEntity(const string& name)
		{
			return (IDTYPE*)get(name);
		}

		/** initialize all assets created so far to accelerate loading during game play. */
		void LoadAsset(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->LoadAsset();
			}
		}
		/** uninitialize all assets created so far to save some memory */
		void UnloadAsset(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->UnloadAsset();
			}
		}
		/** Garbage Collect(free resources of) all unused entity.*/
		void GarbageCollectAll(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->GarbageCollectMe();
			}
		}

		/**
		* check if the entity exist, if so call Refresh().  
		* @param sEntityName: the asset entity key
		* @param bLazyLoad if true it will be lazy loaded.
		*/
		bool CheckRefresh(const std::string& sEntityName)
		{
			ETYPE* pEntity = GetEntity(sEntityName);
			if(pEntity!=0){
				pEntity->Refresh();
				return true;
			}
			return false;
		}

		/** print all asset file to a given file. Each asset is on a single line, in the following format: 
		[AssetFileName]
		* @param pOutputFile: to which file object the result is written to
		* @return: the number of results are returned. 
		*/
		virtual int PrintToFile(CParaFile* pOutputFile)
		{
			int i=0;
			if (pOutputFile)
			{
				typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
				for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP,++i)
				{
					pOutputFile->WriteString(itCurCP->second->GetKey());
					pOutputFile->WriteString("\n");
				}
			}
			return i;
		}

	public:
		virtual void InitDeviceObjects(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->InitDeviceObjects();
			}
		};
		virtual void RestoreDeviceObjects(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->RestoreDeviceObjects();
			}
		}
		virtual void InvalidateDeviceObjects(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->InvalidateDeviceObjects();
			}
		}
		virtual void DeleteDeviceObjects(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for( itCurCP = m_items.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				itCurCP->second->DeleteDeviceObjects();
			}
		}
		/** callback of listening the event that renderer was recreated on Android/WP8
		all opengl related id has already become invalid at this time, no need to release them, just recreate them all in this function.
		*/
		virtual void RendererRecreated(){
			typename AssetItemsSet_t::iterator itCurCP, itEndCP = m_items.end();
			for (itCurCP = m_items.begin(); itCurCP != itEndCP; ++itCurCP)
			{
				itCurCP->second->RendererRecreated();
			}
		}
	public:
		/** identifier */
		std::string m_sName;
		/** human readable name of asset object, which can be used as key to retrieve entity from its manager 
		* Internally we use the key object to uniquely identify an assset.SIdentifer can be nil. 
		*/
		AssetItemsNameMap_t m_names;
		/**
		* A set of all asset entities.
		*/
		AssetItemsSet_t m_items;
		/**
		* mapping from lower cased AssetKey to asset entity. 
		*/
		AssetItemsNameMap_t m_lowercase_item_maps;
	};
}