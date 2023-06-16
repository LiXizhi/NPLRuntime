//-----------------------------------------------------------------------------
// Class:	All test cases in NPL 
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2009.7.10
// Desc: cross platformed. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif
extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace luabind;

#include "NPLRuntime.h"
#include "NPLHelper.h"
#include "ParaScriptingNPL.h"


#include "memdebug.h"

#ifdef _DEBUG
// generally lua_loadbuffer() is 25% faster than MsgStringToLuaObject
// checking msg data is almost same as lua_loadbuffer
// PerfTest(input.interpreter());
void PerfTest(lua_State* pState)
{
	// 2.13s vs. 1.65s
	// const char* input = "msg={nid=10, name=\"value\", tab={name1=\"value1\"}, name2=\"valasddddddddddddue1\", name3=\"vaasdddddddddddddddddddddddddddlue1\", name4=\"vasddddddddddddddddddalue1\", name5=\"vaasddsaddddddddddddddddddddddddddddddddddddddddddddlue1\", name6=\"value1asdasdasdasdasdasdad\"}";
	// 0.99s vs. 0.75s vs. 0.018 vs. 0.711
	const char* input = "msg={nid=10, name=\"value\", tab={name1=\"value1\"}}";
	int nCount = 100000;

	{
		PERF1("StringToLuaObject")
			for(int i=0;i<nCount;++i)
			{
				NPL::NPLHelper::MsgStringToLuaObject(input, -1, pState);
			}
	}
	{
		PERF1("DoString")
			for(int i=0;i<nCount;++i)
			{
				int nResult = luaL_loadbuffer(pState, input, strlen(input), "");
				if (nResult==0) {
					nResult = lua_pcall(pState, 0, LUA_MULTRET, 0);
				}
			}
	}
	{
		PERF1("CreatString")
			for(int i=0;i<nCount;++i)
			{
				string abc;
				abc.reserve(300);
			}
	}
	{
		PERF1("CheckMsgData")
			for(int i=0;i<nCount;++i)
			{
				NPL::NPLHelper::IsSCodePureData(input);
			}
	}
}
extern void Test_NPLTable();
extern void Test_ServiceLog();
#endif

namespace ParaScripting
{
	void CNPL::test(const object& input)
	{
#ifdef _DEBUG
		std::string testcase;
		if(type(input) == LUA_TSTRING)
		{
			testcase = object_cast<const char*>(input);
		}
		Test_ServiceLog();
		// Test_NPLTable();
#endif
	}
}// namespace ParaScripting