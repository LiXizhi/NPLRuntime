#pragma once
#include "NPLMessageQueue.h"
#include "NPLScriptingState.h"
#include "NPLCommon.h"
#include "NeuronFileState.h"
#include "INPLRuntimeState.h"
#include "INPLScriptingState.h"
#include "IAttributeFields.h"
#include "util/mutex.h"
#include "util/unordered_array.hpp"
#include <set>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread.hpp>
#else
#include "util/CoroutineThread.h"
#endif
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>


#ifdef EMSCRIPTEN_SINGLE_THREAD
typedef CoroutineThread npl_thread;
#else
typedef boost::thread npl_thread;
#endif

namespace ParaEngine
{
	struct DLLPlugInEntity;
}
namespace NPL
{
	using namespace std;
	class INPLActivationFile;
	class CNeuronFileState;


	/**
	* A runtime state contains the scripting runtime stack and can be run in a single thread.
	*
	* An NPL runtime state is message driven, however we also support timer and several other event callbacks.
	*/
	class CNPLRuntimeState :
		public ParaEngine::IAttributeFields,
		public INPLRuntimeState,
		public ParaScripting::CNPLScriptingState,
		public boost::enable_shared_from_this<CNPLRuntimeState>,
		private boost::noncopyable
	{
	public:
		typedef std::map<int, NPLTimer_ptr> NPLTimer_Pool_Type;
		typedef boost::signals2::signal<void(CNPLRuntimeState* pRuntimeState)>  Signal_StateLoaded_t;

		/** a type must be provided. it defaults to NPL runtime state. */
		CNPLRuntimeState(const string & name, NPLRuntimeStateType type_ = NPLRuntimeStateType_NPL);
		virtual ~CNPLRuntimeState();

		ATTRIBUTE_DEFINE_CLASS(CNPLRuntimeState);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(ParaEngine::CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetProcessedMsgCount_s, int*) { *p1 = cls->GetProcessedMsgCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetCurrentQueueSize_s, int*) { *p1 = cls->GetCurrentQueueSize(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetTimerCount_s, int*) { *p1 = cls->GetTimerCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, SetMsgQueueSize_s, int) { cls->SetMsgQueueSize(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetMsgQueueSize_s, int*) { *p1 = cls->GetMsgQueueSize(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, HasDebugHook_s, bool*) { *p1 = cls->HasDebugHook(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, IsPreemptive_s, bool*) { *p1 = cls->IsPreemptive(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, PauseAllPreemptiveFunction_s, bool) { cls->PauseAllPreemptiveFunction(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, IsAllPreemptiveFunctionPaused_s, bool*) { *p1 = cls->IsAllPreemptiveFunctionPaused(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetFileName_s, const char**) { *p1 = cls->GetCurrentFileName(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, GetDebugTraceLevel_s, int*) { *p1 = cls->GetDebugTraceLevel(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntimeState, SetDebugTraceLevel_s, int) { cls->SetDebugTraceLevel(p1); return S_OK; }


		/** call this function before calling anything else. It will load all NPL modules into the runtime state. */
		void Init();

		/** reset NPL state. This function must not be called by the NPL state itself.
		all NPL and table memory will be released. This function is usually called in case of a soft ParaEngine restart.
		*/
		void Reset_Imp();

		/** reset NPL state. it will put MSG_TYPE_RESET to the input queue of this runtime process, and wait for the next cycle to call Reset_Imp().
		* all NPL and table memory will be released. This function is usually called in case of a soft ParaEngine restart.
		* @param onResetScode: the code to be executed immediately after runtime state is reset. default to NULL.
		*/
		void Reset(const char* onResetScode = NULL);


		/** this function will create a worker thread to run Run(), and return immediately.
		* This is the advised way to start the runtime state. */
		int Run_Async();

		/**
		* this function does not return until the run time state is about to be destroyed.
		* it wait on the m_semaphore object when input_queue has items, and process messages in input_queue.
		*/
		int Run();

		/** this function should be called whenever there are messages in the input_queue for processing.
		* one can call this function regularly using a timer(the main game thread does this),
		* or wait on the m_semaphore object when input_queue has items (almost all other secondary runtime states use this).
		* @return: if -1, we should exit the runtime state and never call this function again. normally it returns 0.
		*/
		int Process();

		/** return the name of this runtime state. if "", it is considered an anonymous name */
		virtual const string& GetName() const { return m_name; }

		/** stop the thread by sending the quit message to this runtime state. this function will only return when the thread is quit.
		so never call this function from the same thread, otherwise it is a deadlock.
		Usually this function is called from the main NPL runtime thread. */
		int Stop();

		/** same as Stop(), except that it just send the exist message, but does not wait for termination of the thread. */
		int Stop_Async();

		/** Get the current number of messages in the input message queue. Sometimes, a monitor or dispatcher logics may need to know the queue size of all NPL runtime states.
		* and a dispatcher will usually need to add new messages to the NPL state with smallest queue size.
		* This function will lock the message queue to retrieve the queue size, so do not call it very often, but use a timer to query it on some interval.
		* [thread safe]
		*/
		int GetCurrentQueueSize();

		/** the total number of message processed by this runtime state since start. If this number does not increase,
		* perhaps the processing is blocking somewhere, and we should take actions.
		* [Not thread safe] in most cases, it will return correct result even in multi-threaded environment, but since we do not use lock,
		* unexpected result may return. This function is usually used for stat printing and monitoring.
		*/
		int GetProcessedMsgCount();

		/** get the message queue size. default to 500. For busy server side, we can set this to something like 5000
		* [thread safe]
		*/
		int GetMsgQueueSize();

		/** set the message queue size. default to 500. For busy server side, we can set this to something like 5000
		* [thread safe]
		*/
		void SetMsgQueueSize(int nSize = 500);


		/** simply wait for the next message to arrive.
		* @param nMessageCount: if not negative, this function will immediately return when the message queue size is bigger than this value.
		*/
		void WaitForMessage(int nMessageCount = -1);

		/**
		* @return: get a pointer to the object at given index, if exist, or NULL.
		* @note this is thread safe, however the returned object may be invalid if it is popped by another thread when you use it.
		*/
		NPLMessage_ptr PeekMessage(int nIndex);

		/** pop message at given index. usually we need to call peek() first.
		* @return true if popped.
		*/
		NPLMessage_ptr PopMessageAt(int nIndex);

	public:
		virtual const std::string& GetIdentifier();

		/** activate any runtime local or remote file. This function is usually called by dll interface. */
		virtual int activate(const char * sNPLFilename, const char* sCode, int nCodeLength = 0, int priority = 2, int reliability = 4);

		/**
		* load a file without running it in this runtime state.
		* If the file is already loaded,it will not be loaded again.
		* If the file extension is ".dll", it will be treated as a plug-in. if the filepath is "*.dll", it means all DLLs in that directory.
		* IMPORTANT: unlike other activation functions, this is more like "include()", the function
		* will be loaded where it is and returned to the original caller upon finishing.
		* @param filePath: the local file path, it can be StringBuilder or std::string.
		* @param bReload: if true, the file will be reloaded even if it is already loaded.
		*    otherwise, the file will only be loaded if it is not loaded yet.
		* @param bNoReturn: generate no return on lua_state's stack.
		* @return: return true if file is loaded.
		*/
		template <typename StringType>
		bool LoadFile_any(const StringType & filepath, bool bReload = false, lua_State* L = 0, bool bNoReturn = false);
		/**
		* Activate a file(script or dll) in this runtime state. The file should be loaded already.
		* @param filepath: pointer to the file path. it can be StringBuilder or std::string.
		* @return: NPLReturnCode
		*/
		template <typename StringType>
		NPLReturnCode ActivateFile_any(const StringType& filepath, const char * code = NULL, int nLength = 0);

		/**
		* activate the specified file in this runtime state. the file can be script or DLL. The function will just insert the message into the message queue and return immediately.
		* @param code: it is a chunk of code that should be executed in the destination neuron.
		*			this code usually set the values of POL global variables.
		* @param nLength: the code length. if this is 0, length is determined from code, however, it must not exceed 4096 bytes. if it is specified. it can be any code length
		* @param priority: bigger is higher. 0 is the default. if 1, it will be inserted to the front of the queue.
		* @return: NPLReturnCode
		*/
		virtual NPLReturnCode Activate_async(const string & filepath, const char * code = NULL, int nLength = 0, int priority = 0);
		virtual NPLReturnCode Loadfile_async(const string & filepath, int priority = 0);

		/** same as Activate_async. except that input are read from NPLMesage.
		* e.g.
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return Activate_async(msg, priority);
		* @param msg: the caller is should only new but never delete the NPLMessage_ptr. And that the message must be created in the same thread, usually just before calling this function
		*/
		virtual NPLReturnCode Activate_async(NPLMessage_ptr& msg, int priority = 0);

		/** same as Activate_async, except that it is a short cut name. and may be used by external dlls to activate a file on this local state asynchrounously. */
		virtual NPLReturnCode ActivateLocal(const char* filepath, const char * code = NULL, int nLength = 0, int priority = 0);

		/**
		* send a message to the current message queue. This function is rarely needed to call directly, use Activate_async instead.
		* e.g.
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return SendMessage(msg, priority);
		* @param msg: the message to send. Please note that when the function returns, the msg will be reset to null.
		* @return may fail if message queue is full.
		*/
		virtual NPLReturnCode SendMessage(NPLMessage_ptr& msg, int priority = 0);


		/** get a pointer to the current message. This function is usually called by DLL plugin in the LibActivate function.  */
		virtual const char* GetCurrentMsg();

		/** get length of the current message. This function is usually called by DLL plugin in the LibActivate function.  */
		virtual int GetCurrentMsgLength();

		/** get the NPL runtime environment */
		virtual INPLRuntime* GetNPLRuntime();

		/** write a log message
		* @param text: the content of the log message.
		* @param nTextLen: the log text length in byte. if 0, text length will be determined automatically.
		* @param nLogType: if this is 0, it is a normal log message. if this is 1, we will print current time, and runtime state name with the log message.
		*/
		virtual void WriteLog(const char* text, int nTextLen = 0, int nLogType = 0);

		//////////////////////////////////////////////////////////////////////////
		//
		// Timer functions
		//
		//////////////////////////////////////////////////////////////////////////
		/** [thread safe]*/
		int GetTimerCount();

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
		virtual bool SetTimer(int nIDEvent, float fElapse, const char* sNeuronFile);

		/**
		* Destroys the specified timer
		* [thread safe]
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @return : If the function succeeds, the return value is true
		*/
		virtual bool KillTimer(int nIDEvent);

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
		virtual bool ChangeTimer(int nIDEvent, int dueTime, int period);

		/** get the INPLRuntimeState state interface. */
		virtual INPLRuntimeState* GetNPLStateInterface() { return this; }

		/** function to register the a file handler in the current NPL state, so that it is callable from NPL script or C++
		* @param sFilename: any name with cpp file extension can be used. usually it is "states.cpp". The name does not need to be same as the real cpp file.
		* @param pFileHandler: if NULL it will unregister. If not, it is the file handler pointer, the pointer must be always valid, it is usually a static singleton object.
		*/
		virtual void RegisterFile(const char* sFilename, INPLActivationFile* pFileHandler = NULL);

		/** synchronous function call */
		virtual void call(const char * sNPLFilename, const char* sCode, int nCodeLength = 0);;

		/** get string buffer by index. Internally it is an array of std::strings. */
		std::string& GetStringBuffer(int nIndex = 0);


		/** signal: called when state is first loaded */
		Signal_StateLoaded_t StateLoaded;

		/** function for processing a given msg
		* @return: if -1, we should exit the runtime state. normally it returns 0.
		*/
		int ProcessMsg(NPLMessage_ptr msg);

		/** any cross-frame pending messages are processed. */
		int SendTick();

		/** get neuron file state. */
		CNeuronFileState* GetNeuronFileState(const std::string& filename, bool bCreateIfNotExist = true);

		/** whether there is already a debug hook.
		Not thread-safe: can only be called from the current thread.
		*/
		bool HasDebugHook();

		/** whether we are currently running in a preemptive activation function.
		Not thread-safe: can only be called from the current thread.
		*/
		bool IsPreemptive();

		/** whether all preemptive functions are paused for debugging purposes. */
		bool IsAllPreemptiveFunctionPaused() const;
		void PauseAllPreemptiveFunction(bool val);

	protected:

		/** load all NPL related functions. This function must be called for all scripting based classes. */
		void LoadNPLState();

		/** this function is called often enough from the NPL runtime's main thread.
		* [thread safe]
		* @param nTickCount: it should be ::GetTickCount() in millisecond. if 0, we will call the system ::GetTickCount() to get the current tick count.
		* @return the number of active timers
		*/
		int TickTimers(DWORD nTickCount);

		int FrameMoveTick();

		/** construct this to ensure matching calls to SetCurrentMessage(). */
		class CCurrentMessage
		{
		public:
			CCurrentMessage(CNPLRuntimeState* pState, const char* msg, int nLength) :m_pState(pState) {
				if (m_pState)
					m_pState->SetCurrentMessage(msg, nLength);
			}
			~CCurrentMessage() {
				if (m_pState)
					m_pState->SetCurrentMessage(NULL, 0);
			}
			CNPLRuntimeState* m_pState;
		};
		/** set the current message that is being processed. */
		void SetCurrentMessage(const char* msg = NULL, int nLength = 0);

		/** get the mono scripting state. and create one from the NPLMono plugin, if one does not exist.*/
		IMonoScriptingState* GetMonoState();

		void SetPreemptive(bool val);

		void ClearNeuronFiles();
	private:
		typedef map<std::string, ParaEngine::DLLPlugInEntity*>	DLL_Plugin_Map_Type;
		typedef std::vector<NPLTimer_ptr> NPLTimer_TempPool_Type;
		/** loaded dll plugins: mapping from file name to references of ParaEngine::DLLPlugInEntity */
		DLL_Plugin_Map_Type m_dll_plugins_map;

		/// the name of this runtime state. if "", it is considered an anonymous name
		const string m_name;

		/** all active timers by the runtime*/
		NPLTimer_Pool_Type m_activeTimers;

		NPLTimer_TempPool_Type m_temp_timer_pool;

		/// the immutable state type
		const NPLRuntimeStateType m_type;

		/// the input message queue
		CNPLMessageQueue m_input_queue;

		/** Thread in which the NPL runtime state are executed */
		boost::shared_ptr<npl_thread> m_thread;

		/** provide thread safe access to shared data members in this class. */
		ParaEngine::mutex m_mutex;

		/** a counting semaphore used to inform this NPL runtime state's thread that more work are to be done. */
		dummy_condition_variable m_semaphore;

		/** whether we will use semaphore for message inform.*/
		bool m_bUseMessageEvent;

		/** whether we are processing message. this will affect the current queue size parameter. */
		bool m_bIsProcessing;

		/** whether inside a preemptive function. */
		bool m_bIsPreemptive;

		/** whether all preemptive functions are paused for debugging purposes. */
		bool m_bPauseAllPreemptiveFunction;

		/** pointer to the current message. it is only valid during activation call. NULL will be returned */
		const char* m_current_msg;

		/** length of the current message. it is only valid during activation call.*/
		int m_current_msg_length;

		/** the total number of message processed by this runtime state since start. If this number does not increase,
		* perhaps the processing is blocking somewhere, and we should take actions.
		*/
		int m_processed_msg_count;

		/** incremented every tick. */
		int m_nFrameMoveCount;
		/** Mono Scripting State. this is created via the NPLMono C++ plugin DLL. This class is created on demand.
		i.e. whenever the NPL Runtime state contains C# file. */
		IMonoScriptingState* m_pMonoScriptingState;

		/** this is for caching some per-state static buffers. */
		std::vector <std::string> m_string_buffers;

		/** c++ based activation files that is callable from scripting interface or NPL. */
		std::map<std::string, INPLActivationFile*> m_act_files_cpp;

		/** mapping from filename to neuron file state.
		* Not thread-safe: no lock is required, only used by local thread
		*/
		std::map<std::string, CNeuronFileState*> m_neuron_files;
		/** neuron files that have pending messages.
		* Not thread-safe: no lock is required, only used by local thread
		*/
		ParaEngine::unordered_array<CNeuronFileState*> m_active_neuron_files;

		friend class CNPLRuntime;
	};

	/** compare functions for runtime state ptr */
	struct NPLRuntimeState_PtrOps
	{
		bool operator()(const NPLRuntimeState_ptr & a, const NPLRuntimeState_ptr & b) const
		{
			return a.get() < b.get();
		}
	};
}