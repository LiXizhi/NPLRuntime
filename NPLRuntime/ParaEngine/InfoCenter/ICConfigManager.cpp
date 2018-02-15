//----------------------------------------------------------------------
// Class:	CICConfigManager
// Authors:	Liu Weili
// Date:	2006.3.2
//
// desc: 
// This class manages all the configuration of the entire engine. One can load configurations from a LUA file using this class.
// The configurations are accessable with read-write access in runtime. It's also designed to be thread-safe. 
// The configurations can be written back to disk at anytime. 
//
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ICConfigManager.h"
#include "NPLRuntime.h"

using namespace NPL;
using namespace ParaInfoCenter;
using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// CICConfigItem class
//////////////////////////////////////////////////////////////////////////
CICConfigItem::CICConfigItem(const char* value)
{
	etype=IC_CONFIG_TEXT;
	sData=NULL;
	SetValue(value);
}

CICConfigItem::CICConfigItem(int value)
{
	etype=IC_CONFIG_INT;
	iData=value;
}

CICConfigItem::CICConfigItem(double value)
{
	etype=IC_CONFIG_FLOAT;
	fData=value;
}

CICConfigItem::~CICConfigItem()
{
	if (etype==IC_CONFIG_TEXT) {
		SAFE_DELETE(sData);
	}
}

void CICConfigItem::SetValue(const char* value)
{
	if (etype==IC_CONFIG_TEXT) {
		SAFE_DELETE(sData);
		size_t len=strlen(value);
		sData=new char[len+1];
		strncpy(sData,value,len);
		sData[len]='\0';
	}
}
//////////////////////////////////////////////////////////////////////////
// CICConfigItems class
//////////////////////////////////////////////////////////////////////////
CICConfigItems::~CICConfigItems()
{
	vector<CICConfigItem*>::iterator iter=items.begin();
	vector<CICConfigItem*>::iterator iterend=items.end();
	for (;iter!=iterend;iter++) {
		SAFE_DELETE(*iter);
	}

}
//////////////////////////////////////////////////////////////////////////
// CICConfigManager class
//////////////////////////////////////////////////////////////////////////
CICConfigManager::CICConfigManager():m_bOverwrite(true),m_bLocked(false)
{
}

CICConfigManager::~CICConfigManager()
{
	map<string,CICConfigItems*>::iterator iter=m_items.begin();
	map<string,CICConfigItems*>::iterator iterend=m_items.end();
	for (;iter!=iterend;iter++) {
		SAFE_DELETE(iter->second);
	}
}

CICConfigManager* CICConfigManager::GetInstance()
{
	static CICConfigManager g_instance;
	return &g_instance;
}

void CICConfigManager::Lock()
{
	ParaEngine::Lock lock_(m_csFunc);
	m_csLock.lock();
	m_bLocked=true;
}


void CICConfigManager::UnLock()
{
	ParaEngine::Lock lock_(m_csFunc);
	//avoid other thread to leave the critical section
	if (m_bLocked) {
		m_csLock.unlock();
		m_bLocked=false;
	}
}
HRESULT CICConfigManager::GetSize(const char* szName, DWORD *nSize)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);

	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			*nSize=(DWORD)iter->second->items.size();
		}else{
			*nSize=0;
		}
	}

	return hr;
}
HRESULT CICConfigManager::SetIntValue(const char* szName,int value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			if (m_bOverwrite) {
				temp=iter->second;
				char ctemp[30];//as long as the translate function is correct, this buffer will not overrun in 32-bit or 64-bit data types.
				if (index<0||index>=(int)temp->items.size()) {
					hr=E_INVALIDARG;
				}else{
					switch(temp->items[index]->etype) {
					case IC_CONFIG_TEXT:
						itoa(value,ctemp,10);
						temp->items[index]->SetValue(ctemp);
						break;
					case IC_CONFIG_INT:
						temp->items[index]->iData=value;
						break;
					case IC_CONFIG_FLOAT:
						temp->items[index]->fData=value;
						break;
					}
				}
			}else{
				hr=E_FAIL;
			}
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			temp1=new CICConfigItem(value);
			temp->items.push_back(temp1);
			m_items[szName]=temp;
		}
	}

	return hr;
}
HRESULT CICConfigManager::AppendIntValue(const char* szName,int value)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			temp=iter->second;
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			m_items[szName]=temp;
		}
		temp1=new CICConfigItem(value);
		temp->items.push_back(temp1);
	}

	return hr;
}

HRESULT CICConfigManager::SetDoubleValue(const char* szName,double value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			if (m_bOverwrite) {
				temp=iter->second;
				char ctemp[30];//as long as the translate function is correct, this buffer will not overrun in 32-bit or 64-bit data types.
				if (index<0||index>=(int)temp->items.size()) {
					hr=E_INVALIDARG;
				}else{
					switch(temp->items[index]->etype) {
					case IC_CONFIG_TEXT:
						_gcvt(value,20,ctemp);
						temp->items[index]->SetValue(ctemp);
						break;
					case IC_CONFIG_INT:
						temp->items[index]->iData=(int)value;
						break;
					case IC_CONFIG_FLOAT:
						temp->items[index]->fData=value;
						break;
					}
				}
			}else{
				hr=E_FAIL;
			}
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			temp1=new CICConfigItem(value);
			temp->items.push_back(temp1);
			m_items[szName]=temp;
		}
	}

	return hr;
}
HRESULT CICConfigManager::AppendDoubleValue(const char* szName,double value)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			temp=iter->second;
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			m_items[szName]=temp;
		}
		temp1=new CICConfigItem(value);
		temp->items.push_back(temp1);
	}

	return hr;
}
HRESULT CICConfigManager::SetTextValue(const char* szName,const char* value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			if (m_bOverwrite) {
				temp=iter->second;
				if (index<0||index>=(int)temp->items.size()) {
					hr=E_INVALIDARG;
				}else{
					switch(temp->items[index]->etype) {
					case IC_CONFIG_TEXT:
						temp->items[index]->SetValue(value);
						break;
					default:
						hr=E_FAIL;
					}
				}
			}else{
				hr=E_FAIL;
			}
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			temp1=new CICConfigItem(value);
			temp->items.push_back(temp1);
			m_items[szName]=temp;
		}
	}

	return hr;
}

HRESULT CICConfigManager::AppendTextValue(const char* szName,const char* value)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::iterator iter;
		CICConfigItems* temp;
		CICConfigItem* temp1;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name already exists
			temp=iter->second;
		}else{
			//if the name does not exist, create a new item.
			temp=new CICConfigItems();
			m_items[szName]=temp;
		}
		temp1=new CICConfigItem(value);
		temp->items.push_back(temp1);
	}
	return hr;
}

HRESULT CICConfigManager::GetIntValue(const char* szName,int *value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL||value==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::const_iterator iter;
		const CICConfigItems* temp;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name exists
			temp=iter->second;
			if (index<0||index>=(int)temp->items.size()) {
				hr=E_INVALIDARG;
			}else{
				switch(temp->items[index]->etype) {
				case IC_CONFIG_INT:
					*value=temp->items[index]->iData;
					break;
				case IC_CONFIG_FLOAT:
					*value=(int)temp->items[index]->fData;
					break;
				default:
					hr=E_FAIL;
				}
			}
		}else{
			//if the name does not exists, 
			hr=E_INVALIDARG;
		}
	}
	return hr;
}

HRESULT CICConfigManager::GetDoubleValue(const char* szName,double *value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL||value==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::const_iterator iter;
		const CICConfigItems* temp;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name exists
			temp=iter->second;
			if (index<0||index>=(int)temp->items.size()) {
				hr=E_INVALIDARG;
			}else{
				switch(temp->items[index]->etype) {
				case IC_CONFIG_INT:
					*value=(double)temp->items[index]->iData;
					break;
				case IC_CONFIG_FLOAT:
					*value=temp->items[index]->fData;
					break;
				default:
					hr=E_FAIL;
				}
			}
		}else{
			//if the name does not exists, 
			hr=E_INVALIDARG;
		}
	}

	return hr;
}

HRESULT CICConfigManager::GetTextValue(const char* szName,string &value, int index)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else if (szName==NULL) {
		hr= E_INVALIDARG;
	}else{
		map<string,CICConfigItems*>::const_iterator iter;
		const CICConfigItems* temp;
		if ((iter=m_items.find(szName))!=m_items.end()) {
			//if the name exists
			temp=iter->second;
			char ctemp[30];//as long as the translate function is correct, this buffer will not overrun in 32-bit or 64-bit data types.
			if (index<0||index>=(int)temp->items.size()) {
				hr=E_INVALIDARG;
			}else{
				switch(temp->items[index]->etype) {
				case IC_CONFIG_INT:
					itoa(temp->items[index]->iData,ctemp,10);
					value=ctemp;
					break;
				case IC_CONFIG_FLOAT:
					_gcvt(temp->items[index]->fData,20,ctemp);
					value=ctemp;
					break;
				case IC_CONFIG_TEXT:
					value=temp->items[index]->sData;
				}
			}
		}else{
			//if the name does not exists, 
			hr=E_INVALIDARG;
		}
	}

	return hr;
}

HRESULT CICConfigManager::SaveToFile(const char* szFilename/* =NULL */,bool bOverwrite/* =true */)
{
	HRESULT hr=S_OK;
	ParaEngine::Lock lock_(m_csFunc);
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else{
		string filename;
		if (szFilename==NULL) {
			filename="script/config.lua";
		}else{
			filename=szFilename;
		}
		FILE *file=fopen(filename.c_str(),"r");
		if (file!=NULL&&bOverwrite==false) {
			hr=E_FAIL;
		}else{
			if (file!=NULL) {
				fclose(file);
			}
			if ((file=fopen(filename.c_str(),"w+"))==NULL) {
				hr=E_FAIL;
			}else{
				map<string,CICConfigItems*>::const_iterator iter=m_items.begin();
				map<string,CICConfigItems*>::const_iterator iterend=m_items.end();
				CICConfigItems *temp;
				string result;
				DWORD a;
				char ctemp[30];//as long as the translate function is correct, this buffer will not overrun in 32-bit or 64-bit data types.
				for (;iter!=iterend;iter++) {
					temp=iter->second;
					if (temp!=NULL) {
						for (a=0;a<temp->items.size();a++) {
							switch(temp->items[a]->GetType()) {
							case IC_CONFIG_INT:
								itoa(temp->items[a]->iData,ctemp,10);
								if (a>0) {
									result+="Config.AppendIntValue(\"";
								}else
									result+="Config.SetIntValue(\"";
								result+=iter->first;
								result+="\",";
								result+=ctemp;
								result+=");\n";
								break;
							case IC_CONFIG_FLOAT:
								_gcvt(temp->items[a]->fData,20,ctemp);
								if (a>0) {
									result+="Config.AppendDoubleValue(\"";
								}else
									result+="Config.SetDoubleValue(\"";
								result+=iter->first;
								result+="\",";
								result+=ctemp;
								result+=");\n";
								break;
							case IC_CONFIG_TEXT:
								if (a>0) {
									result+="Config.AppendTextValue(\"";
								}else
									result+="Config.SetTextValue(\"";
								result+=iter->first;
								result+="\",\"";
								result+=temp->items[a]->sData;
								result+="\");\n";
								break;
							}
						}
					}
				}
				if(fwrite((void*)result.c_str(),1,result.size(),file)!=result.size()){
					hr=E_FAIL;
				}
				fclose(file);
			}
		}
	}

	return hr;
}

HRESULT CICConfigManager::LoadFromFile(const char* szFilename/* =NULL */,bool bOverwrite/* =true */)
{
	HRESULT hr=S_OK;
	if (m_bLocked) {
		hr=E_ACCESSDENIED;
	}else{
		string filename;
		if (szFilename==NULL) {
			filename="script/config.lua";
		}else{
			filename=szFilename;
		}
		m_bOverwrite=bOverwrite;
		filename="(gl)"+filename;
		NPL::CNPLRuntime::GetInstance()->NPL_LoadFile(NPL::CNPLRuntime::GetInstance()->GetMainRuntimeState(), filename.c_str(),true);
		m_bOverwrite=true;
	}

	return hr;
}

#ifdef _DEBUG
void CICConfigManager::test()
{
	using namespace ParaEngine;
	CICConfigManager *cm=CGlobals::GetICConfigManager();
	HRESULT hr;
	//test adding new value
	hr=cm->SetIntValue("a",1);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetIntValue("b",10000000);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetDoubleValue("c",1.2345678901234567890123456789);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetDoubleValue("d",12345678901234567890.0f);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetTextValue("e","ffff");
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetTextValue("f","");
	if (hr!=S_OK) {
		throw;
	}
	//test save to file with default file, read the output file manually
	/* The output file should be
	Config.SetIntValue("a",1);
	Config.SetIntValue("b",-1530494976);
	Config.SetFloatValue("c",1.2345678901234567);
	Config.SetFloatValue("d",12345678901234567000);
	Config.SetTextValue("e","ffff");
	Config.SetTextValue("f","");
	*/	
	hr=cm->SaveToFile();
	if (hr!=S_OK) {
		throw;
	}

	//test override old value
	hr=cm->SetIntValue("a",2);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetIntValue("b",1);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetDoubleValue("c",-12345678901234567890.0f);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetDoubleValue("d",-1.23456789012345678901234567890f);
	if (hr!=S_OK) {
		throw;
	}
	//test override with different data type
	hr=cm->SetTextValue("a","1");
	if (hr!=E_FAIL) {
		throw;
	}
	hr=cm->SetTextValue("c","1");
	if (hr!=E_FAIL) {
		throw;
	}

	hr=cm->SetIntValue("e",1);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetDoubleValue("f",1.23456789012345678901234567890);
	if (hr!=S_OK) {
		throw;
	}


	/*config1.lua should be
	Config.SetIntValue("a",2);
	Config.SetIntValue("b",1);
	Config.SetDoubleValue("c",-12345679395506094000);
	Config.SetDoubleValue("d",-1.2345678806304932);
	Config.SetTextValue("e","1");
	Config.SetTextValue("f","1.2345678901234567");*/
	//test save to file with another file, read the output file manually
	hr=cm->SaveToFile("script/config1.lua");
	if (hr!=S_OK) {
		throw;
	}

	//set double to int and set int to double
	hr=cm->SetDoubleValue("b",1.234);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->SetIntValue("d",1);
	if (hr!=S_OK) {
		throw;
	}
	//invalid argument
	hr=cm->SetTextValue(NULL,NULL);
	if (hr!=E_INVALIDARG) {
		throw;
	}
	hr=cm->SetIntValue(NULL,1);
	if (hr!=E_INVALIDARG) {
		throw;
	}
	hr=cm->SetDoubleValue(NULL,1.2);
	if (hr!=E_INVALIDARG) {
		throw;
	}


	//test save to file with an existing file and overwrite=false, should fail
	hr=cm->SaveToFile("script/config1.lua",false);
	if (hr!=E_FAIL) {
		throw;
	}

	hr=cm->LoadFromFile("script/config.lua");
	if (hr!=S_OK) {
		throw;
	}

	//this file should be the same as config.lua
	hr=cm->SaveToFile("script/config.bak.lua");
	if (hr!=S_OK) {
		throw;
	}

	//test load file overwrite 
	hr=cm->LoadFromFile("script/config1.lua");
	if (hr!=S_OK) {
		throw;
	}

	hr=cm->LoadFromFile("script/config.lua",false);
	if (hr!=S_OK) {
		throw;
	}

	//this file should be the same as config1.lua
	hr=cm->SaveToFile("script/config.bak1.lua");
	if (hr!=S_OK) {
		throw;
	}

	//test getting value
	string st;
	int it;
	double dt;

	hr=cm->GetIntValue("a",&it);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->GetTextValue("e",st);
	if (hr!=S_OK) {
		throw;
	}
	hr=cm->GetDoubleValue("c",&dt);
	if (hr!=S_OK) {
		throw;
	}

	//the log value should be a=2, c=-12345679395506094000.000000, e=1
	OUTPUT_LOG("a=%d, c=%f, e=%s\n",it,dt,st.c_str());
}
#endif