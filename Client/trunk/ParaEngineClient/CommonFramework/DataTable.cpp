#include "ParaEngine.h"
#include "BaseTable.h"
#include "DataTable.h"

#include "memdebug.h"
using namespace ParaEngine;

int CColumnCollection::ColumnNameCounter=1;
//////////////////////////////////////////////////////////////////////////
// CDataCellItem
//////////////////////////////////////////////////////////////////////////
CDataCellItem::CDataCellItem(IObject* data,CDataRow* parent,CDataColumnInfo* columninfo):m_parent(parent),m_data(data),m_columninfo(columninfo)
{
	if (data){
		int datatype=data->GetType()->GetTypeValue();
		int columntype=columninfo->GetDataType()->GetTypeValue();
		switch(columntype){
		case Type_Bool:
			m_data=new CDataBool();
			switch(datatype){
			case Type_Bool:
				(*(CDataBool*)m_data)=(*(CDataBool*)data);
				break;
			case Type_Int:
				(*(CDataBool*)m_data)=!((*((CDataInt32*)data))==0);
				break;
			case Type_Float:
				(*(CDataBool*)m_data)=!(*((CDataFloat*)data)==0.0f);
				break;
			case Type_Double:
				(*(CDataBool*)m_data)=!(*((CDataDouble*)data)==0.0);
				break;
			case Type_String:
				(*(CDataBool*)m_data)=CBool::Parse(((string&)data).c_str());
				break;
			default:
				(*(CDataBool*)m_data)=false;
			    break;
			}
			m_bIsValid=true;
			return;
			break;
		case Type_Int:
			m_data=new CDataInt32();
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)data)==true){
					(*(CDataInt32*)m_data)=1;
				}else{
					(*(CDataInt32*)m_data)=0;
				}
				break;
			case Type_Int:
				(*(CDataInt32*)m_data)=(*((CDataInt32*)data));
				break;
			case Type_Float:
				(*(CDataInt32*)m_data)=(int)(*((CDataFloat*)data));
				break;
			case Type_Double:
				(*(CDataInt32*)m_data)=(int)(*((CDataDouble*)data));
				break;
			case Type_String:
				(*(CDataInt32*)m_data)=CInt32::Parse(((string&)data).c_str());
				break;
			default:
				(*(CDataInt32*)m_data)=0;
				break;
			}
			m_bIsValid=true;
			return;
			break;
		case Type_Float:
			m_data=new CDataFloat();
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)data)==true){
					(*(CDataFloat*)m_data)=1.0f;
				}else{
					(*(CDataFloat*)m_data)=0.0f;
				}
				break;
			case Type_Int:
				(*(CDataFloat*)m_data)=(float)(*((CDataInt32*)data));
				break;
			case Type_Float:
				(*(CDataFloat*)m_data)=(*((CDataFloat*)data));
				break;
			case Type_Double:
				(*(CDataFloat*)m_data)=(float)(*((CDataDouble*)data));
				break;
			case Type_String:
				(*(CDataFloat*)m_data)=CFloat::Parse(((string&)data).c_str());
				break;
			default:
				(*(CDataFloat*)m_data)=0.0f;
				break;
			}
			m_bIsValid=true;
			return;
			break;
		case Type_Double:
			m_data=new CDataDouble();
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)data)==true){
					(*(CDataDouble*)m_data)=1.0;
				}else{
					(*(CDataDouble*)m_data)=0.0;
				}
				break;
			case Type_Int:
				(*(CDataDouble*)m_data)=(*((CDataInt32*)data));
				break;
			case Type_Float:
				(*(CDataDouble*)m_data)=(*((CDataFloat*)data));
				break;
			case Type_Double:
				(*(CDataDouble*)m_data)=(*((CDataDouble*)data));
				break;
			case Type_String:
				(*(CDataDouble*)m_data)=CDouble::Parse(((string&)data).c_str());
				break;
			default:
				(*(CDataDouble*)m_data)=0.0;
				break;
			}
			m_bIsValid=true;
			return;
			break;
		case Type_String:
			m_data=new CDataString();
			(*(CDataString*)m_data)=data->ToString();
			m_bIsValid=true;
			return;
			break;
		}
		data->addref();
		m_bIsValid=true;
	}else if (columninfo){
		if (!columninfo->GetAllowNull()){
			OUTPUT_LOG("Assign Null to a non-null column\n");
			m_bIsValid=false;
		}else
			m_bIsValid=true;

	}else{
		m_bIsValid=false;
	}
}

CDataCellItem::~CDataCellItem()
{
	SAFE_RELEASE(m_data);
}
void CDataCellItem::Clone(IObject* obj)const
{
	CDataCellItem* pobj=(CDataCellItem*)obj;
	pobj->m_data=m_data;
	pobj->m_parent=m_parent;
	pobj->m_columninfo=m_columninfo;
	pobj->m_data->addref();
}

IObject *CDataCellItem::Clone()const
{
	CDataCellItem* obj=new CDataCellItem(m_data,m_parent,m_columninfo);
	Clone(obj);
	return obj;
}

// bool CDataCellItem::Validate()
// {
// 	try
// 	{
// 		switch(m_columninfo->GetType()->GetTypeValue()){
// 		case Type_Int:
// 			if (){
// 			}
// 			break;
// 		case Type_Bool:
// 			break;
// 		case Type_Float:
// 			break;
// 		case Type_Double:
// 			break;
// 		case Type_String:
// 			break;
// 		default:
// 			return true;
// 		    break;
// 		}
// 	}
// 	catch (CException* e)
// 	{
// 		return false;		
// 	}
// }

CDataCellItem& CDataCellItem::operator =(const string& value)
{
	m_bIsValid=true;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		(*(CDataBool*)m_data)=CBool::Parse(value.c_str());
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		(*(CDataInt32*)m_data)=CInt32::Parse(value.c_str());
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		(*(CDataFloat*)m_data)=CFloat::Parse(value.c_str());
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		(*(CDataDouble*)m_data)=CDouble::Parse(value.c_str());
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=value;
		break;
	default:
		m_bIsValid=false;

	}
	return *this;
}

CDataCellItem& CDataCellItem::operator =(int value)
{
	m_bIsValid=true;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		(*(CDataBool*)m_data)=!(value==0);
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		(*(CDataInt32*)m_data)=value;
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		(*(CDataFloat*)m_data)=(float)value;
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		(*(CDataDouble*)m_data)=(double)value;
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=CDataInt32(value).ToString();
		break;
	default:
		m_bIsValid=false;
	}
	return *this;
}

CDataCellItem& CDataCellItem::operator =(bool value)
{
	m_bIsValid=true;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		(*(CDataBool*)m_data)=value;
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		if (value==true){
			(*(CDataInt32*)m_data)=1;
		}else{
			(*(CDataInt32*)m_data)=0;
		}
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		if (value==true){
			(*(CDataFloat*)m_data)=1.0f;
		}else{
			(*(CDataFloat*)m_data)=0.0f;
		}
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		if (value==true){
			(*(CDataDouble*)m_data)=1.0;
		}else{
			(*(CDataDouble*)m_data)=0.0;
		}
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=CDataBool(value).ToString();
		break;
	default:
		m_bIsValid=false;
	}
	return *this;
}

CDataCellItem& CDataCellItem::operator =(float value)
{
	m_bIsValid=true;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		(*(CDataBool*)m_data)=!(value==0.0f);
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		(*(CDataInt32*)m_data)=(int)value;
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		(*(CDataFloat*)m_data)=value;
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		(*(CDataDouble*)m_data)=value;
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=CDataFloat(value).ToString();
		break;
	default:
		m_bIsValid=false;
	}
	return *this;
}
CDataCellItem& CDataCellItem::operator =(double value)
{
	m_bIsValid=true;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		(*(CDataBool*)m_data)=!(value==0.0);
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		(*(CDataInt32*)m_data)=(int)(value);
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		(*(CDataFloat*)m_data)=(float)(value);
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		(*(CDataDouble*)m_data)=value;
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=CDataDouble(value).ToString();
		break;
	default:
		m_bIsValid=false;
	}
	return *this;
}
CDataCellItem& CDataCellItem::operator =(IObject& value)
{
	m_bIsValid=true;
	int datatype=value.GetType()->GetTypeValue();
	int columntype=m_columninfo->GetDataType()->GetTypeValue();
	switch(columntype){
	case Type_Bool:
		if (m_data==NULL){
			m_data=new CDataBool();
		}
		switch(datatype){
		case Type_Bool:
			(*(CDataBool*)m_data)=((CDataBool&)value);
			break;
		case Type_Int:
			(*(CDataBool*)m_data)=!((((CDataInt32&)value))==0);
			break;
		case Type_Float:
			(*(CDataBool*)m_data)=!(((CDataFloat&)value)==0.0f);
			break;
		case Type_Double:
			(*(CDataBool*)m_data)=!(((CDataDouble&)value)==0.0);
			break;
		case Type_String:
			(*(CDataBool*)m_data)=CBool::Parse(((string&)value).c_str());
			break;
		default:
			(*(CDataBool*)m_data)=false;
			break;
		}
		break;
	case Type_Int:
		if (m_data==NULL){
			m_data=new CDataInt32();
		}
		switch(datatype){
		case Type_Bool:
			if (((CDataBool&)value)==true){
				(*(CDataInt32*)m_data)=1;
			}else{
				(*(CDataInt32*)m_data)=0;
			}
			break;
		case Type_Int:
			(*(CDataInt32*)m_data)=(((CDataInt32&)value));
			break;
		case Type_Float:
			(*(CDataInt32*)m_data)=(int)(((CDataFloat&)value));
			break;
		case Type_Double:
			(*(CDataInt32*)m_data)=(int)(((CDataDouble&)value));
			break;
		case Type_String:
			(*(CDataInt32*)m_data)=CInt32::Parse(((string&)value).c_str());
			break;
		default:
			(*(CDataInt32*)m_data)=0;
			break;
		}
		break;
	case Type_Float:
		if (m_data==NULL){
			m_data=new CDataFloat();
		}
		switch(datatype){
		case Type_Bool:
			if (((CDataBool&)value)==true){
				(*(CDataFloat*)m_data)=1.0f;
			}else{
				(*(CDataFloat*)m_data)=0.0f;
			}
			break;
		case Type_Int:
			(*(CDataFloat*)m_data)=(float)(((CDataInt32&)value));
			break;
		case Type_Float:
			(*(CDataFloat*)m_data)=(((CDataFloat&)value));
			break;
		case Type_Double:
			(*(CDataFloat*)m_data)=(float)(((CDataDouble&)value));
			break;
		case Type_String:
			(*(CDataFloat*)m_data)=CFloat::Parse(((string&)value).c_str());
			break;
		default:
			(*(CDataFloat*)m_data)=0.0f;
			break;
		}
		break;
	case Type_Double:
		if (m_data==NULL){
			m_data=new CDataDouble();
		}
		switch(datatype){
		case Type_Bool:
			if (((CDataBool&)value)==true){
				(*(CDataDouble*)m_data)=1.0;
			}else{
				(*(CDataDouble*)m_data)=0.0;
			}
			break;
		case Type_Int:
			(*(CDataDouble*)m_data)=(((CDataInt32&)value));
			break;
		case Type_Float:
			(*(CDataDouble*)m_data)=(((CDataFloat&)value));
			break;
		case Type_Double:
			(*(CDataDouble*)m_data)=(((CDataDouble&)value));
			break;
		case Type_String:
			(*(CDataDouble*)m_data)=CDouble::Parse(((string&)value).c_str());
			break;
		default:
			(*(CDataDouble*)m_data)=0.0;
			break;
		}
		break;
	case Type_String:
		if (m_data==NULL){
			m_data=new CDataString();
		}
		(*(CDataString*)m_data)=value.ToString();
		break;
	default:
		switch(datatype){
		case Type_Bool:
		case Type_Int:
		case Type_Float:
		case Type_Double:
		case Type_String:
			m_bIsValid=false;
			break;
		default:
			value.addref();
			m_data=&value;
			m_bIsValid=true;
		}
	}

	return *this;
}

CDataCellItem& CDataCellItem::operator =(CDataCellItem* value)
{
	m_bIsValid=true;
	if (value==NULL){
		SetNull();
		return *this;
	}
	if (value->m_columninfo->GetDataType()!=m_columninfo->GetDataType()){
		int datatype=value->m_columninfo->GetDataType()->GetTypeValue();
		int columntype=m_columninfo->GetDataType()->GetTypeValue();
		switch(columntype){
		case Type_Bool:
			if (m_data==NULL){
				m_data=new CDataBool();
			}
			switch(datatype){
			case Type_Bool:
				(*(CDataBool*)m_data)=(*(CDataBool*)value->m_data);
				break;
			case Type_Int:
				(*(CDataBool*)m_data)=!((*((CDataInt32*)value->m_data))==0);
				break;
			case Type_Float:
				(*(CDataBool*)m_data)=!(*((CDataFloat*)value->m_data)==0.0f);
				break;
			case Type_Double:
				(*(CDataBool*)m_data)=!(*((CDataDouble*)value->m_data)==0.0);
				break;
			case Type_String:
				(*(CDataBool*)m_data)=CBool::Parse(((string&)value->m_data).c_str());
				break;
			default:
				(*(CDataBool*)m_data)=false;
				break;
			}
			break;
		case Type_Int:
			if (m_data==NULL){
				m_data=new CDataInt32();
			}
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)value->m_data)==true){
					(*(CDataInt32*)m_data)=1;
				}else{
					(*(CDataInt32*)m_data)=0;
				}
				break;
			case Type_Int:
				(*(CDataInt32*)m_data)=(*((CDataInt32*)value->m_data));
				break;
			case Type_Float:
				(*(CDataInt32*)m_data)=(int)(*((CDataFloat*)value->m_data));
				break;
			case Type_Double:
				(*(CDataInt32*)m_data)=(int)(*((CDataDouble*)value->m_data));
				break;
			case Type_String:
				(*(CDataInt32*)m_data)=CInt32::Parse(((string&)value->m_data).c_str());
				break;
			default:
				(*(CDataInt32*)m_data)=0;
				break;
			}
			break;
		case Type_Float:
			if (m_data==NULL){
				m_data=new CDataFloat();
			}
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)value->m_data)==true){
					(*(CDataFloat*)m_data)=1.0f;
				}else{
					(*(CDataFloat*)m_data)=0.0f;
				}
				break;
			case Type_Int:
				(*(CDataFloat*)m_data)=(float)(*((CDataInt32*)value->m_data));
				break;
			case Type_Float:
				(*(CDataFloat*)m_data)=(*((CDataFloat*)value->m_data));
				break;
			case Type_Double:
				(*(CDataFloat*)m_data)=(float)(*((CDataDouble*)value->m_data));
				break;
			case Type_String:
				(*(CDataFloat*)m_data)=CFloat::Parse(((string&)value->m_data).c_str());
				break;
			default:
				(*(CDataFloat*)m_data)=0.0f;
				break;
			}
			break;
		case Type_Double:
			if (m_data==NULL){
				m_data=new CDataDouble();
			}
			switch(datatype){
			case Type_Bool:
				if ((*(CDataBool*)value->m_data)==true){
					(*(CDataDouble*)m_data)=1.0;
				}else{
					(*(CDataDouble*)m_data)=0.0;
				}
				break;
			case Type_Int:
				(*(CDataDouble*)m_data)=(*((CDataInt32*)value->m_data));
				break;
			case Type_Float:
				(*(CDataDouble*)m_data)=(*((CDataFloat*)value->m_data));
				break;
			case Type_Double:
				(*(CDataDouble*)m_data)=(*((CDataDouble*)value->m_data));
				break;
			case Type_String:
				(*(CDataDouble*)m_data)=CDouble::Parse(((string&)value->m_data).c_str());
				break;
			default:
				(*(CDataDouble*)m_data)=0.0;
				break;
			}
			break;
		case Type_String:
			if (m_data==NULL){
				m_data=new CDataString();
			}
			(*(CDataString*)m_data)=value->m_data->ToString();
			break;
		default:
			switch(datatype){
			case Type_Bool:
			case Type_Int:
			case Type_Float:
			case Type_Double:
			case Type_String:
				m_bIsValid=false;
				break;
			default:
				m_data->addref();
				m_data=value->m_data;
				m_bIsValid=true;
			}
		}
	}
	return *this;
}


CDataCellItem::operator bool()
{
	bool re=false;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		re=(*(CDataBool*)m_data);
		break;
	case Type_Int:
		re=!((*((CDataInt32*)m_data))==0);
		break;
	case Type_Float:
		re=!(*((CDataFloat*)m_data)==0.0f);
		break;
	case Type_Double:
		re=!(*((CDataDouble*)m_data)==0.0);
		break;
	case Type_String:
		re=CBool::Parse(((string&)m_data).c_str());
		break;
	}
	return re;
}

CDataCellItem::operator int()
{
	int re=0;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if ((*(CDataBool*)m_data)==true){
			re=1;
		}else{
			re=0;
		}
		break;
	case Type_Int:
		re=(*((CDataInt32*)m_data));
		break;
	case Type_Float:
		re=(int)(*((CDataFloat*)m_data));
		break;
	case Type_Double:
		re=(int)(*((CDataDouble*)m_data));
		break;
	case Type_String:
		re=CInt32::Parse(((string&)m_data).c_str());
		break;
	}
	return re;

}
CDataCellItem::operator float()
{
	float re=0.0f;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if ((*(CDataBool*)m_data)==true){
			re=1.0f;
		}else{
			re=0.0f;
		}
		break;
	case Type_Int:
		re=(float)(*((CDataInt32*)m_data));
		break;
	case Type_Float:
		re=(*((CDataFloat*)m_data));
		break;
	case Type_Double:
		re=(float)(*((CDataDouble*)m_data));
		break;
	case Type_String:
		re=CFloat::Parse(((string&)m_data).c_str());
		break;
	}
	return re;

}

CDataCellItem::operator double()
{
	double re=0.0;
	switch(m_columninfo->GetDataType()->GetTypeValue()){
	case Type_Bool:
		if ((*(CDataBool*)m_data)==true){
			re=1.0;
		}else{
			re=0.0;
		}
		break;
	case Type_Int:
		re=(*((CDataInt32*)m_data));
		break;
	case Type_Float:
		re=(*((CDataFloat*)m_data));
		break;
	case Type_Double:
		re=(*((CDataDouble*)m_data));
		break;
	case Type_String:
		re=CDouble::Parse(((string&)m_data).c_str());
		break;
	}
	return re;

}

CDataCellItem::operator string()
{
	return m_data->ToString();
}

CDataCellItem::operator const char *()
{
	const char* re=NULL;
	re=m_data->ToString().c_str();
	return re;
}

CDataCellItem::operator IObject*()
{
	return m_data;
}

bool CDataCellItem::SetNull()
{
	if (m_columninfo->GetAllowNull()){
		SAFE_RELEASE(m_data);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
// CDataColumnInfo
//////////////////////////////////////////////////////////////////////////
CDataColumnInfo::CDataColumnInfo(const string& szColumnName):m_bReadonly(false),m_bAllowNull(true),m_nMaxLength(-1),m_parent(NULL),m_datatype(IType::GetType("string"))
{
	m_szColumnName=szColumnName;
	m_type=IType::GetType("void");
}
CDataColumnInfo::CDataColumnInfo(const string&szColumnName,IType* ptype):m_bReadonly(false),m_bAllowNull(true),m_nMaxLength(-1),m_parent(NULL)
{
	m_szColumnName=szColumnName;
	m_datatype=ptype;
	m_type=IType::GetType("void");
}

IObject*  CDataColumnInfo::Clone()const
{	CDataColumnInfo* pobj=new CDataColumnInfo();
	Clone(pobj);
	return pobj;
}

void CDataColumnInfo::Clone(IObject *obj)const
{
	CDataColumnInfo* pobj=(CDataColumnInfo*)obj;
	IObject::Clone(pobj);
	pobj->m_parent=NULL;
	pobj->m_szCaption=m_szCaption;
	pobj->m_szColumnName=m_szColumnName;
}

const char* CDataColumnInfo::GetCaption()const
{
	if (m_szCaption.size()==0){
		return GetColumnName();
	}else
		return m_szCaption.c_str();
}

void CDataColumnInfo::SetCaption(const char* szText)
{
	if (szText==NULL||szText[0]=='\0'){
		m_szCaption.clear();
		return;
	}
	m_szCaption=szText;
}

//////////////////////////////////////////////////////////////////////////
// CDataRow
//////////////////////////////////////////////////////////////////////////
CDataRow::CDataRow():m_eRowState(DataRow_Detached),m_bEditing(false)
{
	CBaseRowColumn(-1,Table_Row,false);
	m_ColumnInfo=new CColumnCollection(this);
}

CDataRow::~CDataRow()
{
	SAFE_DELETE(m_ColumnInfo);
}

IObject* CDataRow::Clone()const
{
	CDataRow* obj=new CDataRow();
	Clone(obj);
	return obj;
}

void CDataRow::Clone(IObject *obj)const
{
	CBaseRowColumn::Clone(obj);
	CDataRow *pobj=(CDataRow*)obj;
	pobj->m_ColumnInfo->Clear();
	for (int a=0;a<m_ColumnInfo->Size();a++){
		pobj->m_ColumnInfo->Add((CDataColumnInfo*)m_ColumnInfo->GetAt(a));
	}
	pobj->m_children.Clear();
	for (int a=0;a<m_children.Size();a++){
		pobj->m_children.Add((CDataRow*)m_children.GetAt(a));
	}
	pobj->m_bEditing=m_bEditing;
	pobj->m_eRowState=DataRow_Detached;
}
void CDataRow::Delete()
{

}
int CDataRow::EndEdit()
{
	for (int a=0;a<m_data.Size();a++){

	}
	m_bEditing=false;
	return 0;
}
ICollection<CDataRow>* CDataRow::GetChildRows()
{
	return &m_children;
}

int CDataRow::SetAt(IObject&obj, int index)
{
	if (m_bEditing){
		return -1;
	}
	int re;
	CDataColumnInfo* temp=m_ColumnInfo->GetAt(index);
	if (temp==NULL&&m_parent!=NULL){
		temp=((CDataTable*)m_parent)->GetColumnInfos()->GetAt(index);
	}
	if (temp==NULL){
		OUTPUT_LOG("Column of %d is not defined\n",index);
		return -1;
	}
	CDataCellItem cell(&obj,this,temp);
	if (!cell.IsValid()){
		OUTPUT_LOG("Input type doesn't match column %d type\n",index);
		return -1;
	}
	
	if ((re=CBaseRowColumn::SetAt(cell,index))==index){
		m_eRowState|=DataRow_Modified;
	}
	return re;
}

int CDataRow::Resize(int newsize)
{
	if (m_bEditing){
		return -1;
	}
	int re=newsize;
	if (newsize!=Size()){
		if ((re=CBaseRowColumn::Resize(newsize))==newsize){
			m_eRowState|=DataRow_Modified;
		}
	}
	return re;
}

const CDataCellItem* CDataRow::GetAt(int index)const
{
	return (const CDataCellItem*)CBaseRowColumn::GetAt(index);
}

CDataCellItem* CDataRow::GetAt(int index)
{
	return (CDataCellItem*)CBaseRowColumn::GetAt(index);
}


const CDataCellItem& CDataRow::operator [](const string& szName)const
{
	int index=m_ColumnInfo->IndexOf(szName);
	if (index==-1){
		if (m_parent){
			index=((CDataTable*)m_parent)->GetColumnInfos()->IndexOf(szName);
		}
	}
	if (index!=-1){
		return *((const CDataCellItem*)m_data.GetAt(index));
	}
	throw("Column does not exist\n");
}
CDataCellItem& CDataRow::operator [](const string& szName)
{
	int index=m_ColumnInfo->IndexOf(szName);
	if (index==-1){
		if (m_parent){
			index=((CDataTable*)m_parent)->GetColumnInfos()->IndexOf(szName);
		}
	}
	if (index!=-1){
		return *((CDataCellItem*)m_data.GetAt(index));
	}
	throw("Column does not exist\n");
}
//////////////////////////////////////////////////////////////////////////
// CColumnCollection
//////////////////////////////////////////////////////////////////////////
CDataColumnInfo* CColumnCollection::GetAt(int index)
{
	return m_data.GetAt(index);
}
const CDataColumnInfo* CColumnCollection::GetAt(int index)const
{
	return m_data.GetAt(index);
}
CDataColumnInfo* CColumnCollection::GetAt(const string& szName)
{
	map<string,int>::const_iterator iter;
	int index;
	if ((iter=m_map.find(szName))!=m_map.end()){
		index=iter->second;
		return GetAt(index);
	}
	return NULL;
}
const CDataColumnInfo* CColumnCollection::GetAt(const string& szName)const
{
	map<string,int>::const_iterator iter;
	int index;
	if ((iter=m_map.find(szName))!=m_map.end()){
		index=iter->second;
		return GetAt(index);
	}
	return NULL;
}
void CColumnCollection::Clear()
{
	m_data.Clear();
	m_map.clear();
	if(m_parent){
		m_parent->Clear();
	}
	if (m_row){
		m_row->Clear();
	}
}
/**
* delete a column and delete
*/
int CColumnCollection::Remove(int index)
{
	CDataColumnInfo* temp=m_data.GetAt(index);
	if (temp!=NULL){
		m_map.erase(temp->GetColumnName());
		if (m_parent){
			m_parent->DeleteColumn(index);
		}
		if (m_row){
			m_row->Remove(index);
		}
		return m_data.Size();
	}else
		return -1;
}
int CColumnCollection::IndexOf(const string& szName)const
{
	map<string,int>::const_iterator iter;
	if ((iter=m_map.find(szName))!=m_map.end()){
		return iter->second;
	}else 
		return -1;
};

void CColumnCollection::Add()
{
	char temp[20];
	string temp1="Column";
	itoa(CColumnCollection::ColumnNameCounter++,temp,10);
	temp1+=temp;
	CDataColumnInfo* newInfo=new CDataColumnInfo(temp1,IType::GetType("string"));
	Add(newInfo);
	SAFE_RELEASE(newInfo);
}
void CColumnCollection::Add(CDataColumnInfo *obj)
{
	if (obj){
		const char* str=obj->GetColumnName();
		if (strlen(str)==0){
			return;
		}
		if (m_map.find(str)!=m_map.end()){
			throw("Duplicate Column\n");
			return;
		}
		obj->m_parent=m_parent;
		m_map[str]=m_data.Size();
		m_data.Add(obj);
	}
}
/**
* Defualt type is string
* 
*/
void CColumnCollection::Add(const char* szName)
{
	string temp1;
	if (szName==NULL){
		char temp[20];
		temp1="Column";
		itoa(CColumnCollection::ColumnNameCounter++,temp,10);
		temp1+=temp;

	}else{
		temp1=szName;
	}
	CDataColumnInfo* newInfo=new CDataColumnInfo(temp1,IType::GetType("string"));
	Add(newInfo);
	SAFE_RELEASE(newInfo);

}
void CColumnCollection::Add(const char* szName,IType* pType)
{
	string temp1;
	if (szName==NULL){
		char temp[20];
		temp1="Column";
		itoa(CColumnCollection::ColumnNameCounter++,temp,10);
		temp1+=temp;

	}else{
		temp1=szName;
	}
	CDataColumnInfo* newInfo;
	if (pType==NULL){
		newInfo=new CDataColumnInfo(temp1,IType::GetType("string"));
	}else
		newInfo=new CDataColumnInfo(temp1,pType);
	Add(newInfo);
	SAFE_RELEASE(newInfo);

}

//////////////////////////////////////////////////////////////////////////
// CDataTable
//////////////////////////////////////////////////////////////////////////
CDataTable::CDataTable()
{
	CBaseTable();
	m_ColumnInfo=new CColumnCollection(this);
}

CDataTable::~CDataTable()
{
	SAFE_DELETE(m_ColumnInfo);
}