//-----------------------------------------------------------------------------
// Class: ParaScriptingAudioiOS.h
// Authors: big
// CreateDate: 2023.9.13
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptBindings/ParaScriptingAudio.h"

namespace ParaScripting
{
    class PE_CORE_DECL ParaScriptingAudioiOS
	{
    public:
        static bool StartRecording();
        static void StopRecording();
        static bool SaveRecording(const std::string &filepath, const float quality);
    };
}
