//-----------------------------------------------------------------------------
// Class:	ribbon emitter ref
// Desc:	Holds a refcount on CParaXModel while global ribbon trails decay
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "particle.h"
#include "ParaXModel.h"
#include "RibbonEmitterRef.h"
#include "memdebug.h"

using namespace ParaEngine;

ParaEngine::RibbonEmitterRef::RibbonEmitterRef(RibbonEmitter* ptr) : m_ptr(ptr)
{
	if (m_ptr && m_ptr->GetParent())
	{
		m_ptr->GetParent()->addref();
	}
}

ParaEngine::RibbonEmitterRef::RibbonEmitterRef(const RibbonEmitterRef& r) : m_ptr(r.m_ptr)
{
	if (m_ptr && m_ptr->GetParent())
	{
		m_ptr->GetParent()->addref();
	}
}

ParaEngine::RibbonEmitterRef::~RibbonEmitterRef()
{
	if (m_ptr && m_ptr->GetParent())
	{
		m_ptr->GetParent()->Release();
	}
}

RibbonEmitterRef& ParaEngine::RibbonEmitterRef::operator=(const RibbonEmitterRef& r)
{
	if (m_ptr != r.m_ptr)
	{
		if (m_ptr && m_ptr->GetParent())
		{
			m_ptr->GetParent()->Release();
		}
		m_ptr = r.m_ptr;
		if (m_ptr && m_ptr->GetParent())
		{
			m_ptr->GetParent()->addref();
		}
	}
	return *this;
}
