#pragma once
#include "NPLTypes.h"
#include <string>

namespace NPL
{
	class INPLRuntime;
	class INPLActivationFile;
	/**
	* The types of CNPLRuntimeState
	*/
	enum NPLRuntimeStateType
	{
		/// the default NPL runtime state, with all NPL and ParaEngine functions loaded. it will consume about 1MB memory at start up. 
		NPLRuntimeStateType_NPL = 0,
		/// the light-weighter NPL runtime state, with only NPL and very limited functions loaded. 
		NPLRuntimeStateType_NPL_LITE,
		/// it consumes nothing. and is usually used with DLL plugins. 
		NPLRuntimeStateType_DLL,
		/// same as NPLRuntimeStateType_NPL, but with externally set lua state
		NPLRuntimeStateType_NPL_ExternalLuaState,
	};

	/**
	* INPLRuntimeState interface for DLL interface
	*/
	class INPLRuntimeState 
	{
	public:
		/** return the name of this runtime state. if "", it is considered an anonymous name */
		virtual const std::string& GetName() const = 0;
		
		/**
		* activate the specified file. It can either be local or remote file. 
		* 
		* @param pState: the source runtime state that initiated this activation. If pState is NULL, the main runtime state is used.
		* @param sNPLFileName: 
		* a globally unique name of a NPL file name instance. 
		* The string format of an NPL file name is like below. 
		* [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
		* 
		* the following is a list of all valid file name combinations: 
		*	"user001@paraengine.com:script/hello.lua"	-- a file of user001 in its default gaming thread
		*	"(world1)server001@paraengine.com:script/hello.lua"		-- a file of server001 in its thread world1
		*	"(worker1)script/hello.lua"			-- a local file in the thread worker1
		*	"(gl)script/hello.lua"			-- a glia (local) file in the current runtime state's thread
		*	"script/hello.lua"			-- a file in the current thread. For a single threaded application, this is usually enough.
		*	"(worker1)NPLRouter.dll"			-- activate a C++ or C# dll. Please note that, in windows, it looks for NPLRonter.dll; in linux, it looks for ./libNPLRouter.so 
		*	"plugin/libNPLRouter.dll"			-- almost same as above, it is recommented to remove the heading 'lib' when loading. In windows, it looks for plugin/NPLRonter.dll; in linux, it looks for ./plugin/libNPLRouter.so
		* 
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file. 
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0', 
		*	but, it must not exceed 4096 bytes. If length is explicitly specified, there is no such a limit. 
		* @param channel:On which channel to send the package. It can be a number in [0,15]. In case it is nil, the default channel (0) is used. 
		* @param priority: From high to low.If this is nil, medium priority(0) is used. 
		following enumerations are used to describe when packets are delivered.
		enum PacketPriority
		{
		SYSTEM_PRIORITY,   /// internally Used to send above-high priority messages.
		HIGH_PRIORITY,   /// High priority messages are send before medium priority messages.
		MEDIUM_PRIORITY,   /// Medium priority messages are send before low priority messages.
		LOW_PRIORITY,   /// Low priority messages are only sent when no other messages are waiting.
		};
		* @param reliability:From unreliable to reliable sequenced. 0 stands for unreliable. If this is nil, RELIABLE_ORDERED(3) is used. 
		following enumerations are used to describe how packets are delivered.
		enum PacketReliability
		{
		UNRELIABLE,   /// Same as regular UDP, except that it will also discard duplicate datagrams.  It adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
		UNRELIABLE_SEQUENCED,  /// Regular UDP with a sequence counter.  Out of order messages will be discarded.  This adds an additional 13 bits on top what is used for UNRELIABLE.
		RELIABLE,   /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
		RELIABLE_ORDERED,   /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
		RELIABLE_SEQUENCED /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
		};
		* @note: pure data table is defined as table consisting of only string, number and other table of the above type. 
		*   NPL.activate function also accepts ParaFileObject typed message data type. ParaFileObject will be converted to base64 string upon transmission. There are size limit though of 10MB.
		*   one can also programmatically check whether a script object is pure date by calling NPL.SerializeToSCode() function. Please note that data types that is not pure data in sCode will be ignored instead of reporting an error.
		* @return: NPLReturnCode. 0 means succeed. 
		*/
		virtual int activate(const char * sNPLFilename, const char* sCode, int nCodeLength=0, int priority=2, int reliability=4) = 0;

		/**
		* activate the specified file in this runtime state. the file can be script or DLL. The function will just insert the message into the message queue and return immediately.
		* @param code: it is a chunk of code that should be executed in the destination neuron. 
		*			this code usually set the values of POL global variables.
		* @param nLength: the code length. if this is 0, length is determined from code, however, it must not exceed 4096 bytes. if it is specified. it can be any code length 
		* @param priority: bigger is higher. 0 is the default. if 1, it will be inserted to the front of the queue.  
		* @return: NPLReturnCode
		*/
		virtual NPLReturnCode Activate_async(const std::string & filepath, const char * code = NULL,int nLength=0, int priority=0) = 0;

		/** same as Activate_async, except that it is a short cut name. and may be used by external dlls to activate a file on this local state asynchrounously. */
		virtual NPLReturnCode ActivateLocal(const char* filepath, const char * code = NULL,int nLength=0, int priority=0) = 0;

		/** same as Activate_async. except that input are read from NPLMesage. 
		* e.g.
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return Activate_async(msg, priority);
		* @param msg: the caller is should only new but never delete the NPLMessage_ptr. And that the message must be created in the same thread, usually just before calling this function
		*/
		virtual NPLReturnCode Activate_async(NPLMessage_ptr& msg, int priority=0) = 0;
		
		/**
		* send a message to the current message queue. This function is rarely needed to call directly, use Activate_async instead. 
		* e.g. 
		*	NPLMessage_ptr msg(new NPLMessage());
		*	return SendMessage(msg, priority);
		* @param msg: the message to send. Please note that when the function returns, the msg will be reset to null. 
		* @return may fail if message queue is full. 
		*/
		virtual NPLReturnCode SendMessage(NPLMessage_ptr& msg, int priority=0) = 0;

		/** get a pointer to the current message */
		virtual const char* GetCurrentMsg() = 0;

		/** get length of the current message */
		virtual int GetCurrentMsgLength() = 0;

		/** get the NPL runtime environment */
		virtual INPLRuntime* GetNPLRuntime() = 0;

		/** write a log message
		* @param text: the content of the log message. 
		* @param nTextLen: the log text length in byte. if 0, text length will be determined automatically. 
		* @param nLogType: if this is 0, it is a normal log message. if this is 1, we will print current time, and runtime state name with the log message. 
		*/
		virtual void WriteLog(const char* text, int nTextLen=0, int nLogType = 0) = 0;

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
		virtual bool SetTimer(int nIDEvent, float fElapse, const char* sNeuronFile) = 0;

		/**
		* Destroys the specified timer
		* [thread safe]
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @return : If the function succeeds, the return value is true
		*/
		virtual bool KillTimer(int nIDEvent) = 0;

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
		virtual bool ChangeTimer(int nIDEvent, int dueTime, int period) = 0;

		/** function to register the a file handler in the current NPL state, so that it is callable from NPL script or C++
		* @param sFilename: any name with cpp file extension can be used. usually it is "states.cpp". The name does not need to be same as the real cpp file.
		* @param pFileHandler: if NULL it will unregister. If not, it is the file handler pointer, the pointer must be always valid, it is usually a static singleton object.
		*/
		virtual void RegisterFile(const char* sFilename, INPLActivationFile* pFileHandler = NULL) = 0;

		/** synchronous function call */
		virtual void call(const char * sNPLFilename, const char* sCode, int nCodeLength = 0) = 0;
	};
}