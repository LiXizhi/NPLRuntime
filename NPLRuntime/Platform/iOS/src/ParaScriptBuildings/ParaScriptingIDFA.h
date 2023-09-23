//-----------------------------------------------------------------------------
// Class: ParaScriptingIDFA.h
// Authors: big
// CreateDate: 2023.9.5
//-----------------------------------------------------------------------------

#include "ParaEngine.h"

namespace ParaScripting
{
    class PE_CORE_DECL ParaScriptingIDFA
	{
    public:
        static void requestTrackingAuthorizationAndFetchIDEA(std::function<void(std::string)> callback);
        static std::string Get();
        static void RequestTrackingAuthorization();
    };
}
