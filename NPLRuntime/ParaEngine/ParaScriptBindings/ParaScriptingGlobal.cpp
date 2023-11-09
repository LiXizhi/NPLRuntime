//-----------------------------------------------------------------------------
// Class:	For ParaGlobal namespace. (almost Cross Platform)
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AISimulator.h"
#include "FrameRateController.h"
#include "FileManager.h"
#include "ParaUtils.hpp"
#include "DynamicAttributeField.h"
#include "Framework/Common/Helper/EditorHelper.h"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#ifdef WIN32
	#include "Framework/Common/Helper/EditorHelper.h"
	#include "SceneObject.h"
	#include <commdlg.h>
#endif

#include "NPLRuntime.h"
#include "NPLHelper.h"
#include "IParaEngineApp.h"
#include "AttributesManager.h"
#undef ShellExecute
#include "ParaScriptingGlobal.h"
#include "ParaScriptingScene.h"
#include "ParaScriptingGUI.h"
#include "BootStrapper.h"
#include "util/StringHelper.h"
#include "util/ParaTime.h"

#ifdef WIN32
#define SUPPORT_SHELL_EXECUTE
#endif

#ifdef SUPPORT_SHELL_EXECUTE
#include <boost/process.hpp>
#if defined(WIN32)
#include <boost/process/windows.hpp>
#endif
#endif

#include <boost/bind.hpp>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#endif
#include "ParaScriptBindings/ParaScriptingNPL.h"

/**@def PARAENGINE_SUPPORT_WRITE_REG: to support writing to registry*/
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && defined(PARAENGINE_CLIENT)
#define PARAENGINE_SUPPORT_WRITE_REG
#endif

#if defined(WIN32) && defined(PARAENGINE_CLIENT)
#define PARAENGINE_SUPPORT_READ_REG
#endif 

#ifndef MAX_LINE
/**@def max number of characters in a single line */
#define MAX_LINE	1024
#endif




extern "C"
{
#include <lua.h>
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include "IParaEngineApp.h"

namespace ParaEngine
{
	extern int my_rand();
}

using namespace ParaEngine;
using namespace ParaScripting;
using namespace luabind;

std::map<const string, ParaObjectNode, CStringCompare> g_paraDictionary;

//--------------------------------------------------------------
// for ParaGlobal namespace table.
//--------------------------------------------------------------
ParaServiceLogger::ParaServiceLogger(ParaEngine::CServiceLogger_ptr& logger) :m_logger_ptr(logger)
{
}

void ParaServiceLogger::log(int level, const object& message)
{
	
	if (type(message) == LUA_TSTRING)
	{
		const char * str = object_cast<const char*>(message);
		if (m_logger_ptr.get() != 0)
			m_logger_ptr->log(level, str);
		else
			CLogger::GetSingleton().log(level, str);
	}
}

int ParaServiceLogger::GetLevel()
{
	if (m_logger_ptr.get() != 0)
		return m_logger_ptr->GetLevel();
	else
		return CLogger::GetSingleton().GetLevel();
}

void ParaServiceLogger::SetLevel(const int level1)
{
	if (m_logger_ptr.get() != 0)
		m_logger_ptr->SetLevel(level1);
	else
		CLogger::GetSingleton().SetLevel(level1);
}


bool ParaServiceLogger::IsEnabledFor(int level)
{
	if (m_logger_ptr.get() != 0)
		return m_logger_ptr->IsEnabledFor(level);
	else 
		return CLogger::GetSingleton().IsEnabledFor(level);
}

void ParaServiceLogger::SetForceFlush(bool bForceFlush)
{
	if (m_logger_ptr.get() != 0)
		m_logger_ptr->SetForceFlush(bForceFlush);
	else
		CLogger::GetSingleton().SetForceFlush(bForceFlush);
}

void ParaServiceLogger::SetAppendMode(bool bAppendToExistingFile)
{
	if (m_logger_ptr.get() != 0)
		m_logger_ptr->SetAppendMode(bAppendToExistingFile);
	else
		CLogger::GetSingleton().SetAppendMode(bAppendToExistingFile);
}

void ParaServiceLogger::SetLogFile(const char* sFileName)
{
	if (m_logger_ptr.get() != 0 && sFileName)
		m_logger_ptr->SetLogFile(sFileName);
	else if (sFileName)
		CLogger::GetSingleton().SetLogFile(sFileName);
}

void ParaGlobal::WriteToConsole(const char* strMessage)
{
	OUTPUT_LOG("%s", strMessage);
}
void ParaGlobal::WriteToLogFile(const char* strMessage)
{
	// the output is assumed to be utf8
	ParaEngine::CLogger::GetSingleton().AddLogStr(strMessage);
}

ParaScripting::ParaServiceLogger ParaScripting::ParaGlobal::GetLogger(const object& name)
{
	if (type(name) == LUA_TSTRING)
	{
		const char * str = object_cast<const char*>(name);
		if (str && str[0])
		{
			CServiceLogger_ptr logger = ParaEngine::CServiceLogger::GetLogger(str);
			return ParaServiceLogger(logger);
		}
	}
	return ParaServiceLogger();
}

ParaScripting::ParaServiceLogger ParaScripting::ParaGlobal::GetLogger_(const char* name)
{
	CServiceLogger_ptr logger = ParaEngine::CServiceLogger::GetLogger(name);
	return ParaServiceLogger(logger);
}


int ParaScripting::ParaGlobal::GetLogPos()
{
	return ParaEngine::CLogger::GetSingleton().GetPos();
}

const char* ParaScripting::ParaGlobal::GetLog(int fromPos, int nCount)
{
	return ParaEngine::CLogger::GetSingleton().GetLog(fromPos, nCount);
}

void ParaGlobal::ExitApp()
{
	CGlobals::GetApp()->Exit(0);
}

void ParaScripting::ParaGlobal::Exit(int nReturnCode)
{
	CGlobals::GetApp()->Exit(nReturnCode);
}

// current selected object
ParaAttributeObject g_selected_attr_obj;

void ParaScripting::ParaGlobal::SelectAttributeObject(const ParaAttributeObject& obj)
{
	g_selected_attr_obj = obj;
}

bool ParaGlobal::WriteToFile(const char* filename, const char* strMessage)
{
	FILE *file;
	std::string fname = filename;
	CParaFile::MakeDirectoryFromFilePath(fname.c_str());
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
	file = ::_wfopen(filename16, L"w+");
#else
	file = ::fopen(filename, "w+");
#endif
	if (file == NULL) {
		return false;
	}
	size_t length = ::strlen(strMessage);
	if (::fwrite(strMessage, 1, length, file) != length){
		::fclose(file);
		return false;
	}
	::fclose(file);
	return true;
}

void ParaGlobal::SetGameStatus(const char* strState)
{
#ifdef PARAENGINE_CLIENT
	if (strcmp(strState, "disable") == 0)
	{
		CGlobals::GetScene()->DisableGame();
	}
	else if (strcmp(strState, "enable") == 0)
	{
		CGlobals::GetScene()->EnableScene(true);
	}
	else if (strcmp(strState, "pause") == 0)
	{
		CGlobals::GetScene()->PauseGame();
	}
	else if (strcmp(strState, "resume") == 0)
	{
		CGlobals::GetScene()->PauseScene(false);
	}
#endif
}
double ParaGlobal::GetGameTime()
{
	return (CGlobals::GetGameTime() * 1000);
}

void ParaGlobal::SaveObject(const char* strObjectName, const object& objObject)
{
	int nType = type(objObject);
	if (nType == LUA_TNUMBER)
	{
		double value = object_cast<double>(objObject);
		pair< map<const string, ParaObjectNode, CStringCompare>::iterator, bool > pr;
		pr = g_paraDictionary.insert(ParaObject_Pair(strObjectName, ParaObjectNode(value)));
		if (pr.second == false)
		{
			/// already exists, then override old value
			(*pr.first).second.oType = ParaObjectNode::number;
			(*pr.first).second.dNumber = value;
		}
	}
	else if (nType == LUA_TSTRING)
	{
		const char * str = object_cast<const char*>(objObject);
		pair< map<const string, ParaObjectNode, CStringCompare>::iterator, bool > pr;
		pr = g_paraDictionary.insert(ParaObject_Pair(strObjectName, ParaObjectNode(str)));
		if (pr.second == false)
		{
			/// already exists, then override old value
			(*pr.first).second.oType = ParaObjectNode::string;
			(*pr.first).second.str = str;
		}
	}
	else if (nType == LUA_TNIL)
	{
		/// already exists, delete it.
		g_paraDictionary.erase(strObjectName);
	}
}

object ParaGlobal::LoadObject(const object& strObjectName)
{
	if (type(strObjectName) == LUA_TSTRING)
	{
		/// get object from dictionary table
		std::map<const string, ParaObjectNode, CStringCompare>::iterator iter = g_paraDictionary.find(object_cast<const char*>(strObjectName));
		if (iter != g_paraDictionary.end())
		{
			if ((*iter).second.oType == ParaObjectNode::number)
				return object(strObjectName.interpreter(), ((*iter).second.dNumber));
			else
				return object(strObjectName.interpreter(), ((*iter).second.str));
		}
		else
		{
			// TODO: push nil
			//objObject = 0; // maybe you should push a nil object beforehand to the lua state
		}
	}

	return object();
}
void ParaGlobal::SetGameLoop(const char* scriptName)
{
	CGlobals::GetAISim()->SetGameLoop(scriptName);
}

void ParaGlobal::SetGameLoopInterval(float fInterval)
{
	CGlobals::GetAISim()->SetGameLoopInterval(fInterval);
}

bool ParaGlobal::CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
{
	return CEditorHelper::CreateProcess(lpApplicationName, lpCommandLine, bWaitOnReturn);
}


bool ParaGlobal::IsPortAvailable(const std::string& ip, const int port, lua_State* L)
{
#ifndef EMSCRIPTEN_SINGLE_THREAD
	using namespace boost::asio;
	using ip::tcp;

	io_service svc;
	tcp::acceptor a(svc);
	boost::system::error_code ec;

	a.open(tcp::v4());

	socket_base::reuse_address option(false);
	a.set_option(option);

	a.bind(tcp::endpoint(ip::address::from_string(ip), port), ec);
	a.close();

	return !(ec == boost::asio::error::address_in_use);
#else
	return false;
#endif
}

void ParaGlobal::Execute(const std::string& exe, const luabind::object& param, lua_State* L)
{
#ifdef SUPPORT_SHELL_EXECUTE
	namespace bp = boost::process;

	std::vector<std::string> args;

	if (type(param) == LUA_TTABLE)
	{
		for (luabind::iterator itCur(param), itEnd; itCur != itEnd; ++itCur)
		{
			const std::string value = object_cast<std::string>(*itCur);
			args.push_back(value);
		}
	}
	else if (type(param) == LUA_TSTRING)
	{
		args.push_back(object_cast<std::string>(param));
	}

#if defined(WIN32)
	bp::environment env = boost::this_process::environment();
#ifdef DEFAULT_FILE_ENCODING
	std::wstring exe_path_unicode = StringHelper::MultiByteToWideChar(exe.c_str(), DEFAULT_FILE_ENCODING);
	bp::child c(exe_path_unicode, bp::args(args), env, bp::windows::hide);
#else
	bp::child c(exe, bp::args(args), env, bp::windows::hide);
#endif

#else
	bp::child c(exe, bp::args(args));
#endif

	c.detach();
#endif
}

bool ParaGlobal::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
{
#ifdef EMSCRIPTEN
	if (lpOperation && strcmp(lpOperation, "open") == 0)
	{
		EM_ASM({
			const url =  UTF8ToString($0);
			console.log('open ' + url);
			window.open(url);
		}, lpFile);
		return true;
	}
	return false;
#else
	return CEditorHelper::ShellExecute(lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// ParaAttributeObject
//
//////////////////////////////////////////////////////////////////////////
ParaAttributeObject::ParaAttributeObject(IAttributeFields * pAttribute)
	:m_pAttribute(pAttribute)
{
	if (pAttribute != 0)
	{
		m_pAttClass = pAttribute->GetAttributeClass();
	}
}
ParaAttributeObject::ParaAttributeObject(IAttributeFields * pAttribute, CAttributeClass* pAttClass)
	: m_pAttribute(pAttribute), m_pAttClass(pAttClass)
{
	if (pAttribute && pAttClass == 0)
	{
		m_pAttClass = pAttribute->GetAttributeClass();
	}
};

bool ParaAttributeObject::IsValid() const
{
	return (m_pAttribute.get()!=0);
}

int  ParaAttributeObject::GetClassID() const
{
	return IsValid() ? m_pAttClass->GetClassID() : -1;
}

const char* ParaAttributeObject::GetClassName1() const
{
	return GetClassName();
}
const char* ParaAttributeObject::GetClassName() const
{
	return IsValid() ? m_pAttClass->GetClassName() : "";
}
const char* ParaAttributeObject::GetClassDescription() const
{
	return IsValid() ? m_pAttClass->GetClassDescription() : "";
}

void ParaAttributeObject::SetOrder(int order)
{
	if (IsValid())
		m_pAttClass->SetOrder((CAttributeClass::Field_Order)order);
}

int ParaAttributeObject::GetOrder()
{
	return IsValid() ? m_pAttClass->GetOrder() : 0;
}

int ParaAttributeObject::GetFieldNum()
{
	return IsValid() ? m_pAttClass->GetFieldNum() : 0;
}

const char* ParaAttributeObject::GetFieldName(int nIndex)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			return pField->GetFieldname().c_str();
		}
	}
	return CGlobals::GetString(G_STR_EMPTY).c_str();
}

int ParaAttributeObject::GetFieldIndex(const char*  sFieldname)
{
	if (IsValid())
		return m_pAttClass->GetFieldIndex(sFieldname);
	return -1;
}
bool ParaAttributeObject::IsFieldReadOnly(int nIndex)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			return (pField->m_offsetSetFunc.ptr_fun == 0);
		}
	}
	return true;
}
const char* ParaAttributeObject::GetFieldType(int nIndex)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			return pField->GetTypeAsString();
		}
	}
	return CGlobals::GetString(G_STR_EMPTY).c_str();
}

const char* ParaAttributeObject::GetFieldSchematics(int nIndex)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			return pField->m_sSchematics.c_str();
		}
	}
	return CGlobals::GetString(G_STR_EMPTY).c_str();
}

void ParaAttributeObject::GetSchematicsMinMax(int nIndex, float fMinIn, float fMaxIn, float& fMin, float& fMax)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			float min, max;
			if (pField->GetSchematicsMinMax(min, max))
			{
				fMinIn = min;
				fMaxIn = max;
			}
		}
	}
	fMin = fMinIn;
	fMax = fMaxIn;
}

const char* ParaAttributeObject::GetSchematicsType(int nIndex)
{
	if (IsValid())
	{
		CAttributeField* pField = m_pAttClass->GetField(nIndex);
		if (pField != 0)
		{
			return pField->GetSchematicsType();
		}
	}
	return CGlobals::GetString(G_STR_EMPTY).c_str();
}

void ParaAttributeObject::CallField(const char*  sFieldname)
{
	if (!IsValid())
		return;
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField != 0)
	{
		if (pField->m_type == FieldType_void)
		{
			pField->Call(m_pAttribute.get());
		}
	}
}

object ParaScripting::ParaAttributeObject::GetDynamicField(const char* sFieldname, const object& output)
{
	if (!IsValid()){
		return object(output);
	}
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	return GetDynamicFieldImp(pField, output);
}

const char* ParaScripting::ParaAttributeObject::GetDynamicFieldNameByIndex(int nIndex)
{
	if (!IsValid()){
		return NULL;
	}
	return m_pAttribute->GetDynamicFieldNameByIndex(nIndex);
}

object ParaScripting::ParaAttributeObject::GetDynamicField_(int nIndex, const object& output)
{
	if (!IsValid()){
		return object(output);
	}
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(nIndex);
	return GetDynamicFieldImp(pField, output);
}

luabind::object ParaScripting::ParaAttributeObject::GetDynamicFieldImp(CDynamicAttributeField* pField, const object &output)
{
	if (pField == 0){
		return object(output);
	}
	if (pField->IsAnimated())
	{
		switch (pField->GetType())
		{
		case FieldType_AnimatedQuaternion:
		{
			Quaternion value;
			if (pField->GetValueByTime(m_pAttribute->GetTime(), value))
			{
				object out(output);
				out[1] = value.x;
				out[2] = value.y;
				out[3] = value.z;
				out[4] = value.w;
				return out;
			}
			break;
		}
		case FieldType_AnimatedVector3:
		{
			Vector3 value;
			if (pField->GetValueByTime(m_pAttribute->GetTime(), value))
			{
				object out(output);
				out[1] = value.x;
				out[2] = value.y;
				out[3] = value.z;
				return out;
			}
			break;
		}
		case FieldType_AnimatedFloat:
		{
			float value;
			if (pField->GetValueByTime(m_pAttribute->GetTime(), value))
			{
				return object(output.interpreter(), value);
			}
			break;
		}
		default:
			break;
		}
	}
	else
	{
		switch (pField->GetType())
		{
		case FieldType_Bool:
		{
			return object(output.interpreter(), (bool)(*pField));
			break;
		}
		case FieldType_Int:
		{
			return object(output.interpreter(), (int)(*pField));
			break;
		}
		case FieldType_DWORD:
		{
			return object(output.interpreter(), (DWORD)(*pField));
			break;
		}
		case FieldType_Float:
		{
			return object(output.interpreter(), (float)(*pField));
			break;
		}
		case FieldType_Double:
		{
			return object(output.interpreter(), (double)(*pField));
			break;
		}
		case FieldType_String:
		{
			return object(output.interpreter(), (const char*)(*pField));
			break;
		}
		case FieldType_void:
		{
			break;
		}
		default:
			break;
		}
	}
	return object(output);
}

luabind::object ParaScripting::ParaAttributeObject::GetFieldKeyValue(const char* sFieldname, int nKeyIndex, const object& output)
{
	if (!IsValid()){
		return object(output);
	}
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		switch (pField->GetType())
		{
		case FieldType_AnimatedQuaternion:
		{
			Quaternion value;
			if (pField->GetValue(nKeyIndex, value))
			{
				object out(output);
				out[1] = value.x;
				out[2] = value.y;
				out[3] = value.z;
				out[4] = value.w;
				return out;
			}
			break;
		}
		case FieldType_AnimatedVector3:
		{
			Vector3 value;
			if (pField->GetValue(nKeyIndex, value))
			{
				object out(output);
				out[1] = value.x;
				out[2] = value.y;
				out[3] = value.z;
				return out;
			}
			break;
		}
		case FieldType_AnimatedFloat:
		{
			float value;
			if (pField->GetValue(nKeyIndex, value))
			{
				return object(output.interpreter(), value);
			}
			break;
		}
		default:
			break;
		}
	}
	return object(output);
}

int ParaScripting::ParaAttributeObject::GetDynamicFieldCount()
{
	if (!IsValid()){
		return 0;
	}
	return m_pAttribute->GetDynamicFieldCount();
}


void ParaScripting::ParaAttributeObject::SetFieldKeyValue(const char* sFieldname, int nKeyIndex, const object& input)
{
	if (!IsValid())
		return;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		int nType = type(input);
		if (nType == LUA_TNUMBER)
		{
			double value;
			value = object_cast<double>(input);
			pField->SetValue(nKeyIndex, value);
		}
		else if (nType == LUA_TTABLE)
		{
			double values[16];
			int nNum = 0;
			for (luabind::iterator itCur(input), itEnd; itCur != itEnd && nNum < 16; ++itCur)
			{
				const object& item = *itCur;
				if (type(item) == LUA_TNUMBER)
				{
					values[nNum] = object_cast<double>(item);
					++nNum;
				}
			}
			if (pField)
			{
				switch (pField->GetType())
				{
				case FieldType_AnimatedQuaternion:
				{
					if (nNum == 4)
					{
						Quaternion value((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
						pField->SetValue(nKeyIndex, value);
					}
					break;
				}
				case FieldType_AnimatedVector3:
				{
					if (nNum == 3)
					{
						Vector3 value((float)values[0], (float)values[1], (float)values[2]);
						pField->SetValue(nKeyIndex, value);
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}
	
}

int ParaScripting::ParaAttributeObject::SetDynamicField(const char* sFieldname, const object& input)
{
	if (!IsValid())
		return -1;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	int nResult = 0;
	int nType = type(input);
	if (nType == LUA_TNUMBER)
	{
		CVariable value;
		value = object_cast<double>(input);
		nResult = m_pAttribute->SetDynamicField(sFieldname, value);
	}
	else if (nType == LUA_TBOOLEAN)
	{
		CVariable value;
		value = object_cast<bool>(input);
		nResult = m_pAttribute->SetDynamicField(sFieldname, value);
	}
	else if (nType == LUA_TSTRING)
	{
		CVariable value;
		value = object_cast<const char*>(input);
		nResult = m_pAttribute->SetDynamicField(sFieldname, value);
	}
	else if (nType == LUA_TNIL)
	{
		CVariable value;
		nResult = m_pAttribute->SetDynamicField(sFieldname, value);
	}
	else if (nType == LUA_TTABLE)
	{
		double values[16];
		int nNum = 0;
		for (luabind::iterator itCur(input), itEnd; itCur != itEnd && nNum < 16; ++itCur)
		{
			const object& item = *itCur;
			if (type(item) == LUA_TNUMBER)
			{
				values[nNum] = object_cast<double>(item);
				++nNum;
			}
		}
		if (pField)
		{
			bool isKeyExist = false;
			switch (pField->GetType())
			{
			case FieldType_AnimatedQuaternion:
			{
				if (nNum == 4)
				{
					Quaternion value((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
					pField->SetValue(pField->AddKey(m_pAttribute->GetTime(), &isKeyExist), value);
				}
				break;
			}
			case FieldType_AnimatedVector3:
			{
				if (nNum == 3)
				{
					Vector3 value((float)values[0], (float)values[1], (float)values[2]);
					pField->SetValue(pField->AddKey(m_pAttribute->GetTime(), &isKeyExist), value);
				}
				break;
			}
			default:
				break;
			}
			if (!isKeyExist)
				nResult = 1;
		}
	}
	return nResult;
}

int ParaScripting::ParaAttributeObject::SetDynamicField_(int nFieldIndex, const object& input)
{
	if (!IsValid())
		return -1;
	int nResult = 0;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(nFieldIndex);
	if (pField)
	{
		int nType = type(input);
		if (nType == LUA_TNUMBER)
		{
			CVariable value;
			value = object_cast<double>(input);
			*pField = value;
		}
		else if (nType == LUA_TBOOLEAN)
		{
			CVariable value;
			value = object_cast<bool>(input);
			*pField = value;
		}
		else if (nType == LUA_TSTRING)
		{
			CVariable value;
			value = object_cast<const char*>(input);
			// *pField = value;
		}
		else if (nType == LUA_TNIL)
		{
			CVariable value;
			*pField = value;
		}
		else if (nType == LUA_TTABLE)
		{
			double values[16];
			int nNum = 0;
			bool isKeyExist = false;
			for (luabind::iterator itCur(input), itEnd; itCur != itEnd && nNum < 16; ++itCur)
			{
				const object& item = *itCur;
				if (type(item) == LUA_TNUMBER)
				{
					values[nNum] = object_cast<double>(item);
					++nNum;
				}
			}
			switch (pField->GetType())
			{
			case FieldType_AnimatedQuaternion:
			{
				if (nNum == 4)
				{
					Quaternion value((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
					pField->SetValue(pField->AddKey(m_pAttribute->GetTime(), &isKeyExist), value);
				}
				break;
			}
			case FieldType_AnimatedVector3:
			{
				if (nNum == 3)
				{
					Vector3 value((float)values[0], (float)values[1], (float)values[2]);
					pField->SetValue(pField->AddKey(m_pAttribute->GetTime(), &isKeyExist), value);
				}
				break;
			}
			default:
				break;
			}
			if (!isKeyExist)
				nResult = 1;
		}
	}
	return nResult;
}

int ParaScripting::ParaAttributeObject::GetFieldKeyNums(const char* sFieldname)
{
	if (!IsValid())
		return 0;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		return pField->GetNumKeys();
	}
	return 0;
}

void ParaScripting::ParaAttributeObject::SetFieldKeyNums(const char* sFieldname, int nKeyNum)
{
	if (!IsValid())
		return;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		return pField->SetNumKeys(nKeyNum);
	}
}

void ParaScripting::ParaAttributeObject::SetFieldKeyTime(const char* sFieldname, int nKeyIndex, int nTime)
{
	if (!IsValid())
		return;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		return pField->SetTime(nKeyIndex, nTime);
	}
}

int ParaScripting::ParaAttributeObject::GetFieldKeyTime(const char* sFieldname, int nKeyIndex)
{
	if (!IsValid())
		return 0;
	CDynamicAttributeField* pField = m_pAttribute->GetDynamicField(sFieldname);
	if (pField)
	{
		return pField->GetTime(nKeyIndex);
	}
	return 0;
}


void ParaScripting::ParaAttributeObject::RemoveAllDynamicFields()
{
	if (!IsValid()){
		return;
	}
	m_pAttribute->RemoveAllDynamicFields();
}

int ParaScripting::ParaAttributeObject::AddDynamicField(const std::string& sName, int dwType)
{
	return IsValid() ? m_pAttribute->AddDynamicField(sName, (ATTRIBUTE_FIELDTYPE)dwType) : -1;
}

ParaScripting::ParaAttributeObject ParaScripting::ParaAttributeObject::GetChild(const std::string& sName)
{
	return ParaAttributeObject(IsValid() ? m_pAttribute->GetChildAttributeObject(sName.c_str()) : NULL);
}

int ParaScripting::ParaAttributeObject::GetChildCount()
{
	return IsValid() ? m_pAttribute->GetChildAttributeObjectCount() : 0;
}

int ParaScripting::ParaAttributeObject::GetChildCount2(int nColumnIndex)
{
	return IsValid() ? m_pAttribute->GetChildAttributeObjectCount(nColumnIndex) : 0;
}

ParaScripting::ParaAttributeObject ParaScripting::ParaAttributeObject::GetChildAt(int nIndex)
{
	return ParaAttributeObject(IsValid() ? m_pAttribute->GetChildAttributeObject(nIndex) : NULL);
}

ParaScripting::ParaAttributeObject ParaScripting::ParaAttributeObject::GetChildAt2(int nRowIndex, int nColumnIndex)
{
	return ParaAttributeObject(IsValid() ? m_pAttribute->GetChildAttributeObject(nRowIndex, nColumnIndex) : NULL);
}

int ParaScripting::ParaAttributeObject::GetColumnCount()
{
	return IsValid() ? m_pAttribute->GetChildAttributeColumnCount() : 0;
}


bool ParaScripting::ParaAttributeObject::AddChild(ParaAttributeObject& obj)
{
	return ((IsValid() && obj.IsValid()) ? m_pAttribute->AddChildAttributeObject(obj.m_pAttribute) : false);
}

const ParaObject& ParaScripting::ParaAttributeObject::QueryObject()
{
	static ParaObject s_obj;
	s_obj.m_pObj = IsValid() ? ((CBaseObject*)(m_pAttribute->QueryObject(ATTRIBUTE_CLASSID_CBaseObject))) : NULL;
	return s_obj;
}

const ParaUIObject& ParaScripting::ParaAttributeObject::QueryUIObject()
{
	static ParaUIObject s_obj;
	s_obj.m_pObj = IsValid() ? ((CGUIBase*)(m_pAttribute->QueryObject(ATTRIBUTE_CLASSID_CGUIBase))) : NULL;
	return s_obj;
}

object ParaAttributeObject::GetField2(const char* sFieldname, lua_State* L)
{
	if (!IsValid())
	{
		// return nil
		return object();
	}

	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasGetFunction())
		// return nil
		return object();

	switch (pField->m_type)
	{
	case FieldType_Bool:
	{
		bool value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_Int:
	case FieldType_Enum:
	{
		int value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_DWORD:
	{
		DWORD value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_Float:
	{
		float value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_Double:
	{
		double value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_String:
	{
		const char* value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
			return object(L, value);
		else
			// return nil;
			return object();
		break;
	}
	case FieldType_Vector2:
	{
		Vector2 value;

		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			out[1] = value.x;
			out[2] = value.y;
			return out;
		}
		else
			// return nil;
			return object();
		
		break;
	}
	case FieldType_Vector3:
	{
		Vector3 value;

		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			out[1] = value.x;
			out[2] = value.y;
			out[3] = value.z;
			return out;
		}
		else
			// return nil;
			return object();
		
		break;
	}
	case FieldType_DVector3:
	{
		DVector3 value;

		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			out[1] = value.x;
			out[2] = value.y;
			out[3] = value.z;
			return out;
		}
		else
			// return nil;
			return object();
		
		break;
	}
	case FieldType_Vector4:
	{
		Vector4 value;

		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			out[1] = value.x;
			out[2] = value.y;
			out[3] = value.z;
			out[4] = value.w;
			return out;
		}
		else
			// return nil;
			return object();
	
		break;
	}
	case FieldType_Quaternion:
	{
		Quaternion value;

		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			out[1] = value.x;
			out[2] = value.y;
			out[3] = value.z;
			out[4] = value.w;
			return out;
		}
		else
			// return nil;
			return object();
		
		break;
	}
	case FieldType_Matrix4:
	{
		Matrix4 value;
		if (SUCCEEDED(pField->Get(m_pAttribute.get(), &value)))
		{
			object out = newtable(L);
			for (int i = 0; i < 16; i++) {
				out[i + 1] = value._m[i];
			}
			return out;
		}
		else
			// return nil;
			return object();

		break;
	}
	default:
		break;
	}


	// return nil
	return object();
}

object ParaAttributeObject::GetField(const char *sFieldname, const object &output)
{
	if (!IsValid()){
		return object(output);
	}

	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasGetFunction())
		return object(output);

	switch (pField->m_type)
	{
	case FieldType_Bool:
	{
		bool value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_Int:
	case FieldType_Enum:
	{
		int value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_DWORD:
	{
		DWORD value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_Float:
	{
		float value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_Double:
	{
		double value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_String:
	{
		const char* value;
		pField->Get(m_pAttribute.get(), &value);
		return object(output.interpreter(), value);
		break;
	}
	case FieldType_Vector2:
	{
		Vector2 value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		out[1] = value.x;
		out[2] = value.y;
		return out;
		break;
	}
	case FieldType_Vector3:
	{
		Vector3 value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		out[1] = value.x;
		out[2] = value.y;
		out[3] = value.z;
		return out;
		break;
	}
	case FieldType_DVector3:
	{
		DVector3 value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		out[1] = value.x;
		out[2] = value.y;
		out[3] = value.z;
		return out;
		break;
	}
	case FieldType_Vector4:
	{
		Vector4 value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		out[1] = value.x;
		out[2] = value.y;
		out[3] = value.z;
		out[4] = value.w;
		return out;
		break;
	}
	case FieldType_Quaternion:
	{
		Quaternion value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		out[1] = value.x;
		out[2] = value.y;
		out[3] = value.z;
		out[4] = value.w;
		return out;
		break;
	}
	case FieldType_Matrix4:
	{
		Matrix4 value;
		pField->Get(m_pAttribute.get(), &value);
		object out(output);
		for (int i = 0; i < 16; i++){
			out[i + 1] = value._m[i];
		}
		return out;
		break;
	}
	default:
		break;
	}

	return object(output);
}

void ParaAttributeObject::SetField(const char*  sFieldname, const object& input)
{
	if (!IsValid())
		return;
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasSetFunction())
		return;
	int nType = type(input);
	if (nType == LUA_TNUMBER || nType == LUA_TBOOLEAN)
	{
		switch (pField->m_type)
		{
		case FieldType_Bool:
		{
			bool value = object_cast<bool>(input);
			pField->Set(m_pAttribute.get(), value);
			break;
		}
		case FieldType_Int:
		case FieldType_Enum:
		{
			int value = object_cast<int>(input);
			pField->Set(m_pAttribute.get(), value);
			break;
		}
		case FieldType_DWORD:
		{
			DWORD value = object_cast<unsigned int>(input);
			pField->Set(m_pAttribute.get(), value);
			break;
		}
		case FieldType_Float:
		{
			float value = object_cast<float>(input);
			pField->Set(m_pAttribute.get(), value);
			break;
		}
		case FieldType_Double:
		{
			double value = object_cast<double>(input);
			pField->Set(m_pAttribute.get(), value);
			break;
		}
		default:
			break;
		}
	}
	else if (nType == LUA_TSTRING)
	{
		if (pField->m_type == FieldType_String)
		{
			pField->Set(m_pAttribute.get(), object_cast<const char*>(input));
		}
	}
	else if (nType == LUA_TTABLE)
	{
		double values[16];
		int nNum = 0;
		for (luabind::iterator itCur(input), itEnd; itCur != itEnd && nNum < 16; ++itCur)
		{
			const object& item = *itCur;
			if (type(item) == LUA_TNUMBER)
			{
				values[nNum] = object_cast<double>(item);
				++nNum;
			}
		}
		switch (pField->m_type)
		{
		case FieldType_Vector2:
		{
			if (nNum == 2)
			{
				Vector2 value((float)values[0], (float)values[1]);
				pField->Set(m_pAttribute.get(), value);
				break;
			}
		}
		case FieldType_Vector3:
		{
			if (nNum == 3)
			{
				Vector3 value((float)values[0], (float)values[1], (float)values[2]);
				pField->Set(m_pAttribute.get(), value);
				break;
			}
		}
		case FieldType_DVector3:
		{
			if (nNum == 3)
			{
				DVector3 value((double)values[0], (double)values[1], (double)values[2]);
				pField->Set(m_pAttribute.get(), value);
				break;
			}
		}
		case FieldType_Vector4:
		{
			if (nNum == 4)
			{
				Vector4 value((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
				pField->Set(m_pAttribute.get(), value);
				break;
			}
		}
		case FieldType_Quaternion:
		{
			if (nNum == 4)
			{
				Quaternion value((float)values[0], (float)values[1], (float)values[2], (float)values[3]);
				pField->Set(m_pAttribute.get(), value);
				break;
			}
		}
		case FieldType_Float_Float:
		{
			if (nNum == 2)
			{
				pField->Set(m_pAttribute.get(), (float)values[0], (float)values[1]);
				break;
			}
		}
		case FieldType_Float_Float_Float:
		{
			if (nNum == 3)
			{
				pField->Set(m_pAttribute.get(), (float)values[0], (float)values[1], (float)values[2]);
				break;
			}
		}
		case FieldType_Float_Float_Float_Float:
		{
			if (nNum == 4)
			{
				pField->Set(m_pAttribute.get(), (float)values[0], (float)values[1], (float)values[2], (float)values[3]);
				break;
			}
		}
		case FieldType_Matrix4:
		{
			if (nNum == 16)
			{
				Matrix4 mat(
					(float)values[0], (float)values[1], (float)values[2], (float)values[3],
					(float)values[4], (float)values[5], (float)values[6], (float)values[7],
					(float)values[8], (float)values[9], (float)values[10], (float)values[11],
					(float)values[12], (float)values[13], (float)values[14], (float)values[15]);
				pField->Set(m_pAttribute.get(), mat);
				break;
			}
			else if (nNum == 12)
			{
				Matrix4 mat(
					(float)values[0], (float)values[1], (float)values[2], 0,
					(float)values[3], (float)values[4], (float)values[5], 0,
					(float)values[6], (float)values[7], (float)values[8], 0,
					(float)values[9], (float)values[10], (float)values[11], 1.f);
				pField->Set(m_pAttribute.get(), mat);
				break;
			}
			else
			{
				pField->Set(m_pAttribute.get(), Matrix4::IDENTITY);
			}
		}
		default:
			break;
		}
	}
}

const char* ParaAttributeObject::GetStringField(const char*  sFieldname)
{
	if (!IsValid()){
		return NULL;
	}
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasGetFunction())
		return NULL;

	if (pField->m_type == FieldType_String)
	{
		const char* value = NULL;
		pField->Get(m_pAttribute.get(), &value);
		return value;
	}
	return NULL;
}

double ParaAttributeObject::GetValueField(const char*  sFieldname, int nIndex)
{
	double output = 0;
	if (!IsValid()){
		return 0;
	}
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasGetFunction())
		return 0;

	switch (pField->m_type)
	{
	case FieldType_Bool:
	{
		bool value;
		pField->Get(m_pAttribute.get(), &value);
		output = value ? 1 : 0;
		break;
	}
	case FieldType_Int:
	case FieldType_Enum:
	{
		int value;
		pField->Get(m_pAttribute.get(), &value);
		output = value;
		break;
	}
	case FieldType_DWORD:
	{
		DWORD value;
		pField->Get(m_pAttribute.get(), &value);
		output = value;
		break;
	}
	case FieldType_Float:
	{
		float value;
		pField->Get(m_pAttribute.get(), &value);
		output = value;
		break;
	}
	case FieldType_Double:
	{
		double value;
		pField->Get(m_pAttribute.get(), &value);
		output = value;
		break;
	}
	case FieldType_String:
	{
		break;
	}
	case FieldType_Vector2:
	{
		Vector2 value;
		pField->Get(m_pAttribute.get(), &value);
		output = value[nIndex % 2];
		break;
	}
	case FieldType_Vector3:
	{
		Vector3 value;
		pField->Get(m_pAttribute.get(), &value);
		output = value[nIndex % 3];
		break;
	}
	case FieldType_DVector3:
	{
		DVector3 value;
		pField->Get(m_pAttribute.get(), &value);
		output = value[nIndex % 3];
		break;
	}
	case FieldType_Vector4:
	{
		Vector4 value;
		pField->Get(m_pAttribute.get(), &value);
		output = value[nIndex % 4];
		break;
	}
	case FieldType_Quaternion:
	{
		Quaternion value;
		pField->Get(m_pAttribute.get(), &value);
		output = value[nIndex % 4];
		break;
	}
	default:
		break;
	}

	return output;
}

void ParaAttributeObject::SetStringField(const char*  sFieldname, const char* input)
{
	if (!IsValid() && input != 0)
		return;
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasSetFunction())
		return;
	if (pField->m_type == FieldType_String)
	{
		pField->Set(m_pAttribute.get(), input);
	}
}

void ParaAttributeObject::SetValueField(const char*  sFieldname, int nIndex, double input)
{
	if (!IsValid())
		return;
	CAttributeField* pField = m_pAttClass->GetField(sFieldname);
	if (pField == 0 || !pField->HasSetFunction())
		return;
	switch (pField->m_type)
	{
	case FieldType_Bool:
	{
		bool value = input > 0 ? true : false;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_Int:
	case FieldType_Enum:
	{
		int value = (int)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_DWORD:
	{
		DWORD value = (DWORD)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}

	case FieldType_Float:
	{
		float value = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_Double:
	{
		double value = input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_String:
	{
		break;
	}
	case FieldType_Vector2:
	{
		Vector2 value;
		pField->Get(m_pAttribute.get(), &value);
		value[nIndex % 2] = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_Vector3:
	{
		Vector3 value;
		pField->Get(m_pAttribute.get(), &value);
		value[nIndex % 3] = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_DVector3:
	{
		DVector3 value;
		pField->Get(m_pAttribute.get(), &value);
		value[nIndex % 3] = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_Vector4:
	{
		Vector4 value;
		pField->Get(m_pAttribute.get(), &value);
		value[nIndex % 4] = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	case FieldType_Quaternion:
	{
		Quaternion value;
		pField->Get(m_pAttribute.get(), &value);
		value[nIndex % 4] = (float)input;
		pField->Set(m_pAttribute.get(), value);
		break;
	}
	default:
		break;
	}
}

void ParaAttributeObject::PrintObject(const char* file)
{
#ifdef PARAENGINE_CLIENT
	if (IsValid())
		CGlobals::GetAttributesManager()->PrintObject(file, m_pAttribute);
#endif
}
bool ParaAttributeObject::ResetField(int nFieldID)
{
	if (IsValid())
		return m_pAttribute->ResetField(nFieldID);
	return false;
}

bool ParaAttributeObject::InvokeEditor(int nFieldID, const char* sParameters)
{
	if (IsValid())
		return m_pAttribute->InvokeEditor(nFieldID, sParameters);
	return false;
}

ParaAttributeObject ParaAttributeObject::GetAttributeObject()
{
	return ParaAttributeObject(m_pAttribute);
}
bool ParaAttributeObject::equals(const ParaAttributeObject& obj) const
{
	return m_pAttribute.get() == obj.m_pAttribute.get();
}

double ParaScripting::ParaGlobal::random()
{
	// random between [0,1]
	return ((double)(ParaEngine::my_rand() % 1000)) / (1000 - 1);
}

std::string ParaScripting::ParaGlobal::GenerateUniqueID()
{
	return ParaEngine::GenerateUniqueIDByTime();
}

double ParaScripting::ParaGlobal::GetSysDateTime()
{
	return ParaEngine::GetSysDateTime();
}

std::string ParaScripting::ParaGlobal::GetDateFormat(const object&  sFormat)
{
	return ParaEngine::GetDateFormat(NPL::NPLHelper::LuaObjectToString(sFormat));
}

std::string ParaScripting::ParaGlobal::GetTimeFormat(const object&  sFormat)
{
	return ParaEngine::GetTimeFormat(NPL::NPLHelper::LuaObjectToString(sFormat));
}

DWORD ParaScripting::ParaGlobal::timeGetTime()
{
	return GetTickCount();
}

bool ParaScripting::ParaGlobal::WriteRegStr(const string& root_key, const string& sSubKey, const string& name, const string& value)
{
#ifdef PARAENGINE_SUPPORT_WRITE_REG
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->WriteRegStr(root_key, sSubKey, name, value);
	else
		return false;
#else
	OUTPUT_LOG("WriteRegStr not supported for security reasons. \n");
	return false;
#endif
}

const char* ParaScripting::ParaGlobal::ReadRegStr(const string& root_key, const string& sSubKey, const string& name)
{
#ifdef PARAENGINE_SUPPORT_READ_REG
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->ReadRegStr(root_key, sSubKey, name);
	else
		return NULL;
#else
	return NULL;
#endif
}

bool ParaScripting::ParaGlobal::WriteRegDWORD(const string& root_key, const string& sSubKey, const string& name, DWORD value)
{
#ifdef PARAENGINE_SUPPORT_WRITE_REG
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->WriteRegDWORD(root_key, sSubKey, name, value);
	else
		return false;
#else
	OUTPUT_LOG("WriteRegDWORD not supported for security reasons. \n");
	return false;
#endif
}

DWORD ParaScripting::ParaGlobal::ReadRegDWORD(const string& root_key, const string& sSubKey, const string& name)
{
#ifdef PARAENGINE_SUPPORT_READ_REG
	if (CGlobals::GetApp())
		return CGlobals::GetApp()->ReadRegDWORD(root_key, sSubKey, name);
	else
		return 0;
#else
	return 0;
#endif
}

/*
bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
#ifdef WIN32
	if (type(inout) != LUA_TTABLE)
	{
		return false;
	}
	// OpenFileDialog
	OPENFILENAMEA ofn = { 0 };
	memset(&ofn, 0, sizeof(ofn));
	char szFileName[MAX_LINE] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.lpstrFile = szFileName;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.nMaxFile = MAX_LINE;
	bool bIsSavingFile = false;
	for (luabind::iterator itCur(inout), itEnd; itCur != itEnd; ++itCur)
	{
		// we only serialize item with a string key
		const object& key = itCur.key();
		if (type(key) == LUA_TSTRING)
		{
			string sKey = object_cast<const char*>(key);
			const object& Value = *itCur;
			if (sKey == "initialdir")
			{
				ofn.lpstrInitialDir = object_cast<const char*>(Value);
			}
			else if (sKey == "filter")
			{
				ofn.lpstrFilter = object_cast<const char*>(Value);
			}
			else if (sKey == "title")
			{
				ofn.lpstrTitle = object_cast<const char*>(Value);
			}
			else if (sKey == "save")
			{
				if (object_cast<bool>(Value))
				{
					bIsSavingFile = true;
					ofn.Flags &= ~OFN_FILEMUSTEXIST;
				}
			}
			else if (sKey == "multi")
			{
				if (object_cast<bool>(Value))
				{
					ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
				}
			}
		}
	}
	char buf[MAX_LINE + 1] = { 0 };
	int nCount = GetCurrentDirectoryA(MAX_LINE, buf);

	// switch to windowed mode to display the win32 common dialog.
	bool bOldWindowed = CGlobals::GetApp()->IsWindowedMode();  // Preserve original windowed flag
	if (bOldWindowed == false)
	{
		CGlobals::GetApp()->SetWindowedMode(true);
	}

	bool bResult = bIsSavingFile ? (!!::GetSaveFileNameA(&ofn)) : (!!::GetOpenFileNameA(&ofn));

	if (bOldWindowed == false)
	{
		CGlobals::GetApp()->SetWindowedMode(false);
	}

	// reset directory. 
	if (nCount > 0)
		SetCurrentDirectoryA(buf);

	inout["result"] = bResult;
	if (bResult)
	{
		if (ofn.lpstrFile)
		{
			if ((ofn.Flags & OFN_ALLOWMULTISELECT) != 0)
			{
				for (int i = 0; i < MAX_LINE; ++i)
				{
					if (ofn.lpstrFile[i] == 0 && ofn.lpstrFile[i + 1] != 0)
						ofn.lpstrFile[i] = '|';
				}
			}
			string filename = ofn.lpstrFile;
			inout["filename"] = filename;
		}
	}
	return bResult;
#else
	return false;
#endif
}
*/

double ParaScripting::ParaGlobal::getAccurateTime()
{
	static int64 start_time = CParaUtils::GetTimeMS();
	double elapsedTime = (double)(CParaUtils::GetTimeMS() - start_time);
	return elapsedTime / 1000;
}

bool ParaScripting::ParaBootStrapper::LoadFromFile(const char* sXMLfile)
{
	return CBootStrapper::GetSingleton()->LoadFromFile((sXMLfile != 0) ? sXMLfile : "");
}

bool ParaScripting::ParaBootStrapper::SaveToFile(const char*  sXMLfile)
{
	return CBootStrapper::GetSingleton()->SaveToFile((sXMLfile != 0) ? sXMLfile : "");
}

void ParaScripting::ParaBootStrapper::LoadDefault()
{
	CBootStrapper::GetSingleton()->LoadDefault();
}


const char* ParaScripting::ParaBootStrapper::GetMainLoopFile()
{
	return CBootStrapper::GetSingleton()->GetMainLoopFile().c_str();
}

void ParaScripting::ParaBootStrapper::SetMainLoopFile(const char* sMainFile)
{
	CBootStrapper::GetSingleton()->SetMainLoopFile((sMainFile != 0) ? sMainFile : "");
}

// for LuaJit, only for function that maybe called millions of time per second
extern "C" {

	PE_CORE_DECL uint32_t ParaGlobal_timeGetTime()
	{
		return ParaScripting::ParaGlobal::timeGetTime();
	}

	PE_CORE_DECL double ParaGlobal_getAccurateTime()
	{
		return ParaScripting::ParaGlobal::getAccurateTime();
	}

	PE_CORE_DECL void ParaGlobal_WriteToLogFile(const char* strMessage)
	{
		ParaScripting::ParaGlobal::WriteToLogFile(strMessage);
	}

	PE_CORE_DECL bool ParaGlobal_SetFieldCData(const char* sFieldname, void* pValue)
	{
		if (g_selected_attr_obj.IsValid())
		{
			if (g_selected_attr_obj.IsValid())
			{
				CAttributeField* pField = g_selected_attr_obj.m_pAttClass->GetField(sFieldname);
				if (pField != 0 && pField->HasSetFunction())
				{
					pField->Set(g_selected_attr_obj.m_pAttribute.get(), pValue);
					return true;
				}
			}
		}
		return false;
	}

	PE_CORE_DECL bool ParaGlobal_GetFieldCData(const char* sFieldname, void* pValueOut)
	{
		if (g_selected_attr_obj.IsValid())
		{
			CAttributeField* pField = g_selected_attr_obj.m_pAttClass->GetField(sFieldname);
			if (pField != 0 && pField->HasGetFunction())
			{
				pField->Get(g_selected_attr_obj.m_pAttribute.get(), pValueOut);
				return true;
			}
		}
		return false;
	}

	PE_CORE_DECL void* ParaGlobal_GetLuaState(const char* name)
	{
		auto pRuntimeState = NPL::CNPLRuntime::GetInstance()->GetRuntimeState(name);
		if (pRuntimeState)
		{
			return pRuntimeState->GetLuaState();
		}
		return nullptr;
	}
};
