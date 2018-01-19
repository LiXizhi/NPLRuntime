//----------------------------------------------------------------------
// Class:	Reference counted
// Authors:	LiXizhi, Liu Weili
// Date:	2006.3.17
//
// desc: 
// This interface is intended to provide a common interface for object managers
// to manager the objects. When someone wants to get an copy of the object from
// the object manager, the object manager can call Clone() method to generate
// an indentical copy. The manager can also use Equals() to decide whether the 
// two object are the same. When the program terminates, the Release() method is
// called to release all the resource allocated before for the object. 
// We don't need to know the actual type of the objects through this interface.
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ObjectAutoReleasePool.h"
#include "Type.h"
#include "PERef.h"

using namespace ParaEngine;

ParaEngine::CRefCounted::CRefCounted()
	: m_refcount(0)
{

}

ParaEngine::CRefCounted::~CRefCounted()
{

}

CRefCounted* ParaEngine::CRefCounted::AddToAutoReleasePool()
{
	CObjectAutoReleasePool::GetInstance()->AddObject(this);
	return this;
}

int ParaEngine::CRefCounted::Release()
{
	if (delref()) {
		int nRefCount = GetRefCount();
		delete this;
		return nRefCount;
	}
	return GetRefCount();
}

ParaEngine::CRefCountedOne::CRefCountedOne()
{
	m_refcount = 1;
}

void ParaEngine::IObject::Clone(IObject* obj) const
{
}

IObject* ParaEngine::IObject::Clone() const
{
	IObject *obj = new IObject();
	Clone(obj);
	return obj;
}

bool ParaEngine::IObject::Equals(const IObject *obj) const
{
	return this == obj;
}

std::string ParaEngine::IObject::ToString() const
{
	return "";
}

ParaEngine::IObject::~IObject()
{
	if (m_weak_reference){
		m_weak_reference.UnWatch();
	}
}

IObject::WeakPtr_type& IObject::GetWeakReference()
{
	if (m_weak_reference)
		return m_weak_reference;
	else
	{
		m_weak_reference = new IObject::WeakPtr_type::WeakRefObject_type(this);
		return m_weak_reference;
	}
}


