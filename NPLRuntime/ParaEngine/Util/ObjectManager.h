#pragma once
#include "CSingleton.h"
#include <map>
#include <string>
using namespace std;
namespace ParaEngine
{
	class CObjectManager
	{
	public:
		CObjectManager();
		~CObjectManager();
		/**
		 * Set the object with the given name.
		 * If the name already exists, the old object with the given will be released
		 * and the new object will be copied to the object manager. Otherwist, it will 
		 * copy the input object to the object manager.
		 **/
		void SetObject(const char* name, IObject *pobj);
		/**
		 * Test if an object exists. 
		 **/
		bool IsExist(const char * name)const ;
		/**
		 * Clone an object.  
		 **/
		IObject *CloneObject(const char* name)const;
		void CloneObject(const char* name,IObject* pobj)const;
		/**
		 * Get an object. 
		 **/
		const IObject* GetObject(const char* name)const;
		//this is called before the asset manager finalization in order to safe release the stored objects
		void Finalize();
	private:
		map<string, IObject*> m_pool;
	};
}