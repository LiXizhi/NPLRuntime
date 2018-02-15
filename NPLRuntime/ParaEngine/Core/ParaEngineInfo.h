#pragma once

#include <string>
/** ParaEngine related information.*/
namespace ParaEngineInfo
{
	using namespace std;
	/** information about the ParaEngine. */
	class CParaEngineInfo
	{
	public:
		/** get the ParaEngine and product version*/
		static string GetVersion();
		static int GetParaEngineMajorVersion();
		static int GetParaEngineMinorVersion();
		static int GetProductMajorVersion();
		static int GetProductMinorVersion();

		static string GetWaterMarkText();
		static string GetPublicKey();
		static string GetAuthorizedTo();
		static string GetCopyright();
	};
	/** this function may be called at different places of the program. Which regularly performs multi-step authorization check.*/
	void AuthorizationCheck();
}
#ifdef _DEBUG
	#define AUTHORIZATION_CHECK()
#else
	#define AUTHORIZATION_CHECK() ParaEngineInfo::AuthorizationCheck();
#endif