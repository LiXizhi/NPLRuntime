//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.11
// Description:	API for IO
//-----------------------------------------------------------------------------
#pragma once
#ifdef PARAENGINE_CLIENT
#include <boost/shared_ptr.hpp>
#include "InterprocessQueue.hpp"

namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;
	using namespace luabind;

	/** a interprocess message queue wrapper
	*/
	class ParaIPCQueue
	{
	public:
		ParaIPCQueue(){};
		/* ParaIPCQueue
		* @param	sQueueName	Name of the queue. 
		* @param	nUsage		The usage. 
		*/
		ParaIPCQueue(const char* sQueueName, int nUsage);
		ParaIPCQueue(boost::shared_ptr<CInterprocessQueue>& pQueue);

		bool IsValid();

		const char* GetName();

		void Cleanup();

		bool Remove();

		void Clear();

		/** blocking call to send a message. Send a message and block if queue is full until message is sent out. 
		* param msg: a table of  
		{
			method : string [optional] default to "NPL"
			to : string [required] the receiver IPC queue name
			from: string [optional] default the sender queue name. the sender IPC queue name
			type : number [optional] default to 0. 
			param1 : number [optional] default to 0. 
			param2 : number [optional] default to 0. 
			filename : string [optional] the file name 
			code : string or table [optional], if method is "NPL", code should be a pure table or nil.
			
			priority: number [optional] default to 0. Message priority
		} 
		*/
		int send(const object& msg);

		/** same as send(), except that it is not blocking call and will return 0 if messages are successfully sent. 
		only send if queue is not full. non-blocking. Internally it check available queue size and send via the blocking send() method. 
		*/
		int try_send(const object& msg);

		/** blocking call to force receive a message.
		* @param msg: a in|out table of
		{
			method : string
			from: the sender IPC queue name
			type : number 
			param1 : number
			param2 : number
			filename : the file name 
			code : table or nil
			priority: Message priority
		}
		* @return IPRC_OK(0) if a message is received. or IPRC_FAILED 
		*/
		int receive(const object& msg);

		/**  non-blocking call
		* @return IPRC_OK(0) if a message is received. or IPRC_QUEUE_IS_EMPTY is no complete message is read. or IPRC_FAILED 
		*/
		int try_receive(const object& msg);
	private:
		static bool ConvertObjectToMsg(const object& msg, InterProcessMessage& outMsg, int & nPriority);
		static bool ConvertMsgToObject(InterProcessMessage& inMsg, const object& msg, int nPriority);
	public:
		boost::shared_ptr<CInterprocessQueue> m_pQueue;
	};

	/** for interprocess message communication. */
	class ParaIPC
	{
	public:
		/** Create Get an IPC queue 
		* @param filename		Filename of the queue. 
		* @param	nCreationFlag	The creation flag. this should be 2 for IPQU_open_or_create
		* @return the queue. 
		*/
		static ParaIPCQueue CreateGetQueue(const char* filename, int nCreationFlag);

		/** clear all system watcher references that is created by GetQueue() */
		static void Clear();

		/** delete a watcher, it will no longer receive callbacks. 
		* @please note that if someone else still keeps a pointer to the directory watcher, it will not be deleted. 
		*/
		static void RemoveQueue(const char* name);
	};
}
#endif