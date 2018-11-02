//----------------------------------------------------------------------
// Class:	CObjectManager
// Authors:	Liu Weili
// Date:	2006.3.18
//
// desc: This class holds a copy of any object in a global array. 
// This input object should implement the IObject interface. 
// We can use this class to store the default class of any kind. So when we create a new object, 
// we can clone an object from this class. This can reduce the cost of creating a new object and
// trying to set its properties to default. 
// This class should be used with CSingleton to ensure there is only one instance in the program.
// This class is thread-safe. All functions are mutually exclusive from other threads. So the data
// integrity is preserved. 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ObjectManager.h"
#include "memdebug.h"
using namespace ParaEngine;
CObjectManager::CObjectManager()
{
}
CObjectManager::~CObjectManager()
{
	Finalize();
}

bool CObjectManager::IsExist(const char* name)const
{
	if (m_pool.find(name)==m_pool.end()) {
		return false;
	}
	return true;
}

void CObjectManager::SetObject(const char* name, IObject *pobj)
{
	map<string,IObject*>::iterator iter;
	if ((iter=m_pool.find(name))!=m_pool.end()) {
		SAFE_RELEASE(iter->second);
	}
	m_pool[name]=pobj->Clone();
}

IObject* CObjectManager::CloneObject(const char* name)const
{
	map<string,IObject*>::const_iterator iter;
	if ((iter=m_pool.find(name))==m_pool.end()) {
		return NULL;
	}
	return iter->second->Clone();
}

void CObjectManager::CloneObject(const char* name,IObject* pobj)const
{
	map<string,IObject*>::const_iterator iter;
	if ((iter=m_pool.find(name))==m_pool.end()) {
		return;
	}
	iter->second->Clone(pobj);

}

const IObject* CObjectManager::GetObject(const char* name)const
{
	map<string,IObject*>::const_iterator iter;
	if ((iter=m_pool.find(name))==m_pool.end()) {
		return NULL;
	}
	return iter->second;
}

void CObjectManager::Finalize()
{
	map<string,IObject*>::iterator iter,iterend=m_pool.end();
	for (iter=m_pool.begin();iter!=iterend;iter++) {
		SAFE_RELEASE(iter->second);
	}
	m_pool.clear();
}