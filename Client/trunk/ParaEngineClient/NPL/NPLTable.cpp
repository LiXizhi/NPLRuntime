//-----------------------------------------------------------------------------
// Class:	NPL Table
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Corporation
// Date:	2009.7.9
// Desc: cross-platformed 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLTable.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"	
#endif

using namespace NPL;
using namespace std;

NPLTable::~NPLTable()
{
	Clear();
}

void NPLTable::ToString(std::string& str)
{
}

void NPLTable::Clear()
{
	m_fields.clear();
}

void NPLTable::SetField(const string& sName, const NPLObjectProxy& pObject)
{
	TableFieldMap_Type::iterator iter = m_fields.find(sName);
	if(iter == m_fields.end())
	{
		if(pObject.get() != 0)
		{
			m_fields[sName] = pObject;
		}
	}
	else
	{
		if(pObject.get() != 0)
		{
			iter->second = pObject;
		}
		else
		{
			m_fields.erase(iter);
		}
	}
}

void NPLTable::SetField(int nIndex, const NPLObjectProxy& pObject)
{
	TableIntFieldMap_Type::iterator iter = m_index_fields.find(nIndex);
	if(iter == m_index_fields.end())
	{
		if(pObject.get() != 0)
		{
			m_index_fields[nIndex] = pObject;
		}
	}
	else
	{
		if(pObject.get() != 0)
		{
			iter->second = pObject;
		}
		else
		{
			m_index_fields.erase(iter);
		}
	}
}

NPLObjectProxy NPLTable::GetField(int nIndex)
{
	TableIntFieldMap_Type::iterator iter = m_index_fields.find(nIndex);
	return (iter != m_index_fields.end()) ? iter->second: NPLObjectProxy();
}

NPLObjectProxy NPLTable::GetField(const string& sName)
{
	TableFieldMap_Type::iterator iter = m_fields.find(sName);
	return (iter != m_fields.end()) ? iter->second: NPLObjectProxy();
}

NPLObjectProxy& NPLTable::CreateGetField(int nIndex)
{
	return m_index_fields[nIndex];
}

NPLObjectProxy& NPLTable::CreateGetField(const string& sName)
{
	return m_fields[sName];
}

//////////////////////////////////////////////////////////////////////////
//
// NPLObjectProxy
//
//////////////////////////////////////////////////////////////////////////

NPL::NPLObjectProxy::NPLObjectProxy( NPLObjectBase* pObject ) : NPLObjectBase_ptr(pObject)
{

}

NPLObjectProxy::operator double()
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_Number)
		*this = NPLObjectProxy(new NPLNumberObject());
	return ((NPLNumberObject*)get())->GetValue();
}

int NPL::NPLObjectProxy::toInt()
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Number)
		*this = NPLObjectProxy(new NPLNumberObject());
	return (int)((NPLNumberObject*)get())->GetValue();
}

void NPLObjectProxy::operator = (double value) 
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_Number)
		*this = NPLObjectProxy(new NPLNumberObject());
	((NPLNumberObject*)get())->SetValue(value);
}

NPLObjectProxy::operator bool ()
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_Bool)
		*this = NPLObjectProxy(new NPLBoolObject());
	return (((NPLBoolObject*)get())->GetValue());
}

void NPLObjectProxy::operator = (bool value) 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Bool)
		*this = NPLObjectProxy(new NPLBoolObject());
	((NPLBoolObject*)get())->SetValue(value);

}

NPLObjectProxy::operator const string& ()
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_String)
		*this = NPLObjectProxy(new NPLStringObject());
	return (((NPLStringObject*)get())->GetValue());
}

const char* NPL::NPLObjectProxy::c_str()
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_String)
		*this = NPLObjectProxy(new NPLStringObject());
	return (((NPLStringObject*)get())->GetValue().c_str());
}

bool NPLObjectProxy::operator == (const string& value)
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_String)
		return false;
	else
		return ((NPLStringObject*)get())->GetValue()==value;
}
bool NPLObjectProxy::operator == (const char* value) 
{
	if(get()==0 ||   GetType() != NPLObjectBase::NPLObjectType_String)
		return false;
	else
		return ((NPLStringObject*)get())->GetValue()==value;
}

void NPLObjectProxy::operator = (const std::string& value) 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_String)
		*this = NPLObjectProxy(new NPLStringObject());
	((NPLStringObject*)get())->SetValue(value);
}

void NPLObjectProxy::operator = (const char* value) 
{ 
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_String)
		*this = NPLObjectProxy(new NPLStringObject());
	((NPLStringObject*)get())->SetValue(value);
}

NPLObjectProxy& NPLObjectProxy::operator [] (const string& sName) 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->CreateGetField(sName);
};

NPLObjectProxy& NPLObjectProxy::operator [] (const char* sName) 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->CreateGetField(sName);

};

NPLObjectProxy& NPLObjectProxy::operator [](int nIndex) 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->CreateGetField(nIndex);
};

void NPL::NPLObjectProxy::SetField(const string& sName, const NPLObjectProxy& pObject)
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	((NPLTable*)get())->SetField(sName, pObject);
}

void NPL::NPLObjectProxy::SetField(int nIndex, const NPLObjectProxy& pObject)
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	((NPLTable*)get())->SetField(nIndex, pObject);
}

NPLObjectProxy NPLObjectProxy::GetField(const string& sName)
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
	{
		return NPLObjectProxy();
	}
	else
	{
		return ((NPLTable*)get())->GetField(sName);
	}
}


NPLObjectProxy NPLObjectProxy::GetField(const char* sName)
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
	{
		return NPLObjectProxy();
	}
	else
	{
		return ((NPLTable*)get())->GetField(sName);
	}
}

void NPLObjectProxy::MakeNil() 
{
	reset();
}

NPLObjectBase::Iterator_Type NPLObjectProxy::begin() 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->begin();
}; 

NPLObjectBase::Iterator_Type NPLObjectProxy::end() 
{
	if(get()==0 ||  GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->end();
}; 

NPLObjectBase::IndexIterator_Type NPL::NPLObjectProxy::index_begin()
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->index_begin();
}

NPLObjectBase::IndexIterator_Type NPL::NPLObjectProxy::index_end()
{
	if (get() == 0 || GetType() != NPLObjectBase::NPLObjectType_Table)
		*this = NPLObjectProxy(new NPLTable());
	return ((NPLTable*)get())->index_end();
}

/** get the type */
NPLObjectBase::NPLObjectType  NPLObjectProxy::GetType() 
{
	return (get() != 0) ? get()->GetType() : NPLObjectBase::NPLObjectType_Nil;
}
