//-----------------------------------------------------------------------------
// Class:	IRefObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2006.8.23
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "StringHelper.h"
#include "IRefObject.h"

using namespace ParaEngine;

bool RefListItem::operator ==(const char* r)
{
	return this->m_object->GetName() == r;
}

//////////////////////////////////////////////////////////////////////////
//
// IRefObject
//
//////////////////////////////////////////////////////////////////////////

int IRefObject::GetNameW(std::u16string& out)
{
	return StringHelper::UTF8ToUTF16(GetIdentifier(), out) ? out.size() : 0;
}

const std::string& IRefObject::GetIdentifier()
{
	return m_sIdentifer;
}
void IRefObject::SetIdentifier(const std::string& sID)
{
	m_sIdentifer = sID;
}
const std::string& IRefObject::GetName()
{
	return GetIdentifier();
}

IRefObject* IRefObject::GetRefObjectByName(const char* sName)
{
	RefList::iterator itCur, itEnd = m_references.end();
	for (itCur = m_references.begin();itCur!=itEnd; ++itCur)
	{
		if( (*itCur)==sName)
		{
			return (*itCur).m_object;
		}
	}
	return NULL;
}


RefListItem* ParaEngine::IRefObject::GetRefObjectByTag( int nTag )
{
	RefList::iterator itCur, itEnd = m_references.end();
	for (itCur = m_references.begin();itCur!=itEnd; ++itCur)
	{
		if( (*itCur).m_tag==nTag)
		{
			return &(*itCur);
		}
	}
	return NULL;
}

RefResult IRefObject::AddReference(IRefObject* maker, int nTag )
{
	if(maker == 0 || maker==this) // prevent adding reference to itself.
		return REF_FAIL;
	RefList::iterator itCur, itEnd = m_references.end();
	for (itCur = m_references.begin();itCur!=itEnd; ++itCur)
	{
		if( (*itCur)==maker)
		{
			// already has it.
			(*itCur).m_tag = nTag;
			return REF_SUCCEED;
		}
	}
	if(nTag>=0)
		m_references.push_back(RefListItem(maker, nTag));
	else
	{
		m_references.insert(m_references.begin(), RefListItem(maker, nTag));
	}
	maker->OnRefAdded(this, nTag);
	return REF_SUCCEED;
}

RefResult IRefObject::DeleteReference(IRefObject* ref)
{
	if(ref == 0)
		return REF_FAIL;
	RefList::iterator itCur, itEnd = m_references.end();
	for (itCur = m_references.begin();itCur!=itEnd; ++itCur)
	{
		if( (*itCur)==ref)
		{
			m_references.erase(itCur);
			ref->OnRefDeleted(this);
			return REF_SUCCEED;
		}
	}
	return REF_FAIL; 
}

int IRefObject::DeleteAllRefsByTag(int nTag)
{
	int nCount = 0;
	RefList::iterator itCur, itEnd = m_references.end();
	for (itCur = m_references.begin();itCur!=itEnd; ++itCur)
	{
		if( (*itCur).m_tag == nTag)
		{
			IRefObject* ref = itCur->m_object;
			m_references.erase(itCur);
			if(ref!=0)
				ref->OnRefDeleted(this);
			nCount++;
			break;
		}
	}
	if(nCount == 0)
		return 0;

	// we shall delete recursively to prevent potential nested calls in ref->OnRefDeleted() in above calls. 
	return DeleteAllRefsByTag(nTag)+nCount;
}

RefResult IRefObject::DeleteAllRefs()
{
	if(!m_references.empty())
	{
		RefList tmp = m_references;
		m_references.clear();

		RefList::iterator itCur, itEnd = tmp.end();
		for (itCur = tmp.begin();itCur!=itEnd; ++itCur)
		{
			(*itCur).m_object->OnRefDeleted(this);
		}
	}
	return REF_SUCCEED;
};

void IRefObject::OnRefAdded(IRefObject* rm, int nTag) 
{
	AddReference(rm, nTag);
} 

void IRefObject::OnRefDeleted(IRefObject* rm) 
{
	DeleteReference(rm);
}  

int IRefObject::GetRefObjNum()
{
	return (int)m_references.size();	
}

RefListItem* IRefObject::GetRefObject(int nIndex)
{
	if( nIndex<(int)m_references.size() && nIndex>=0)
	{
		return &(m_references[nIndex]);
	}
	else
		return NULL;	
}

ParaEngine::IRefObject::~IRefObject()
{
	DeleteAllRefs();
}
