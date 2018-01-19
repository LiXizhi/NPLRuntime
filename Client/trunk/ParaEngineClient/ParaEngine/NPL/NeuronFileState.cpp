//-----------------------------------------------------------------------------
// Class:	Neuron File State
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2016.6.24
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NeuronFileState.h"

using namespace NPL;
using namespace ParaEngine;

#define ACTIONSTATE_CLEARMESSAGE  0x1

NPL::CNeuronFileState::CNeuronFileState(const std::string& filename) : m_filename(filename), m_isProcessing(false), m_nMaxQueueSize(1000), m_nActivationThisTick(0), m_nFrameMoveId(0), m_nTotalActivations(0), m_nPreemptiveInstructionCount(0), m_nActionState(0)
{

}


bool NPL::CNeuronFileState::IsProcessing() const
{
	return m_isProcessing;
}

void NPL::CNeuronFileState::SetProcessing(bool val)
{
	m_isProcessing = val;
}

bool NPL::CNeuronFileState::PushMessage(NPLMessage_ptr& msg)
{
	if ((int)m_queue.size() < GetMaxQueueSize()) {
		m_queue.push_back(msg);
		return true;
	}
	return false;
}

bool NPL::CNeuronFileState::PopMessage(NPLMessage_ptr& msg)
{
	if (!m_queue.empty()) {
		msg = m_queue.front();
		m_queue.pop_front();
		return true;
	}
	return false;
}

bool NPL::CNeuronFileState::GetMessage(NPLMessage_ptr& msg)
{
	if (!m_queue.empty()) {
		msg = m_queue.front();
		return true;
	}
	return false;
}

bool NPL::CNeuronFileState::IsEmpty()
{
	return m_queue.empty();
}

void NPL::CNeuronFileState::Tick(int nFrameMoveId)
{
	if (m_nFrameMoveId == nFrameMoveId) 
	{
		m_nActivationThisTick++;
	}
	else
	{
		m_nFrameMoveId = nFrameMoveId;
		m_nActivationThisTick = 1;
	}
	m_nTotalActivations++;
}

int NPL::CNeuronFileState::GetTick(int nFrameMoveId)
{
	return (m_nFrameMoveId == nFrameMoveId) ? m_nActivationThisTick : 0;
}

int32 NPL::CNeuronFileState::GetMaxQueueSize() const
{
	return m_nMaxQueueSize;
}

void NPL::CNeuronFileState::SetMaxQueueSize(int32 val)
{
	m_nMaxQueueSize = val;
}

bool NPL::CNeuronFileState::IsPreemptive()
{
	return m_nPreemptiveInstructionCount > 0;
}

int32 NPL::CNeuronFileState::GetPreemptiveInstructionCount() const
{
	return m_nPreemptiveInstructionCount;
}

void NPL::CNeuronFileState::SetPreemptiveInstructionCount(int32 val)
{
	m_nPreemptiveInstructionCount = val;
}

void NPL::CNeuronFileState::ClearMessageImp()
{
	m_queue.clear();
	m_queue.shrink_to_fit();
}

void NPL::CNeuronFileState::ClearMessage()
{
	m_nActionState |= ACTIONSTATE_CLEARMESSAGE;
}

bool NPL::CNeuronFileState::PopClearState()
{
	if ((m_nActionState & ACTIONSTATE_CLEARMESSAGE) > 0)
	{
		m_nActionState &= (~((DWORD)ACTIONSTATE_CLEARMESSAGE));
		return true;
	}
	return false;
}
