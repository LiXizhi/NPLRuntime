//-----------------------------------------------------------------------------
// Class:	particle system ref
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2010.3.10
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "particle.h"
#include "ParaXModel.h"
#include "TextureEntity.h"
#include "ParticleSystemRef.h"

#include "memdebug.h"

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// ParticleSystemRef
//
//////////////////////////////////////////////////////////////////////////

ParaEngine::ParticleSystemRef::ParticleSystemRef( ParticleSystem* ptr ) :m_ptr(ptr)
{
	if(m_ptr){
		if(m_ptr->GetParent()){
			m_ptr->GetParent()->addref();
		}
	}
}

ParaEngine::ParticleSystemRef::ParticleSystemRef( const ParticleSystemRef& r ) :m_ptr(r.m_ptr)
{
	if(m_ptr){
		if(m_ptr->GetParent()){
			m_ptr->GetParent()->addref();
		}
	}
}

ParaEngine::ParticleSystemRef::~ParticleSystemRef()
{
	if(m_ptr){
		if(m_ptr->GetParent()){
			m_ptr->GetParent()->Release();
		}
	}
}

ParticleSystemRef & ParaEngine::ParticleSystemRef::operator=(const ParticleSystemRef& r) /* never throws */
{
	if (m_ptr != r.m_ptr)
	{
		if(m_ptr!=0){
			if(m_ptr->GetParent()){
				m_ptr->GetParent()->Release();
			}
		}
		m_ptr = r.m_ptr;
		if(m_ptr!=0){
			if(m_ptr->GetParent()){
				m_ptr->GetParent()->addref();
			}
		}
	}
	return *this;
}