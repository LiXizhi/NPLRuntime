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
        static void Save();
        static void Play();
    };
}
