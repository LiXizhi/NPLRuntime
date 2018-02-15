//-----------------------------------------------------------------------------
// Class:	NPLMessage
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.23
// Desc:  NPLMessage uses boost::intrusive_ptr instead of boost::shared_ptr, 
// and there is a 15% boost on high load test, mainly because there are half heap allocations. 
// See NPLMessage_ptr
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "NPLMessage.h"

NPL::NPLMessage::NPLMessage()
:m_type(MSG_TYPE_FILE_ACTIVATION)
{

}

NPL::NPLMessage::~NPLMessage()
{

}