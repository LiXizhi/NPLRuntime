//----------------------------------------------------------------------
// Authors:	Liu Weili
// Date:	2006.5.26
//
// desc: 
// Contains many types and a type manager.
// IType is the interface for all types. CXXXX types are base types for conversion and identification
// CDataXXX types are the actual data container.
// CTypeManager is a singleton. use CSingleton to access it.
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "Type.h"
#include "CSingleton.h"
#include "StringHelper.h"

#include "memdebug.h"

using namespace ParaEngine;
const char CVoid::TypeName[]="void";
const char CInt32::TypeName[]="int";
const int CInt32::MaxValue=2147483647;
const int CInt32::MinValue=-2147483647;
const char CFloat::TypeName[]="float";
//const float CFloat::MinValue=-3.40282347E+38;
//const float CFloat::MaxValue=3.40282347E+38;
const char CDouble::TypeName[]="double";
const double CDouble::MinValue=-1.7976931348623157E+308;
const double CDouble::MaxValue=1.7976931348623157E+308;
const char CBool::TypeName[]="bool";
const char CStr::TypeName[]="string";

const IType* CDataInt32::m_type=NULL;
const IType* CDataBool::m_type=NULL;
const IType* CDataFloat::m_type=NULL;
const IType* CDataDouble::m_type=NULL;
const IType* CDataString::m_type=NULL;

const char CGUIRootType::TypeName[]="guiroot";
const char CGUITextType::TypeName[]="guitext";
const char CGUIButtonType::TypeName[]="guibutton";
const char CGUISliderType::TypeName[]="guislider";
const char CGUIVideoType::TypeName[]="guivideo";
const char CGUIEditBoxType::TypeName[]="guieditbox";
const char CGUIIMEEditBoxType::TypeName[]="guiimeeditbox";
const char CGUIToolTipType::TypeName[]="guitooltip";
const char CGUIPainterType::TypeName[]="guipainter";
const char CGUIGridType::TypeName[]="guigrid";
const char CGUIContainerType::TypeName[]="guicontainer";
const char CGUIScrollBarType::TypeName[]="guiscrollbar";
const char CGUIListBoxType::TypeName[]="guilistbox";
const char CGUICanvasType::TypeName[]="guicanvas";
const char CGUIWebBrowserType::TypeName[]="guiwebbrowser";

//////////////////////////////////////////////////////////////////////////
// CTypeManager
//////////////////////////////////////////////////////////////////////////
CTypeManager::CTypeManager()
{
	IType* obj;
	obj=new CVoid();
	SetType(obj);
	obj=new CInt32();
	SetType(obj);
	obj=new CFloat();
	SetType(obj);
	obj=new CDouble();
	SetType(obj);
	obj=new CGUIRootType();
	SetType(obj);
	obj=new CGUITextType();
	SetType(obj);
	obj=new CGUIButtonType();
	SetType(obj);
	obj=new CGUISliderType();
	SetType(obj);
	obj=new CGUIVideoType();
	SetType(obj);
	obj=new CGUIEditBoxType();
	SetType(obj);
	obj=new CGUIIMEEditBoxType();
	SetType(obj);
	obj=new CGUIToolTipType();
	SetType(obj);
	obj=new CGUIPainterType();
	SetType(obj);
	obj=new CGUIGridType();
	SetType(obj);
	obj=new CGUIContainerType();
	SetType(obj);
	obj=new CGUIListBoxType();
	SetType(obj);
	obj=new CGUIScrollBarType();
	SetType(obj);
	obj=new CGUICanvasType();
	SetType(obj);
	obj=new CGUIWebBrowserType();
	SetType(obj);
}

CTypeManager::~CTypeManager()
{
	map<string,IType*>::iterator iter,iterend=m_types.end();
	for (iter=m_types.begin();iter!=iterend;iter++){
		delete iter->second;
	}
}

IType* CTypeManager::GetType(const char* szTypeName)const
{
	map<string,IType*>::const_iterator iter;
	if ((iter=m_types.find(szTypeName))!=m_types.end()){
		return iter->second;
	}
	return NULL;
}

bool CTypeManager::SetType(IType* pType)
{
	const char* name=pType->GetTypeName();
	if (m_types.find(name)!=m_types.end()){
		return false;
	}
	m_types[name]=pType;
	return true;
}

void CTypeManager::ReleaseType(IType* pType)
{
	if(pType) { 
		delete (pType); 
	}
}
//////////////////////////////////////////////////////////////////////////
// IType
//////////////////////////////////////////////////////////////////////////
IType* IType::GetType(const char* szTypeName)
{
	CTypeManager* tm=&CSingleton<CTypeManager>::Instance();
	return tm->GetType(szTypeName);
}

IType* IType::GetType()const
{
	switch(m_etype){
	case Type_Int:
		return IType::GetType("int");
		break;
	case Type_Float:
		return IType::GetType("float");
		break;
	case Type_Bool:
		return IType::GetType("bool");
		break;
	case Type_Double:
		return IType::GetType("double");
		break;
	case Type_String:
		return IType::GetType("string");
		break;
	default:
		return IType::GetType("void");
	    break;
	}
}

bool ParaEngine::IType::operator==(const char *szTypeName) const
{
	if (szTypeName){
		return strcmp(GetTypeName(), szTypeName) == 0;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// CBool
//////////////////////////////////////////////////////////////////////////
bool CBool::Parse(const char* input)
{
	if (input==NULL){
		OUTPUT_LOG("CBool::Parse error: input==NULL");
		return false;
	}
	if (strcmp(input,"false")==0){
		return false;
	}else if(strcmp(input,"true")==0){
		return true;
	}else{
		OUTPUT_LOG("CBool::Parse error: not valid boolean value");
		return false;
	}
}

int ParaEngine::CBool::Verify(const char* input) const
{
	if (input == NULL){
		return Verify_Unknown;
	}
	if (strcmp(input, "false") == 0 || strcmp(input, "true") == 0){
		return Verify_Success;
	}
	return Verify_Unknown;
}

//////////////////////////////////////////////////////////////////////////
// CInt32
//////////////////////////////////////////////////////////////////////////
int CInt32::Parse(const char* input)
{
	if (input==NULL){
		OUTPUT_LOG("CInt32::Parse error: input==NULL");
		return 0;
	}
	return atoi(input);
}

int ParaEngine::CInt32::Verify(const char* input) const
{
	if (input == NULL){
		return Verify_Unknown;
	}
	int re = Verify_Success;
#ifdef PARAENGINE_CLIENT
	long long temp = _atoi64(input);
	if (temp > MaxValue){
		re = Verify_Overflow;
	}
	else if (temp < MinValue){
		re = Verify_Underflow;
	}
#endif
	return re;
}

//////////////////////////////////////////////////////////////////////////
// CFloat
//////////////////////////////////////////////////////////////////////////
float CFloat::Parse(const char* input)
{
	if (input==NULL){
		OUTPUT_LOG("CFloat::Parse error: input==NULL");
		return 0;
	}
	return (float)atof(input);
}

int ParaEngine::CFloat::Verify(const char* input) const
{
	if (input == NULL){
		return Verify_Unknown;
	}
	int re = Verify_Success;
	double temp = atof(input);
	/*if (temp>MaxValue){
		re=Verify_Overflow;
		}else if (temp<MinValue){
		re=Verify_Underflow;
		}*/
	return re;
}

//////////////////////////////////////////////////////////////////////////
// CDouble
//////////////////////////////////////////////////////////////////////////
double CDouble::Parse(const char* input)
{
	if (input==NULL){
		OUTPUT_LOG("CDouble::Parse error: input==NULL");
		return 0;
	}
	return atof(input);
}

int ParaEngine::CDouble::Verify(const char* input) const
{
	if (input == NULL){
		return Verify_Unknown;
	}
	int re = Verify_Success;
#ifdef PARAENGINE_CLIENT
	double temp = atof(input);
	int sign = _fpclass(temp);
	if (temp == _FPCLASS_PINF){
		re = Verify_Overflow;
	}
	else if (temp == _FPCLASS_NINF){
		re = Verify_Underflow;
	}
#endif
	return re;
}

//////////////////////////////////////////////////////////////////////////
// CDataBool
//////////////////////////////////////////////////////////////////////////
string CDataBool::ToString()const
{
	if (m_data){
		return "true";
	}else
		return "false";
}

//////////////////////////////////////////////////////////////////////////
// CDataInt32
//////////////////////////////////////////////////////////////////////////
string CDataInt32::ToString()const
{
	char temp[30];
	itoa(m_data,temp,10);
	return temp;
}

//////////////////////////////////////////////////////////////////////////
// CDataFloat
//////////////////////////////////////////////////////////////////////////
string CDataFloat::ToString()const
{
	char temp[255];
	_gcvt(m_data,6,temp);
	return temp;
}

//////////////////////////////////////////////////////////////////////////
// CDataDouble
//////////////////////////////////////////////////////////////////////////
string CDataDouble::ToString()const
{
	char temp[20];
	_gcvt(m_data,6,temp);
	return temp;
}

//////////////////////////////////////////////////////////////////////////
// CDataString
//////////////////////////////////////////////////////////////////////////
string CDataString::ToString()const
{
	return m_data;
}
CDataString& CDataString::operator =(const wstring& data)
{
	m_data=StringHelper::WideCharToMultiByte(data.c_str(),DEFAULT_GUI_ENCODING);
	return *this;
}

CDataString& CDataString::operator =(const WCHAR* data)
{
	if (data==NULL){
		m_data="";
		return *this;
	}
	m_data=StringHelper::WideCharToMultiByte(data, DEFAULT_GUI_ENCODING);
	return *this;
}

CDataString::operator const wstring()const
{
	wstring temp=StringHelper::MultiByteToWideChar(m_data.c_str(), DEFAULT_GUI_ENCODING);
	return temp;
}

bool CDataString::operator ==(const wstring& value)const
{
	bool re=(m_data==StringHelper::WideCharToMultiByte(value.c_str(), DEFAULT_GUI_ENCODING));
	return re;
}

CDataString& ParaEngine::CDataString::operator=(const char* data)
{
	if (data == NULL){
		m_data = "";
		return *this;
	}
	m_data = data;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// CDouble
//////////////////////////////////////////////////////////////////////////

int ParaEngine::CStr::Verify(const char* input) const
{
	if (input == NULL){
		return Verify_Unknown;
	}
	return Verify_Success;
}
