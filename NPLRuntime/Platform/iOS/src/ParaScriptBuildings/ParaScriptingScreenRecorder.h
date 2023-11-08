//-----------------------------------------------------------------------------
// Class: ParaScriptingScreenRecorder.h
// Authors: big
// CreateDate: 2021.12.14
//-----------------------------------------------------------------------------

#include "ParaEngine.h"

namespace ParaScripting
{
    class PE_CORE_DECL ParaScriptingScreenRecorder
	{
    public:
        static void Start();
        static void Stop();
        static std::string Save();
        static void Play();
        static void RemoveVideo();
    };
}
