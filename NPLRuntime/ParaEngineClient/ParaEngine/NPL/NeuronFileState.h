#pragma once

#include "NPLMessageQueue.h"
#include "NPLCommon.h"

namespace NPL
{
	using namespace std;
	class CNPLRuntimeState;

	/** each neuron file that is activated will have a CNeuronFileState structure kept. 
	* This class is only used by the local NPL thread, so no lock is needed.
	*/
	class CNeuronFileState
	{
	public:
		CNeuronFileState(const std::string& filename);

		/** whether the message is still being processed in the activation function.
		some activation will set this flag to true in case their processing is cross multiple NPL time slice (tick).*/
		bool IsProcessing() const;
		void SetProcessing(bool val);

		/** @return true if succeed, or false if queue is full. */
		bool PushMessage(NPLMessage_ptr& msg);

		/** Pop a message. 
		* @return true if succeed or false if queue is empty.
		*/
		bool PopMessage(NPLMessage_ptr& msg);

		/** get the message without removing it from the stack*/
		bool GetMessage(NPLMessage_ptr& msg);

		/** if message queue is empty. */
		bool IsEmpty();

		/** clear all messages */
		void ClearMessage();

		/* check if there is clear state and unset it. 
		* return true if there is a clear state */
		bool PopClearState();

		/** increase internal tick counter for activation. 
		* @param nFrameMoveId: parent NPL thread's frame move id
		*/
		void Tick(int nFrameMoveId);

		/** get tick count for current frame. 
		* @param nFrameMoveId: parent NPL thread's frame move id
		*/
		int GetTick(int nFrameMoveId);

		/** max number of messages in the pending message queue. Any new incoming messages will be dropped.
		* By default its size is same as parent NPL thread's message queue size
		*/
		int32 GetMaxQueueSize() const;
		void SetMaxQueueSize(int32 val);

		/** neuron file name */
		const std::string& GetFilename() const { return m_filename; }
		
		/** Preemptive multi-tasking is simulated by counting instructions. */
		bool IsPreemptive();
		/** if not 0, we will use coroutine and debug hook to simulate preemptive scheduling in user mode. */
		int32 GetPreemptiveInstructionCount() const;
		void SetPreemptiveInstructionCount(int32 val);

	protected:
		/* clear all messages and shrink message queue (releasing memory).*/
		void ClearMessageImp();
		friend class CNPLRuntimeState;
	protected:
		/** total number of activation calls in this/last tick. */
		int32 m_nActivationThisTick;
		/** different for each tick */
		int32 m_nFrameMoveId;
		/** total number of activations since program starts. */
		int32 m_nTotalActivations;
		/** if not 0, we will use coroutine and debug hook to simulate preemptive scheduling in user mode. */
		int32 m_nPreemptiveInstructionCount;
		/** action state */
		DWORD m_nActionState;
		/** whether the message is still being processed in the activation function.
		some activation will set this flag to true in case their processing is cross multiple NPL time slice (tick).*/
		bool m_isProcessing;
		/** max number of messages in the pending message queue. Any new incoming messages will be dropped. 
		By default its size is same as parent NPL thread's message queue size 
		*/
		int32 m_nMaxQueueSize;
		/** the pending message queue */
		std::deque<NPLMessage_ptr> m_queue;
		/** neuron file name */
		std::string m_filename;
	};
}