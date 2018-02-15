//-----------------------------------------------------------------------------
// Class:	ParaScriptingIPC
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.27
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef PARAENGINE_CLIENT
#include "ParaScriptingIPC.h"
#include "IPCManager.h"
#include "NPLHelper.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>

using namespace luabind;
using namespace ParaScripting;
using namespace ParaEngine;

ParaScripting::ParaIPCQueue::ParaIPCQueue( const char* sQueueName, int nUsage )
{
	m_pQueue.reset(new CInterprocessQueue(sQueueName, (IPQueueUsageEnum)nUsage));
}

ParaScripting::ParaIPCQueue::ParaIPCQueue( boost::shared_ptr<CInterprocessQueue>& pQueue ) :m_pQueue(pQueue)
{
}

bool ParaScripting::ParaIPCQueue::IsValid()
{
	return m_pQueue && m_pQueue->IsValid();
}

const char* ParaScripting::ParaIPCQueue::GetName()
{
	if(m_pQueue)
	{
		m_pQueue->GetName().c_str();
	}
	return "";
}

void ParaScripting::ParaIPCQueue::Cleanup()
{
	if(m_pQueue)
		m_pQueue->Cleanup();
}

bool ParaScripting::ParaIPCQueue::Remove()
{
	return m_pQueue && m_pQueue->Remove();
}

void ParaScripting::ParaIPCQueue::Clear()
{
	if(m_pQueue)
		m_pQueue->Clear();
}

bool ParaScripting::ParaIPCQueue::ConvertObjectToMsg(const object& msg, InterProcessMessage& outMsg, int & nPriority)
{
	if(type(msg) == LUA_TTABLE)
	{
		for (luabind::iterator itCur(msg), itEnd;itCur!=itEnd;++itCur)
		{
			const object& key = itCur.key();
			if(type(key) == LUA_TSTRING)
			{
				std::string sKey = object_cast<const char*>(key);
				const object& input = *itCur;
				if(sKey == "method")
				{
					if(type(input) == LUA_TSTRING)
						outMsg.m_method = object_cast<const char*>(input);
				}
				else if(sKey == "from")
				{
					if(type(input) == LUA_TSTRING)
						outMsg.m_from = object_cast<const char*>(input);
				}
				else if(sKey == "to")
				{
					if(type(input) == LUA_TSTRING)
						outMsg.m_to = object_cast<const char*>(input);
				}
				else if(sKey == "filename")
				{
					if(type(input) == LUA_TSTRING)
						outMsg.m_filename = object_cast<const char*>(input);
				}
				else if(sKey == "type")
				{
					if(type(input) == LUA_TNUMBER)
						outMsg.m_nMsgType = object_cast<int>(input);
				}
				else if(sKey == "param1")
				{
					if(type(input) == LUA_TNUMBER)
						outMsg.m_nParam1 = object_cast<int>(input);
				}
				else if(sKey == "param2")
				{
					if(type(input) == LUA_TNUMBER)
						outMsg.m_nParam2 = object_cast<int>(input);
				}
				else if(sKey == "priority")
				{
					if(type(input) == LUA_TNUMBER)
						nPriority = object_cast<int>(input);
				}
				else if(sKey == "code")
				{
					if(type(input) == LUA_TTABLE)
					{
						// encode the table or variables in to the "msg" variable and serialize in to sCode string.
						// it is the receiver's responsibility to validate the scode according to its source.
						NPL::NPLHelper::SerializeToSCode("msg", input, outMsg.m_code);
					}
					else if(type(input) == LUA_TSTRING)
					{
						// For security reason, shall we allow any string? well, it is the receiver's job to verify it. 
						outMsg.m_code = object_cast<const char*>(input);
					}
				}
			}
		}
	}
	return true;
}

int ParaScripting::ParaIPCQueue::send( const object& msg )
{
	if(m_pQueue)
	{
		InterProcessMessage outMsg;
		int nPriority = 0;
		if(ConvertObjectToMsg(msg, outMsg, nPriority))
		{
			return m_pQueue->send(outMsg, nPriority);
		}
	}
	return IPRC_FAILED;
}

int ParaScripting::ParaIPCQueue::try_send( const object& msg )
{
	if(m_pQueue)
	{
		InterProcessMessage outMsg;
		int nPriority = 0;
		if(ConvertObjectToMsg(msg, outMsg, nPriority))
		{
			return m_pQueue->try_send(outMsg, nPriority);
		}
	}
	return IPRC_FAILED;
}

bool ParaScripting::ParaIPCQueue::ConvertMsgToObject(InterProcessMessage& inMsg, const object& msg, int nPriority)
{
	msg["method"] = inMsg.m_method;
	msg["from"] = inMsg.m_from;
	msg["type"] = inMsg.m_nMsgType;
	msg["param1"] = inMsg.m_nParam1;
	msg["param2"] = inMsg.m_nParam2;
	msg["filename"] = inMsg.m_filename;
	bool bIsTable = false;
	int nCodeLength = (int)(inMsg.m_code.size());
	if(nCodeLength>4 && inMsg.m_code[4] == '{' && inMsg.m_code[nCodeLength-1] == '}')
	{
		luabind::object out;
		bIsTable = NPL::NPLHelper::StringToLuaObject(inMsg.m_code.c_str()+4, nCodeLength-4, out, msg.interpreter());
		if(bIsTable)
		{
			msg["code"] = out;
		}
	}
	if(!bIsTable)
	{
		msg["code"] = inMsg.m_code;
	}
	
	msg["priority"] = nPriority;
	return true;
}

int ParaScripting::ParaIPCQueue::receive( const object& msg )
{
	if(m_pQueue)
	{
		InterProcessMessage inMsg;
		
		unsigned int nPriority = 0;
		int nReturnCode = m_pQueue->receive(inMsg, nPriority);
		if(nReturnCode == IPRC_OK)
		{
			if(ConvertMsgToObject(inMsg, msg, (int)nPriority))
				return IPRC_OK;
			else
				return IPRC_FAILED;
		}
		return nReturnCode;
	}
	return IPRC_FAILED;
}

int ParaScripting::ParaIPCQueue::try_receive( const object& msg )
{
	if(m_pQueue)
	{
		InterProcessMessage inMsg;

		unsigned int nPriority = 0;
		int nReturnCode = m_pQueue->try_receive(inMsg, nPriority);
		if(nReturnCode == IPRC_OK)
		{
			if(ConvertMsgToObject(inMsg, msg, (int)nPriority))
				return IPRC_OK;
			else
				return IPRC_FAILED;
		}
		return nReturnCode;
	}
	return IPRC_FAILED;
}

ParaScripting::ParaIPCQueue ParaScripting::ParaIPC::CreateGetQueue( const char* filename, int nCreationFlag )
{
	CInterprocessQueuePtr pQueue = CIPCManager::GetInstance()->CreateGetQueue(filename, (IPQueueUsageEnum) nCreationFlag);
	return ParaIPCQueue(pQueue);
}

void ParaScripting::ParaIPC::Clear()
{
	CIPCManager::GetInstance()->Clear();
}

void ParaScripting::ParaIPC::RemoveQueue( const char* name )
{
	CIPCManager::GetInstance()->RemoveQueue(name);
}
#endif