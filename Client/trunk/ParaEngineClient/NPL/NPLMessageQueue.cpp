//-----------------------------------------------------------------------------
// Class:	NPLMessageQueue
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "NPLMessageQueue.h"

#define NPL_DEFAULT_MSG_QUEUE_SIZE		500

NPL::CNPLMessageQueue::CNPLMessageQueue()
	: concurrent_ptr_queue<NPLMessage_ptr>(NPL_DEFAULT_MSG_QUEUE_SIZE)
{
}

NPL::CNPLMessageQueue::CNPLMessageQueue( int capacity )
	: concurrent_ptr_queue<NPLMessage_ptr>(capacity)
{
}

NPL::CNPLMessageQueue::~CNPLMessageQueue()
{
}