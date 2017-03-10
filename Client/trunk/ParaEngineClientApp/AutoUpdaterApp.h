#pragma once
#include <string>
#include <list>
#include "AutoUpdaterClient.h"

namespace ParaEngine
{
	class CMyProcessAppThread;
	class CCommandLineParser;

	/** This is used to create an application using interprocess communication. */
	class CAutoUpdaterApp: public NPL::IAutoUpdaterClient::IAutoUpdaterCallback
	{
	public:
		typedef std::list <std::string> Domain_Map_t;

		CAutoUpdaterApp();
		~CAutoUpdaterApp();


		/** init command line so that we can use IPC calls before calling Run() method. 
		* @return 0 if command line is empty or not changed. otherise return 1. 
		*/
		virtual int SetCommandLine(const char* sCmdLine);

		/** set working directory 
		* @return: -1 if app_dir is not allowed. it must be in user's local dir. return 0 if succeed. 
		*/
		virtual int SetAppDir();

		/** begin core update. one can use TryGetNextCoreUpdateMessage() to process the update message. 
		* @param sCurVersion: "0" is forcing full update, "" is using the current version in the current working directory. or a version number like "0.2.70"
		* @param sSessionDir: we will keep temporary download file under update/[sUpdaterName]/[version] directory. if this is NULL, we will use the default name "web"
		*/
		virtual bool BeginCoreUpdate(const char* sCurVersion, const char* sSessionDir);

		/** sent interprocess message to the host process. */
		virtual bool SendHostMsg(int nMsg, DWORD param1 = 0, DWORD param2 = 0, const char* filename = NULL, const char* sCode = NULL, int nPriority = 0);

		/** send error message to host app.*/
		virtual void SendError(int nErrorCode, const char* errorMsg = NULL);

		/** send log message to host app.*/
		virtual void SendLog(const char* logMsg);

		/** whether the url is from a trusted web domain. */
		virtual bool IsUrlTrusted( const std::string& sUrl );

		/** auto updater call back. */
		virtual void OnProgress(const NPL::IAutoUpdaterClient::ProgressMessage& msg);

	protected:
		
		bool TryGetNextCoreUpdateMessage();

		NPL::CAutoUpdaterClient* CreateGetAutoUpdater();

		void ReleaseAutoUpdater();

		/** update the core game engine. */
		int DoCoreUpdate_imp();


		/** 
		* whether the input string match a given file pattern. Only wild card * is supported, where * can only match to AlphaNumeric character. 
		* e.g :
		*    assert(MatchFilePattern("http://update.paraengine.com/webplayer/redist1.exe", "http://*.paraengine.com/webplayer/") == 1)
		*    assert(MatchFilePattern("http://www.paraengine.com/webplayer/redist1.exe", "http://*.paraengine.com/webplayer/*.*") == 0)
		*    assert(MatchFilePattern("http://www.paraengine.com/webplayer/redist1.exe", "http://*.paraengine.com/*") == -1)
		* @return 0 if it is a perfect match. 1 if the beginning of input matches the sPattern perfectly. or -1 if something does not match. 
		*/
		static int MatchFilePattern( const std::string& input, const std::string& sPattern);
		static bool IsAlphaNumeric( char c);

	protected:
		std::string m_sCmdLine;
		CCommandLineParser* m_pCmdLineParams;
		bool m_bAutoUpdateComplete;
		NPL::CAutoUpdaterClient* m_pAutoUpdater;

		Domain_Map_t m_trusted_domains;
	};

}