//-----------------------------------------------------------------------------
// Class:	Auto release pool for base object
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.1
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BaseObject.h"
#include "ObjectAutoReleasePool.h"

using namespace ParaEngine;

CObjectAutoReleasePool* CObjectAutoReleasePool::s_singleInstance = nullptr;


ParaEngine::CObjectAutoReleasePool::CObjectAutoReleasePool()
{
	m_object_pool.reserve(100);
}

ParaEngine::CObjectAutoReleasePool::~CObjectAutoReleasePool()
{
	clear();
}

CObjectAutoReleasePool* CObjectAutoReleasePool::GetInstance()
{
	if (s_singleInstance == nullptr)
	{
		s_singleInstance = new CObjectAutoReleasePool();
	}
	return s_singleInstance;
}

void CObjectAutoReleasePool::DestoryInstance()
{
	if (s_singleInstance)
	{
		delete s_singleInstance;
		s_singleInstance = nullptr;
	}
}

void ParaEngine::CObjectAutoReleasePool::AddObject(CRefCounted *object)
{
	object->addref();
	m_object_pool.push_back(object);
}

void ParaEngine::CObjectAutoReleasePool::clear()
{
	int nReleasedCount = 0;
	for (const auto &obj : m_object_pool)
	{
		if (obj->Release() == 0)
			nReleasedCount++;
	}
	m_object_pool.clear();
	if (nReleasedCount > 0)
	{
#ifdef _DEBUG
		// OUTPUT_LOG("CObjectAutoReleasePool release count %d \n", nReleasedCount);
#endif
	}
}

bool ParaEngine::CObjectAutoReleasePool::contains(CRefCounted* object) const
{
	for (const auto& obj : m_object_pool)
	{
		if (obj == object)
			return true;
	}
	return false;
}

void ParaEngine::CObjectAutoReleasePool::dump()
{
	OUTPUT_LOG("CObjectAutoReleasePool: number of managed object %d\n", (int)m_object_pool.size());
	OUTPUT_LOG("%20s%20s%20s\n", "Object pointer", "Object id", "reference count");
	for (const auto &obj : m_object_pool)
	{
		OUTPUT_LOG("%20p%20u\n", obj, obj->GetRefCount());
	}
}