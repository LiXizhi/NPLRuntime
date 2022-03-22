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
        static void Start();
        static void Stop();
        static void Save();
        static void Play();
    };
}
