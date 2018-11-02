#pragma once

#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#include <tchar.h>
#include <string>
#include <queue>

// ParaEngine includes
#include "PluginAPI.h"
#include "PluginLoader.hpp"

// ParaEngine interfaces that we use
#include "INPL.h"
#include "INPLRuntime.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"

#include "NPLInterface.hpp"

#include "util/Mutex.hpp"
#include "util/Semaphore.hpp"

namespace NPL
{
	/** auto updater interface. */
	class IAutoUpdaterClient
	{
	public:
		/** auto update progress */
		enum AutoUpdaterProgress
		{
			AUP_NOT_STARTED,
			AUP_STARTED,
			AUP_NOCHANGE,
			AUP_UNKNOWN,
			AUP_BROKENFILE,
			AUP_ERROR,
			AUP_PROGRESS,
			// apply patch completed
			AUP_COMPLETED,
			// info host to apply patch. 
			AUP_DO_APPLY_PATCH,
		};
		/** progress message*/
		struct ProgressMessage
		{
			ProgressMessage():m_eventType(AUP_NOT_STARTED),m_finishcount(0), m_allcount(0) {};
			ProgressMessage(AutoUpdaterProgress eventType, const char* msg=NULL, int finishcount=0, int allcount=0)
				:m_eventType(eventType),m_finishcount(finishcount), m_allcount(allcount) 
			{
				if(msg)
					m_msg = msg;
			};
			AutoUpdaterProgress m_eventType;
			std::string m_msg;
			int m_finishcount;
			int m_allcount;
		};

		/** progress call back. */
		class IAutoUpdaterCallback
		{
		public:
			virtual void OnProgress(const ProgressMessage& msg) = 0;
		};

		/** set the autoupdater.dll file path 
		* @param sDllPath: if not set, it will default to "AutoUpdater.dll" in the current working directory. 
		*/
		virtual void SetUpdaterDllPath(const char* sDllPath = NULL) = 0;

		/**
		* This function is called for update progress. 
		*/
		virtual void OnProgress(AutoUpdaterProgress eventType, const char* msg=NULL, int finishcount=0, int allcount=0) = 0;

		/** start updating the client in the current working directory. 
		* @param curVersion: 0 is forcing full update, -1 is using the current version in the current working directory. or a version number like "0.2.70"
		* @param sUpdaterName: we will keep temporary download file under update/[sUpdaterName]/[version] directory. if this is NULL, we will use the default name "web"
		* @return: S_OK if succeed, or E_FAIL. 
		*/
		virtual int BeginUpdate(const std::string& curVersion, const char* sUpdaterName = NULL) = 0;

		/** call the auto updater. */
		virtual void ActivateAutoUpdater(const std::string& sMsg) = 0;

		/** this function is called whenever we receive a message from autoupdater.dll */
		virtual void OnMessageCallback(NPLInterface::NPLObjectProxy& msg) = 0;

		/** this function returns until auto updater is finished. 
		* One can use GetMessage() and PeekMessage() instead of this function. 
		* @param pCallback: the caller can implement the interface. The callback will be called in the main thread. So there is no threading issues. 
		* return the last progress message. 
		*/
		virtual AutoUpdaterProgress WaitForCompletion(IAutoUpdaterCallback* pCallback = 0) = 0;

		/** Get a progress message from the queue. This function will block until a message arrive, much like the Win32 API. */
		virtual bool GetUpdaterMessage(ProgressMessage& msg) = 0;

		/** Peek a progress message from the queue. This function will return false if there is no message in the queue, much like the Win32 API. */
		virtual bool PeekUpdaterMessage(ProgressMessage& msg) = 0;

		/** join all threads and unload dll. This function is automatically called when the interface is cleaned up. */
		virtual void ExitUpdater() = 0;

		/** apply the recently downloaded patch */
		virtual void ApplyPatch() = 0;
	};

	/** CAutoUpdaterClietn also implements a dummy INPL Runtime state, so that it can receive call back from the autoupdater.dll activation. 
	*/
	class CAutoUpdaterClient : public INPLRuntimeState, public IAutoUpdaterClient
	{
	public:
		CAutoUpdaterClient();
		~CAutoUpdaterClient();

	public:
		/** set the autoupdater.dll file path 
		* @param sDllPath: if not set, it will default to "AutoUpdater.dll" in the current working directory. 
		*/
		virtual void SetUpdaterDllPath(const char* sDllPath = NULL);

		/** call the auto updater. */
		virtual void ActivateAutoUpdater(const std::string& sMsg);

		/** begin updating the application. 
		* @return: S_OK, update succeed, E_FAIL autoupdater.dll is not loaded. 
		*/
		virtual int BeginUpdate(const std::string& curVersion, const char* sUpdaterName = NULL);

		/** on progress */
		virtual void OnProgress(AutoUpdaterProgress eventType, const char* msg=NULL, int finishcount=0, int allcount=0);

		/** this function is called whenever we receive a message. */
		virtual void OnMessageCallback(NPLInterface::NPLObjectProxy& msg);

		/** this function returns until auto updater is finished. */
		virtual AutoUpdaterProgress WaitForCompletion(IAutoUpdaterCallback* pCallback = 0);

		/** Get a progress message from the queue. This function will block until a message arrive, much like the Win32 API. */
		virtual bool GetUpdaterMessage(ProgressMessage& msg);

		/** Peek a progress message from the queue. This function will return false if there is no message in the queue, much like the Win32 API. */
		virtual bool PeekUpdaterMessage(ProgressMessage& msg);

		/** join all threads and unload dll. This function is automatically called when the interface is cleaned up. */
		virtual void ExitUpdater();

		/** apply the recently downloaded patch */
		virtual void ApplyPatch();
	public:
		/** return the name of this runtime state. if "", it is considered an anonymous name */
		virtual const std::string& GetName() const;

		/**
		* activate the specified file. It can either be local or remote file. 
		*/
		virtual int activate(const char * sNPLFilename, const char* sCode, int nCodeLength=0, int priority=2, int reliability=4);

		/**
		* activate the specified file in this runtime state. the file can be script or DLL. The function will just insert the message into the message queue and return immediately.
		* @param code: it is a chunk of code that should be executed in the destination neuron. 
		*			this code usually set the values of POL global variables.
		* @param nLength: the code length. if this is 0, length is determined from code, however, it must not exceed 4096 bytes. if it is specified. it can be any code length 
		* @param priority: bigger is higher. 0 is the default. if 1, it will be inserted to the front of the queue.  
		* @return: NPLReturnCode
		*/
		virtual NPLReturnCode Activate_async(const std::string & filepath, const char * code = NULL,int nLength=0, int priority=0);

		/** same as Activate_async, except that it is a short cut name. and may be used by external dlls to activate a file on this local state asynchrounously. */
		virtual NPLReturnCode ActivateLocal(const char* filepath, const char * code = NULL,int nLength=0, int priority=0);

		/** same as Activate_async. except that input are read from NPLMesage. 
		* e.g.
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return Activate_async(msg, priority);
		* @param msg: the caller is should only new but never delete the NPLMessage_ptr. And that the message must be created in the same thread, usually just before calling this function
		*/
		virtual NPLReturnCode Activate_async(NPLMessage_ptr& msg, int priority=0);

		/**
		* send a message to the current message queue. This function is rarely needed to call directly, use Activate_async instead. 
		* e.g. 
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return SendMessage(msg, priority);
		* @param msg: the message to send. Please note that when the function returns, the msg will be reset to null. 
		* @return may fail if message queue is full. 
		*/
		virtual NPLReturnCode SendMessage(NPLMessage_ptr& msg, int priority=0);

		/** get a pointer to the current message */
		virtual const char* GetCurrentMsg();

		/** get length of the current message */
		virtual int GetCurrentMsgLength();

		/** get the NPL runtime environment */
		virtual INPLRuntime* GetNPLRuntime();

		/** write a log message
		* @param text: the content of the log message. 
		* @param nTextLen: the log text length in byte. if 0, text length will be determined automatically. 
		* @param nLogType: if this is 0, it is a normal log message. if this is 1, we will print current time, and runtime state name with the log message. 
		*/
		virtual void WriteLog(const char* text, int nTextLen=0, int nLogType = 0);

		//////////////////////////////////////////////////////////////////////////
		//
		// Timer functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** creates a timer with the specified time-out value
		* [thread safe]
		* @param nIDEvent: Specifies a positive timer identifier. For nIDEvent<=0, they are reserved for internal uses.
		* If the NPL runtime already has a timer with the value nIDEvent, 
		* then the existing timer is replaced by the new timer. When SetTimer replaces a timer, the timer is reset. 
		* @param fElapse: Specifies the time-out value, in seconds. Please note that a timer will not be repeatedly activated if
		*		its timeout is shorter than the frame rate of the NPL simulation pipeline .
		* @param sNeuronFile: The NPL file to be activated when the time-out value elapses. For more information about the file name
		*  See NPL.activate(). 
		* @return: true if succeeds.An application can pass the value of the nIDEvent parameter to the NPL.KillTimer function to destroy the timer.
		*/
		virtual bool SetTimer(int nIDEvent, float fElapse, const char* sNeuronFile) {return false;};

		/**
		* Destroys the specified timer
		* [thread safe]
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @return : If the function succeeds, the return value is true
		*/
		virtual bool KillTimer(int nIDEvent) { return false;};

		/**
		* Changes the start time and the interval between method invocations for a timer, using 32-bit signed integers to measure time intervals. 
		* [thread safe]
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @param dueTime: The amount of time to delay before the invoking the callback method specified when the Timer was constructed, in milliseconds. Specify zero (0) to restart the timer immediately.
		*  however, the current implementation does not accept dueTime that is larger than MAX_TIMER_DUE_TIME	10000000, which is 10000 seconds. 
		* @param period:The time interval between invocations of the callback method specified when the Timer was constructed, in milliseconds. 
		* @return : If the function succeeds, the return value is true
		*/
		virtual bool ChangeTimer(int nIDEvent, int dueTime, int period) { return false;};

		/** set the update url. */
		virtual void SetUpdateUrl(const std::string& updateurl);
		virtual void RegisterFile(const char* sFilename, INPLActivationFile* pFileHandler = NULL);

		virtual void call(const char * sNPLFilename, const char* sCode, int nCodeLength = 0);
	protected:
		/** pointer to the current message. it is only valid during activation call. NULL will be returned */
		const char* m_current_msg;

		/** length of the current message. it is only valid during activation call.*/
		int m_current_msg_length;

		/** the name of this runtime state. if "", it is considered an anonymous name */
		std::string m_name;

		/** get update url*/
		std::string m_updateurl;

		/** the Auto updater plugin */
		ParaEngine::CPluginLoader m_auto_updater_plugin;


		AutoUpdaterProgress m_progress_status;
		ParaEngine::Mutex m_mutex;
		ParaEngine::Semaphore m_semaphore;

		/** all messages */
		std::queue<ProgressMessage> m_msgs;

		/** the directory where the downloaded core update files are cached. */
		std::string m_session_dir;
		/** apply patch message */
		std::string m_sApplyPatchMsg;
		/** autoupdater.dll file path. */
		std::string m_updater_plugin_path;

		IAutoUpdaterCallback* m_pUpdaterCallback;
	};
}
