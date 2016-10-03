#include "ParaEngine.h"
#ifdef PARAENGINE_MOBILE
#include "local_bridge_manual.h"
#include "tolua_fix.h"
#include "platform/OpenGLWrapper.h"
#include "extensions/cocos-ext.h"
#include "CCLuaStack.h"
#include "CCLuaValue.h"
#include "CCLuaEngine.h"
#include "AppDelegate.h"
#include "protocol/pb.h"
#include "ParaEngine.h"
#include "IO/FileUtils.h"
using namespace cocos2d;
using namespace cocos2d::extension;
#ifdef PLATFORM_ANDROID
#include "jni/hellolua/Java_org_cocos2dx_lua_LocalBridge.h"
#elif defined PLATFORM_IOS
#include "platform/ios/LocalBridgeHandler_ios.h"
#endif
static int lua_localCall(lua_State* L)
{
	const char* api_key = tolua_tostring(L, 1, "");
	IOString* iostring = (IOString*)luaL_checkudata(L, 2, IOSTRING_META);
	//LUA_FUNCTION callback = toluafix_ref_function(L, 3, 0);
	LUA_FUNCTION callback = 0;
	const char* buff = iostring->buf;
	int size = iostring->size;
	//OUTPUT_LOG("===================lua_localCall api_key:%s\n", api_key);
	//OUTPUT_LOG("===================lua_localCall buff:%s\n", buff);
	//OUTPUT_LOG("===================lua_localCall buff size:%d\n", size);
#ifdef PLATFORM_ANDROID
	jni_localCall(api_key, buff, size, callback);
#elif defined PLATFORM_IOS
	LocalBridgeHandler::ios_localCall(api_key, buff);
#endif
	return 1;
}
int register_all_local_bridge(lua_State* L)
{
	if (nullptr == L)
		return 0;

	tolua_open(L);
	tolua_module(L, NULL, 0);
	tolua_beginmodule(L, NULL);
	tolua_module(L, "LocalBridge", 0);
	tolua_beginmodule(L, "LocalBridge");
	tolua_function(L, "call", lua_localCall);
	tolua_endmodule(L);

	return 1;
}
#endif
