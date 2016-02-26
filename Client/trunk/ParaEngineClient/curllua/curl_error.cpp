//-----------------------------------------------------------------------------
// Class:	CCurlError
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2008.11.25
// Desc: this file is added to replace the luaL_error with a version that output to ParaEngine log and does not throw exceptions. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "curl_error.h"

#ifndef MAX_DEBUG_STRING_LENGTH
#define MAX_DEBUG_STRING_LENGTH 1024
#endif
static char buf_[MAX_DEBUG_STRING_LENGTH*2+2];

int luaL_error_nobreak( lua_State *L, const char *zFormat, ... )
{
	va_list args;
	va_start(args, zFormat);
	vsnprintf(buf_, MAX_DEBUG_STRING_LENGTH, zFormat, args);
	va_end(args);
	ParaEngine::CLogger::GetSingleton().AddLogStr(buf_);

	/*va_list args;
	va_start(args, zFormat);
	ParaEngine::CLogger::GetSingleton().WriteFormated(zFormat, args);
	va_end(args);*/
	return 0;
}