#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Corporation, All Rights Reserved.
// Date:  2009.6
// Author: LiXizhi
// Description:	NPL Types
//-----------------------------------------------------------------------------

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

/**
* the NPL protocol versions
*/
#define NPL_VERSION_MAJOR	1
#define NPL_VERSION_MINOR	0

#ifndef ParaIntrusivePtr
#define ParaIntrusivePtr boost::intrusive_ptr
#endif

namespace NPL
{
	struct NPLAddress;
	class CNPLConnection;
	class CNPLConnectionManager;
	class CNPLDispatcher;
	struct NPLFileName;
	class CNPLNetServer;
	struct NPLMessage;
	class CNPLMessageQueue;
	struct NPLMsgHeader;
	struct NPLMsgIn;
	class NPLMsgIn_parser;
	class NPLMsgOut;
	class CNPLRuntime;
	struct NPLRuntimeAddress;
	class CNPLRuntimeState;
	class CNPLScriptingState;
	class NPLServerInfo;
	class INPLStimulationPipe;
	struct NPLTimer;

	typedef boost::shared_ptr<CNPLRuntimeState> NPLRuntimeState_ptr;
	typedef boost::shared_ptr<NPLServerInfo> NPLServerInfo_ptr;
	typedef ParaIntrusivePtr<NPLMessage> NPLMessage_ptr;
	typedef boost::shared_ptr<NPLRuntimeAddress> NPLRuntimeAddress_ptr;
	typedef boost::shared_ptr<NPLTimer> NPLTimer_ptr;

	//
	// common data structures and forward declarations. 
	//
	
	enum NPLReturnCode
	{
		NPL_OK = 0, 
		NPL_Error, 
		NPL_ConnectionNotEstablished,
		NPL_QueueIsFull,
		NPL_StreamError,
		NPL_RuntimeState_NotExist,
		NPL_FailedToLoadFile,
		NPL_RuntimeState_NotReady,
		NPL_FileAccessDenied,
		NPL_ConnectionEstablished,
		NPL_UnableToResolveName,
		NPL_ConnectionTimeout,
		NPL_ConnectionDisconnected,
		NPL_ConnectionAborted,
		NPL_Command,
	};

	/**
	* This describes the possible states of a NPLConnection object.
	* the larger the value, the later the stage. 
	*/
	enum NPLConnectionState
	{
		/** socket is in disconnected state. */
		ConnectionDisconnected,              
		/** socket is currently trying to establish a connection. */
		ConnectionConnecting,                
		/** socket is connected to the server but authentication is not (yet) done. */
		ConnectionConnected, 
		/** socket is connected and being authenticated */
		ConnectionAuthenticating, 
		/** socket is connected and authenticated */
		ConnectionAuthenticated, 
	};

	/// These enumerations are used to describe when packets are delivered.
	enum PacketPriority
	{
		SYSTEM_PRIORITY,   /// \internal Used by network layer to send above-high priority messages.
		HIGH_PRIORITY,   /// High priority messages are send before medium priority messages.
		MEDIUM_PRIORITY,   /// Medium priority messages are send before low priority messages.
		LOW_PRIORITY,   /// Low priority messages are only sent when no other messages are waiting.
		NUMBER_OF_PRIORITIES
	};

	/// These enumerations are used to describe how packets are delivered.
	/// \note  Note to self: I write this with 3 bits in the stream.  If I add more remember to change that
	enum PacketReliability
	{
		UNRELIABLE,   /// Same as regular UDP, except that it will also discard duplicate datagrams.  The network layer adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
		UNRELIABLE_SEQUENCED,  /// Regular UDP with a sequence counter.  Out of order messages will be discarded.  This adds an additional 13 bits on top what is used for UNRELIABLE.
		RELIABLE,   /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
		RELIABLE_ORDERED,   /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
		RELIABLE_SEQUENCED /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
	};
}
