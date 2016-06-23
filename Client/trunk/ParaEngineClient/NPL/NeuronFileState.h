#pragma once

#include "NPLMessageQueue.h"
#include "NPLCommon.h"

namespace NPL
{
	using namespace std;

	/** each neuron file that is activated will have a CNeuronFileState structure kept. 
	* This class is only used by the local NPL thread, so no lock is needed.
	*/
	class CNeuronFileState
	{
	public:
		CNeuronFileState();

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

		/** if message queue is empty. */
		bool IsEmpty();

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
	protected:
		/** total number of activation calls in this/last tick. */
		int32 m_nActivationThisTick;
		/** different for each tick */
		int32 m_nFrameMoveId;
		/** total number of activations since program starts. */
		int32 m_nTotalActivations;
		/** whether the message is still being processed in the activation function.
		some activation will set this flag to true in case their processing is cross multiple NPL time slice (tick).*/
		bool m_isProcessing;
		/** max number of messages in the pending message queue. Any new incoming messages will be dropped. 
		By default its size is same as parent NPL thread's message queue size 
		*/
		int32 m_nMaxQueueSize;
		/** the pending message queue */
		std::deque<NPLMessage_ptr> m_queue;
	};
}