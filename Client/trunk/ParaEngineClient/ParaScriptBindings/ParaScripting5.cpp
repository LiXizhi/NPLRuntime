//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState and ParaScripting
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised 2: Andy Wang 2007.8
//		Note: move in class LoadHAPI_NPL, LoadHAPI_ResourceManager and LoadHAPI_SceneManager from ParaScripting.cpp
// Desc: partially cross platformed
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "util/StringHelper.h"
#include "util/bitlib_lua.h"
#include "util/lua_pack.h"

using namespace ParaEngine;

#if defined(PARAENGINE_CLIENT) && !defined(NPLRUNTIME)
#define USE_NPL_PROFILER
#define USE_NPL_CURL
#endif
/** 
for luabind, The main drawback of this approach is that the compilation time will increase for the file 
that does the registration, it is therefore recommended that you register everything in the same cpp-file.
*/
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"

#if defined(USE_NPL_PROFILER)
	extern int luaopen_cURL(lua_State *L);
	extern int profiler_pause(lua_State *L);
	extern int profiler_resume(lua_State *L);
	extern int profiler_init(lua_State *L);
	extern int profiler_stop(lua_State *L);
#endif
	extern int luaopen_lfs (lua_State *L);
#if defined(USE_NPL_DARKNET)
	extern int luaopen_LuaDarkNet(lua_State *L);
#endif
	// extern int luaopen_profiler(lua_State *L);
}

#include <luabind/luabind.hpp>
#include <luabind/out_value_policy.hpp>
#include <luabind/raw_policy.hpp>

#include "ParaScriptingIO.h"
#include "ParaScriptingIC.h"
#include "ParaScriptingGlobal.h"
#include "ParaScriptingGUI.h"
#include "ParaScriptingMisc.h"

#include "ParaScripting.h"
#include "luaSQLite.h"
#include "ic/ICConfigManager.h"
#include "ParaScriptingMovie.h"

namespace ParaScripting
{
	/** for registering functions */
	typedef struct { const char * name; int (*func)(lua_State *); } lua_func_entry;
	typedef struct { const char * name; int value; } lua_vale_entry;

	/** for registering functions */
	static void lua_register_funcs_totable(lua_State * L, lua_func_entry entries[])
	{
		int index;
		lua_newtable(L);
		for( index=0; entries[index].name; index++)
		{
			lua_pushstring(L, entries[index].name);
			lua_pushcfunction(L, entries[index].func);
			lua_rawset(L, -3);
		}
	}

	/** for registering values */
	static void lua_register_values_totable(lua_State * L, lua_vale_entry entries[])
	{
		int index;
		lua_newtable(L);
		for( index=0; entries[index].name; index++)
		{
			lua_pushstring(L, entries[index].name);
			lua_pushnumber(L, entries[index].value);
			lua_rawset(L, -3);
		}
	}
	/**
	* register functions into a given table. create the table if it does not exist.
	*/
	static void lua_register_on_table(lua_State* L, char const* const tableName, lua_func_entry entries[])
	{
		lua_getfield(L, LUA_GLOBALSINDEX, tableName);  // push table onto stack
		if (!lua_istable(L, -1))                       // not a table, create it
		{
			lua_createtable(L, 0, 1);      // create new table
			lua_setfield(L, LUA_GLOBALSINDEX, tableName);  // add it to global context

			// reset table on stack
			lua_pop(L, 1);                 // pop table (nil value) from stack
			lua_getfield(L, LUA_GLOBALSINDEX, tableName);  // push table onto stack
		}
		int index;
		for (index = 0; entries[index].name; index++)
		{
			lua_pushstring(L, entries[index].name);
			lua_pushcfunction(L, entries[index].func);
			lua_rawset(L, -3);
		}
		lua_pop(L, 1);                     // pop table from stack
	}


/** 
* @ingroup global
e.g. print("Hello ParaEngine %d", 10);
Receives any number of arguments, and prints their values in stdout, using the tostring function to 
convert them to strings. This function is not intended for formatted output, but only as a quick way to show a value, 
typically for debugging. For formatted output, use string.format */
static int NPL_Print(lua_State *L) {
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s;
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushvalue(L, i);   /* value to print */
		lua_call(L, 1, 1);
		s = lua_tostring(L, -1);  /* get result */
		if (s == NULL)
			return luaL_error(L, "`tostring' must return a string to `print'");
		if (i>1) 
			OUTPUT_LOG("\t");
		OUTPUT_LOG("%s", s);
		lua_pop(L, 1);  /* pop result */
	}
	OUTPUT_LOG("\r\n");
	return 0;
}

#define L_ESC '%'

/** 
* @ingroup global
this function is the same as string.format, except that it only support %d, and %s, and %f (always printed as %.2f)
We do not use sprintf internally, so it runs much faster than string.format which uses sprintf. 
e.g. a = format("%s hello %d, %f", "Hello ParaEngine",10, 10.1234);
*/
static int NPL_Format(lua_State *L) 
{
	int arg = 1;
	size_t sfl;
	const char *strfrmt = luaL_checklstring(L, arg, &sfl);
	const char *strfrmt_end = strfrmt+sfl;
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	while (strfrmt < strfrmt_end) 
	{
		if (*strfrmt != L_ESC)
			luaL_addchar(&b, *strfrmt++);
		else if (*++strfrmt == L_ESC)
			luaL_addchar(&b, *strfrmt++);  /* %% */
		else 
		{ /* format item */
			arg++;
			switch (*strfrmt++) 
			{
			case 's': 
				{
					size_t l;
					const char *s = luaL_checklstring(L, arg, &l);
					if (l >= 100) {
						/* no precision and string is too long to be formatted;
						keep original string */
						lua_pushvalue(L, arg);
						luaL_addvalue(&b);
					}
					else {
						luaL_addlstring(&b, s, l);
					}
					break;
				}
			case 'd': 
				{
					char buff[15];
					int nSize = ParaEngine::StringHelper::fast_itoa((int64)luaL_checknumber(L, arg), buff, 15);
					luaL_addlstring(&b, buff, nSize);
					break;
				}
			case 'f': 
				{
					char buff[20];
					// similar to "%.2f" but without trailing zeros. 
					int nSize = ParaEngine::StringHelper::fast_dtoa((double)luaL_checknumber(L, arg), buff, 20, 2);
					luaL_addlstring(&b, buff, nSize);
					break;
				}
			default: 
				{
					return luaL_error(L, "invalid option " LUA_QL("%%%c") " to "
						LUA_QL("format"), *(strfrmt - 1));
				}
			}
		}
	}
	luaL_pushresult(&b);
	return 1;
}

lua_func_entry luaxml_api_entries[] = {
	{ "LuaXML_ParseFile",		ParaXML::LuaXML_ParseFile },
	{ "LuaXML_ParseString", ParaXML::LuaXML_ParseString },
	{ 0, 0 }
};


/**
* the luaxml api will be returned in a table, usually we can assign this table like this
* ParaXML = luaopen_luaxml();
*/
static int luaopen_luaxml(lua_State *L)
{
	lua_register_funcs_totable(L, luaxml_api_entries);
	return 1;	/* api*/
}

#if defined(USE_NPL_PROFILER)
lua_func_entry profiler_api_entries[] = {
	{ "pause", profiler_pause },
	{ "resume", profiler_resume },
	{ "start", profiler_init },
	{ "stop", profiler_stop },
	{ NULL, NULL }
};

/**
* the profiler_api_entries api will be returned in a table, usually we can assign this table like this
* profiler = luaopen_profiler_table();
*/
static int luaopen_profiler_table(lua_State *L)
{
	OUTPUT_LOG("NPL Profiler is loaded\n");
	OUTPUT_LOG("warning: security alert! io functions are turned on in npl profiler. Do not use this in release build. ");
	luaopen_io(L);
	luaopen_os(L);

	lua_register_funcs_totable(L, profiler_api_entries);
	return 1;	/* api*/
}
#endif

void CNPLScriptingState::LoadHAPI_Globals()
{
	using namespace luabind;
	lua_State* L = GetLuaState();

	// override the LUA base print function, and output to log file instead.
#ifndef PARAENGINE_MOBILE
	lua_register(L, "print", NPL_Print);
#endif
	// provide a faster format function. 
	lua_register(L, "format", NPL_Format); 
	
	// load sqlite3
	lua_register(L, "luaopen_sqlite3", luaopen_sqlite3);
	// load bit lib
	lua_register(L, "luaopen_bit", luaopen_bit_local);
	// load string.pack
	lua_register(L, "luaopen_lua_pack", luaopen_lua_pack);

#if defined(USE_NPL_CURL)
	// load cURL
	lua_register(L, "luaopen_cURL", luaopen_cURL);
#endif

#if defined(USE_NPL_PROFILER)
	// load profiler
	lua_register(L, "luaopen_profiler", luaopen_profiler_table);
#endif

	// load lua file system
	lua_register(L, "luaopen_lfs", luaopen_lfs);

#if defined(USE_NPL_DARKNET)
	luaopen_LuaDarkNet(L);
#endif

	// load luaxml
	lua_register(L, "luaopen_luaxml", luaopen_luaxml);
	luaL_dostring(L, "ParaXML = luaopen_luaxml()"); // load function to ParaXML namespace.
	module(L)
	[
		namespace_("ParaXML")
		[
			// declarations
			def("IsWhiteSpaceCondensed", & ParaXML::IsWhiteSpaceCondensed),
			def("SetCondenseWhiteSpace", &ParaXML::SetCondenseWhiteSpace)
		]
	];
	module(L)
	[
		def("log", & ParaGlobal::WriteToLogFile),

		namespace_("ParaGlobal")
		[
			class_<ParaServiceLogger>("ParaServiceLogger")
			.def(constructor<>())
			.def("GetLevel", &ParaServiceLogger::GetLevel)
			.def("SetLevel", &ParaServiceLogger::SetLevel)
			.def("IsEnabledFor", &ParaServiceLogger::IsEnabledFor)
			.def("SetAppendMode", &ParaServiceLogger::SetAppendMode)
			.def("SetForceFlush", &ParaServiceLogger::SetForceFlush)
			.def("SetLogFile", &ParaServiceLogger::SetLogFile)
			.def("log", &ParaServiceLogger::log),

			// declarations
			def("ExitApp", & ParaGlobal::ExitApp),
			def("Exit", & ParaGlobal::Exit),
			def("WriteToConsole", & ParaGlobal::WriteToConsole),
			def("WriteToLogFile", & ParaGlobal::WriteToLogFile),
			def("GetLogPos", & ParaGlobal::GetLogPos),
			def("GetLog", & ParaGlobal::GetLog),
			def("GetLogger", & ParaGlobal::GetLogger),
			def("SetGameStatus", & ParaGlobal::SetGameStatus),
			def("GetGameTime", & ParaGlobal::GetGameTime),
			def("GetTimeFormat", & ParaGlobal::GetTimeFormat),
			def("GetDateFormat", & ParaGlobal::GetDateFormat),
			def("timeGetTime", & ParaGlobal::timeGetTime),
			def("getAccurateTime", & ParaGlobal::getAccurateTime),
			def("GetSysDateTime", & ParaGlobal::GetSysDateTime),
			def("random", & ParaGlobal::random),
			def("GenerateUniqueID", & ParaGlobal::GenerateUniqueID),
			def("SetGameLoop", & ParaGlobal::SetGameLoop),
			def("SetGameLoopInterval", & ParaGlobal::SetGameLoopInterval),
			def("SaveObject", & ParaGlobal::SaveObject),
			def("LoadObject", & ParaGlobal::LoadObject), //, pure_out_value(_2)),
			def("CreateProcess", & ParaGlobal::CreateProcess), 
	 		def("IsPortAvailable", &ParaGlobal::IsPortAvailable),
	 		def("Execute", &ParaGlobal::Execute),
			def("ShellExecute", &ParaGlobal::ShellExecute),
			def("ExecWmicCmd", & ParaGlobal::ExecWmicCmd),
			def("OpenFileDialog", & ParaGlobal::OpenFileDialog), 

			def("WriteRegStr", & ParaGlobal::WriteRegStr), 
			def("ReadRegStr", & ParaGlobal::ReadRegStr), 
			def("ReadRegDWORD", & ParaGlobal::ReadRegDWORD), 
			def("WriteRegDWORD", & ParaGlobal::WriteRegDWORD), 
			
			def("SelectAttributeObject", &ParaGlobal::SelectAttributeObject),

			def("log", & ParaGlobal::WriteToLogFile)
		]
	];
	module(L)
	[
		namespace_("ParaBootStrapper")
		[
			// declarations
			def("LoadFromFile", & ParaBootStrapper::LoadFromFile),
			def("SaveToFile", & ParaBootStrapper::SaveToFile),
			def("GetMainLoopFile", & ParaBootStrapper::GetMainLoopFile),
			def("SetMainLoopFile", & ParaBootStrapper::SetMainLoopFile),
			def("LoadDefault", & ParaBootStrapper::LoadDefault)
		]
	];
	module(L)
	[
		class_<ParaAttributeObject>("ParaAttributeObject")
		.def(constructor<>())
		.def("IsValid", &ParaAttributeObject::IsValid)
		.def("equals", &ParaAttributeObject::equals)
		.def("GetChild", &ParaAttributeObject::GetChild)
		.def("GetChildAt", &ParaAttributeObject::GetChildAt)
		.def("GetChildAt", &ParaAttributeObject::GetChildAt2)
		.def("GetChildCount", &ParaAttributeObject::GetChildCount)
		.def("GetChildCount", &ParaAttributeObject::GetChildCount2)
		.def("GetColumnCount", &ParaAttributeObject::GetColumnCount)
		.def("QueryObject", &ParaAttributeObject::QueryObject)
		.def("QueryUIObject", &ParaAttributeObject::QueryUIObject)
		.def("AddChild", &ParaAttributeObject::AddChild)
		.def("GetClassID", &ParaAttributeObject::GetClassID)
		.def("GetClassName", &ParaAttributeObject::GetClassName)
		.def("GetClassDescription", &ParaAttributeObject::GetClassDescription)
		.def("SetOrder", &ParaAttributeObject::SetOrder)
		.def("GetOrder", &ParaAttributeObject::GetOrder)
		.def("IsFieldReadOnly", &ParaAttributeObject::IsFieldReadOnly)
		.def("GetFieldNum", &ParaAttributeObject::GetFieldNum)
		.def("GetFieldName", &ParaAttributeObject::GetFieldName)
		.def("GetFieldIndex", &ParaAttributeObject::GetFieldIndex)
		.def("GetFieldType", &ParaAttributeObject::GetFieldType)
		.def("GetFieldSchematics", &ParaAttributeObject::GetFieldSchematics)
		.def("GetSchematicsMinMax", &ParaAttributeObject::GetSchematicsMinMax, pure_out_value(_5) + pure_out_value(_6))
		.def("GetSchematicsType", &ParaAttributeObject::GetSchematicsType)
		.def("GetField", &ParaAttributeObject::GetField)
		.def("GetField", &ParaAttributeObject::GetField2, raw(_3))
		.def("SetField", &ParaAttributeObject::SetField)
		.def("CallField", &ParaAttributeObject::CallField)
		.def("ResetField", &ParaAttributeObject::ResetField)
		.def("InvokeEditor", &ParaAttributeObject::InvokeEditor)
		.def("PrintObject", &ParaAttributeObject::PrintObject)
		.def("GetDynamicField", &ParaAttributeObject::GetDynamicField)
		.def("GetDynamicField", &ParaAttributeObject::GetDynamicField_)
		.def("GetDynamicFieldNameByIndex", &ParaAttributeObject::GetDynamicFieldNameByIndex)
		.def("GetDynamicFieldCount", &ParaAttributeObject::GetDynamicFieldCount)
		.def("SetDynamicField", &ParaAttributeObject::SetDynamicField)
		.def("SetDynamicField", &ParaAttributeObject::SetDynamicField_)
		.def("AddDynamicField", &ParaAttributeObject::AddDynamicField)
		.def("RemoveAllDynamicFields", &ParaAttributeObject::RemoveAllDynamicFields)
		.def("GetFieldKeyNums", &ParaAttributeObject::GetFieldKeyNums)
		.def("SetFieldKeyNums", &ParaAttributeObject::SetFieldKeyNums)
		.def("SetFieldKeyTime", &ParaAttributeObject::SetFieldKeyTime)
		.def("GetFieldKeyTime", &ParaAttributeObject::GetFieldKeyTime)
		.def("SetFieldKeyValue", &ParaAttributeObject::SetFieldKeyValue)
		.def("GetFieldKeyValue", &ParaAttributeObject::GetFieldKeyValue)
		.def("GetAttributeObject", &ParaAttributeObject::GetAttributeObject)
	];
	module(L)
	[
		namespace_("ParaEngine")
		[
			// declarations
			def("GetVersion", & ParaScripting::CParaEngine::GetVersion),
			def("GetAuthorTo", & ParaScripting::CParaEngine::GetAuthorTo),
			def("GetCopyright", & ParaScripting::CParaEngine::GetCopyright),
			def("ForceRender", & ParaScripting::CParaEngine::ForceRender),
			def("Sleep", & ParaScripting::CParaEngine::Sleep),
			def("GetLocale", & ParaEngineSettings::GetLocaleS),
			def("SetLocale", & ParaEngineSettings::SetLocaleS),
			def("SetWindowText", & ParaEngineSettings::SetWindowText),
			def("GetWindowText", & ParaEngineSettings::GetWindowText),
			def("WriteConfigFile", & ParaEngineSettings::WriteConfigFile),
			def("IsDebugging", & ParaEngineSettings::IsDebugging),
			def("GetAppCommandLine", & ParaEngineSettings::GetAppCommandLine),
			def("GetAppCommandLineByParam", & ParaEngineSettings::GetAppCommandLineByParam),
			def("SetAppCommandLine", & ParaEngineSettings::SetAppCommandLine),
			def("ActivateProduct", & ParaEngineSettings::ActivateProduct),
			def("IsProductActivated", & ParaEngineSettings::IsProductActivated),
			def("IsWindowedMode", & ParaEngineSettings::IsWindowedMode),
			def("SetWindowedMode", & ParaEngineSettings::SetWindowedMode),
			def("SetMouseInverse", & ParaEngineSettings::SetMouseInverse_s),
			def("GetMouseInverse", & ParaEngineSettings::GetMouseInverse_s),
			def("SaveParaXMesh", & ParaScripting::CParaEngine::SaveParaXMesh),
			//def("GetCtorColor", & ParaEngineSettings::GetCtorColorS),
			//def("SetCtorColor", & ParaEngineSettings::SetCtorColorS),
			def("GetCtorHeight", & ParaEngineSettings::GetCtorHeightS),
			def("SetCtorHeight", & ParaEngineSettings::SetCtorHeightS),
			def("GetCtorSpeed", & ParaEngineSettings::GetCtorSpeedS),
			def("SetCtorSpeed", & ParaEngineSettings::SetCtorSpeedS),
			def("GetStats", & ParaEngineSettings::GetStats),
			def("GetClientSize", & ParaEngineSettings::GetClientSize, pure_out_value(_1)+pure_out_value(_2)),
			def("LoadGameEffectSet", & ParaEngineSettings::LoadGameEffectSetS),
			def("GetGameEffectSet", & ParaEngineSettings::GetGameEffectSetS),
			def("IsEditing", & ParaEngineSettings::IsEditing),
			def("SetEditingMode", & ParaEngineSettings::SetEditingMode),
			def("GetWatermark", & ParaScripting::CParaEngine::GetWatermark),
			def("GenerateActivationCodes", & ParaEngineSettings::GenerateActivationCodes),
			def("GetProductKey", & ParaEngineSettings::GetProductKey),
			def("IsPassiveRenderingEnabled", & ParaEngineSettings::IsPassiveRenderingEnabled),
			def("EnablePassiveRendering", & ParaEngineSettings::EnablePassiveRendering),
			def("GetRenderTarget", & ParaScripting::CParaEngine::GetRenderTarget),
			def("SetRenderTarget", & ParaScripting::CParaEngine::SetRenderTarget),
			def("SetRenderTarget", &ParaScripting::CParaEngine::SetRenderTarget2),
			def("SetRenderTarget2", &ParaScripting::CParaEngine::SetRenderTarget2),
			def("StretchRect", & ParaScripting::CParaEngine::StretchRect),
			def("SetVertexDeclaration", & ParaScripting::CParaEngine::SetVertexDeclaration),
			def("DrawQuad", & ParaScripting::CParaEngine::DrawQuad),
			def("DrawQuad2", &ParaScripting::CParaEngine::DrawQuad2),
			def("GetAsyncLoaderItemsLeft", & ParaEngineSettings::GetAsyncLoaderItemsLeftS),
			def("GetAsyncLoaderBytesReceived", & ParaEngineSettings::GetAsyncLoaderBytesReceivedS),
			def("GetViewportAttributeObject", &ParaScripting::CParaEngine::GetViewportAttributeObject),
			def("GetAttributeObject", &ParaScripting::CParaEngine::GetAttributeObject)
		]
	];
	module(L)
	[
		namespace_("ParaMovie")
		[
			// declarations
			def("GetAttributeObject", &ParaScripting::ParaMovie::GetAttributeObject),
			def("TakeScreenShot_Async", &ParaScripting::ParaMovie::TakeScreenShot_Async),
			def("TakeScreenShot_Async", &ParaScripting::ParaMovie::TakeScreenShot2_Async),
			def("TakeScreenShot_Async", &ParaScripting::ParaMovie::TakeScreenShot3_Async),
			def("TakeScreenShot", &ParaScripting::ParaMovie::TakeScreenShot),
			def("TakeScreenShot", &ParaScripting::ParaMovie::TakeScreenShot3),
			def("RenderToTexture", & ParaScripting::ParaMovie::RenderToTexture),
			def("ResizeImage", & ParaScripting::ParaMovie::ResizeImage),
			def("GetImageInfo", & ParaScripting::ParaMovie::GetImageInfo, pure_out_value(_2) + pure_out_value(_3) + pure_out_value(_4)),
			def("SetEncodeMethod", & ParaScripting::ParaMovie::SetEncodeMethod),
			def("GetEncodeMethod", & ParaScripting::ParaMovie::GetEncodeMethod),
			def("BeginCapture", & ParaScripting::ParaMovie::BeginCapture),
			def("EndCapture", & ParaScripting::ParaMovie::EndCapture),
			def("PauseCapture", & ParaScripting::ParaMovie::PauseCapture),
			def("ResumeCapture", & ParaScripting::ParaMovie::ResumeCapture),
			def("IsInCaptureSession", & ParaScripting::ParaMovie::IsInCaptureSession),
			def("SetMovieScreenSize", & ParaScripting::ParaMovie::SetMovieScreenSize),
			def("GetMovieScreenSize", & ParaScripting::ParaMovie::GetMovieScreenSize, pure_out_value(_1) + pure_out_value(_2)),
			def("SetCaptureGUI", & ParaScripting::ParaMovie::SetCaptureGUI),
			def("CaptureGUI", & ParaScripting::ParaMovie::CaptureGUI),
			def("GetMovieFileName", & ParaScripting::ParaMovie::GetMovieFileName),
			def("SetRecordingFPS", & ParaScripting::ParaMovie::SetRecordingFPS),
			def("GetRecordingFPS", & ParaScripting::ParaMovie::GetRecordingFPS),
			def("SetStereoCaptureMode", & ParaScripting::ParaMovie::SetStereoCaptureMode),
			def("GetStereoCaptureMode", & ParaScripting::ParaMovie::GetStereoCaptureMode),
			def("SetStereoEyeSeparation", & ParaScripting::ParaMovie::SetStereoEyeSeparation),
			def("GetStereoEyeSeparation", & ParaScripting::ParaMovie::GetStereoEyeSeparation),
			def("IsRecording", & ParaScripting::ParaMovie::IsRecording)
		]
	];

	module(L)
	[
		namespace_("ParaIO")
		[
			// class declarations
			class_<ParaSearchResult>("ParaSearchResult")
				.def(constructor<>())
				.def("IsValid", &ParaSearchResult::IsValid)
				.def("AddResult", &ParaSearchResult::AddResult)
				.def("GetItem", &ParaSearchResult::GetItem)
				.def("GetItemData", &ParaSearchResult::GetItemData)
				.def("GetNumOfResult", &ParaSearchResult::GetNumOfResult)
				.def("GetRootPath", &ParaSearchResult::GetRootPath)
				.def("Release", &ParaSearchResult::Release),
			class_<ParaFileObject>("ParaFileObject")
				.def(constructor<>())
				.def("IsValid", &ParaFileObject::IsValid)
				.def("seek", &ParaFileObject::seek)
				.def("seekRelative", &ParaFileObject::seekRelative)
				.def("getpos", &ParaFileObject::getpos)
				.def("SetFilePointer", &ParaFileObject::SetFilePointer)
				.def("SetEndOfFile", &ParaFileObject::SetEndOfFile)
				.def("GetText", &ParaFileObject::GetText)
				.def("GetText", &ParaFileObject::GetText2)
				.def("write", &ParaFileObject::write)
				.def("WriteBytes", &ParaFileObject::WriteBytes)
				.def("ReadBytes", &ParaFileObject::ReadBytes)
				.def("WriteFloat", &ParaFileObject::WriteFloat)
				.def("ReadFloat", &ParaFileObject::ReadFloat)
				.def("WriteInt", &ParaFileObject::WriteInt)
				.def("ReadInt", &ParaFileObject::ReadInt)
				.def("WriteUInt", &ParaFileObject::WriteUInt)
				.def("ReadUInt", &ParaFileObject::ReadUInt)
				.def("WriteShort", &ParaFileObject::WriteShort)
				.def("ReadShort", &ParaFileObject::ReadShort)
				.def("WriteUShort", &ParaFileObject::WriteUShort)
				.def("ReadUShort", &ParaFileObject::ReadUShort)
				.def("WriteChar", &ParaFileObject::WriteChar)
				.def("ReadChar", &ParaFileObject::ReadChar)
				.def("WriteWord", &ParaFileObject::WriteWord)
				.def("ReadWord", &ParaFileObject::ReadWord)
				.def("WriteDouble", &ParaFileObject::WriteDouble)
				.def("ReadDouble", &ParaFileObject::ReadDouble)
				.def("WriteByte", &ParaFileObject::WriteByte)
				.def("ReadByte", &ParaFileObject::ReadByte)
				.def("GetFileSize", &ParaFileObject::GetFileSize)
				.def("ReadString", &ParaFileObject::ReadString, raw(_3))
				.def("WriteString", &ParaFileObject::WriteString)
				.def("WriteString", &ParaFileObject::WriteString2)
				.def("writeline", &ParaFileObject::writeline)
				.def("readline", &ParaFileObject::readline)
				.def("GetBase64String", &ParaFileObject::GetBase64String)
				.def("SetSegment", &ParaFileObject::SetSegment)
				.def("close", &ParaFileObject::close),
			class_<ParaZipWriter>("ParaZipWriter")
				.def(constructor<>())
				.def("IsValid", &ParaZipWriter::IsValid)
				.def("ZipAdd", &ParaZipWriter::ZipAdd)
				.def("ZipAddData", &ParaZipWriter::ZipAddData)
				.def("ZipAddFolder", &ParaZipWriter::ZipAddFolder)
				.def("AddDirectory", &ParaZipWriter::AddDirectory)
				.def("close", &ParaZipWriter::close),
			class_<ParaFileSystemWatcher>("ParaFileSystemWatcher")
				.def(constructor<>())
				.def("AddDirectory", &ParaFileSystemWatcher::AddDirectory)
				.def("RemoveDirectory", &ParaFileSystemWatcher::RemoveDirectory)
				.def("AddCallback", &ParaFileSystemWatcher::AddCallback),

			// function declarations
			def("CreateDirectory", & ParaIO::CreateDirectory),
			def("CopyFile", & ParaIO::CopyFile),
			def("MoveFile", & ParaIO::MoveFile),
			def("DeleteFile", & ParaIO::DeleteFile),
			def("SetDiskFilePriority", & ParaIO::SetDiskFilePriority),
			def("GetDiskFilePriority", & ParaIO::GetDiskFilePriority),
			def("CreateZip", & ParaIO::CreateZip),
			def("CreateNewFile", & ParaIO::CreateNewFile),
			def("OpenFileWrite", & ParaIO::OpenFileWrite),
			def("OpenFile", & ParaIO::OpenFile),
			def("OpenAssetFile", & ParaIO::OpenAssetFile),
			def("OpenAssetFile", & ParaIO::OpenAssetFile2),
			def("DoesAssetFileExist", & ParaIO::DoesAssetFileExist),
			def("DoesAssetFileExist", & ParaIO::DoesAssetFileExist2),
			def("CheckAssetFile", & ParaIO::CheckAssetFile),
			def("LoadReplaceFile", & ParaIO::LoadReplaceFile),
			def("SyncAssetFile_Async", &ParaIO::SyncAssetFile_Async),
			def("ConvertPathFromUTF8ToAnsci", & ParaIO::ConvertPathFromUTF8ToAnsci),
			
			def("open", & ParaIO::open),
			def("openimage", & ParaIO::openimage),
			def("openimage", &ParaIO::openimage2),
			def("readline", & ParaIO::readline),
			def("readline", &ParaIO::readline2),
			def("WriteString", & ParaIO::WriteString),
			def("write", & ParaIO::write),
			def("DoesFileExist", & ParaIO::DoesFileExist),
			def("DoesFileExist", & ParaIO::DoesFileExist_),
			def("BackupFile", & ParaIO::BackupFile),
			def("SearchFiles", & ParaIO::SearchFiles),
			def("SearchFiles", & ParaIO::SearchFiles_c),
			def("GetCurDirectory", & ParaIO::GetCurDirectory),
			def("GetParentDirectoryFromPath", & ParaIO::GetParentDirectoryFromPath),
			def("AutoFindParaEngineRootPath", & ParaIO::AutoFindParaEngineRootPath),
			def("ChangeFileExtension", & ParaIO::ChangeFileExtension),
			def("GetRelativePath", & ParaIO::GetRelativePath),
			def("GetFileExtension", & ParaIO::GetFileExtension),
			def("GetAbsolutePath", & ParaIO::GetAbsolutePath),
			def("GetFileName", & ParaIO::GetFileName),
			def("GetFileOriginalName", & ParaIO::GetFileOriginalName),
			def("ToCanonicalFilePath", & ParaIO::ToCanonicalFilePath),
			def("UpdateMirrorFiles", & ParaIO::UpdateMirrorFiles),
			def("CRC32", & ParaIO::CRC32),
			def("GetFileSize", & ParaIO::GetFileSize),
			def("GetFileInfo", &ParaIO::GetFileInfo),
			def("AddSearchPath", & ParaIO::AddSearchPath),
			def("AddSearchPath2", & ParaIO::AddSearchPath2),
			def("RemoveSearchPath", & ParaIO::RemoveSearchPath),
			def("ClearAllSearchPath", & ParaIO::ClearAllSearchPath),
			def("GetWritablePath", &ParaIO::GetWritablePath),
			def("DecodePath", & ParaIO::DecodePath),
			def("EncodePath", & ParaIO::EncodePath),
			def("EncodePath", & ParaIO::EncodePath2),
			def("AddPathVariable", & ParaIO::AddPathVariable),
			def("CloseFile", & ParaIO::CloseFile),
			def("GetFileSystemWatcher", & ParaIO::GetFileSystemWatcher),
			def("DeleteFileSystemWatcher", & ParaIO::DeleteFileSystemWatcher)
		]
	];

	lua_func_entry ParaIO_api_entries[] = {
		{ "SetCurrentFile", ParaIO::CBind_SetCurrentFile},
		{ "seek", ParaIO::CBind_seek },
		{ "ReadUInt", ParaIO::CBind_ReadUInt },
		{ "ReadNumbers", ParaIO::CBind_ReadNumbers },
		{ "ReadNumber", ParaIO::CBind_ReadNumber },
		{ 0, 0 }
	};
	lua_register_on_table(L, "ParaIO", ParaIO_api_entries);
}
}//namespace ParaScripting
