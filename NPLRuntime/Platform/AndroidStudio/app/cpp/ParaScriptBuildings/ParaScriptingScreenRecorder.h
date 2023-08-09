//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.h
// Authors: big
// CreateDate: 2022.2.25
//-----------------------------------------------------------------------------

#include "ParaEngine.h"

namespace ParaScripting
{
    class PE_CORE_DECL ParaScriptingScreenRecorder
	{
    public:
        static void LuabindRegisterScreenRecorderGlobalFunctions(lua_State *L);
        static void Start();
        static void Stop();
        static std::string Save();
        static void Play();
        static void RemoveVideo();
    };
}
