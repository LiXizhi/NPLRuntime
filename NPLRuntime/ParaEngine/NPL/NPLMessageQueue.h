#pragma once
#include "NPLMessage.h"

#include <boost/circular_buffer.hpp>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread.hpp>
#endif
#include "util/mutex.h"
#include <queue>

#ifdef EMSCRIPTEN_SINGLE_THREAD
typedef ParaEngine::mutex boost_mutex;
typedef ParaEngine::scoped_Lock<boost_mutex> boost_mutex_scoped_lock;
#else
typedef boost::mutex boost_mutex;
typedef boost::mutex::scoped_lock boost_mutex_scoped_lock;
#endif

namespace NPL
{
	/**
	* dummy condition variable
	*/
	class dummy_condition_variable
	{
	public:
		/** notify one dummy */
		void notify_one(){};

		template <typename T>
		void wait(T type_) {};
	};

	// forward declare
#ifdef EMSCRIPTEN_SINGLE_THREAD
	template <typename Data, typename Condition = dummy_condition_variable>
#else
	template <typename Data, typename Condition = boost::condition_variable>
#endif
	class concurrent_ptr_queue;

	/**
	* it implements a producer/consumer(s) queue design pattern. 
	* this is similar to concurrent_queue, except it will call reset() after push and pop operation. 
	* @note: typename Data must be shared_ptr or intrusive_ptr with reset() method, because we will call reset when 
	* push and pop operation returns. This allows non-thread-safe reference counted object to work as data items. 
	* for an example, please see NPLMessage_ptr. 
	*/
	template<typename Data, typename Condition>
	class concurrent_ptr_queue
	{
	protected:
		typedef boost::circular_buffer<Data> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::value_type value_type;

		mutable boost_mutex m_mutex;
		Condition m_condition_variable;

		/** whether to use event to inform consumer when new data items are added to the queue. Default to true.
		* if one uses polling,such as in the main game thread, there is no need to enable use_event. 
		* when enabled, there is a performance hit. 
		*/
		bool m_use_event;

		container_type m_container;
	public:
		enum BufferStatus
		{
			BufferOverFlow = 0,
			BufferFull = 1,
			BufferNormal = 2,
			BufferEmpty = 3,
			BufferFirst = 3,
		};

		explicit concurrent_ptr_queue(size_type capacity) : m_container(capacity),m_use_event(true) {}

		/** whether to use event to inform consumer when new data items are added to the queue. Default to true.
		* if one uses polling,such as in the main game thread, there is no need to enable use_event. 
		* when enabled, there is a performance hit. 
		*/
		void SetUseEvent(bool bUseEvent)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			m_use_event = bUseEvent;
		}

		/** try push to back of the queue.
		* @param item: the object to add. It is usually a reference counted object. item.reset() is called when the function is returned. so one should not use item any more after the function returns. 
		* @return: return buffer status after the item is added.  If BufferOverFlow, it means adding is failed; if BufferFirst, this is the first item added. 
		* if BufferFull, the buffer is full after inserting the new one, in other cases, BufferNormal is returned. 
		@note: thread safe
		*/
		BufferStatus try_push(value_type& item) 
		{
			boost_mutex_scoped_lock lock(m_mutex);
			BufferStatus bufferStatus = m_container.empty() ? BufferFirst : BufferNormal;
			if(m_container.full())
			{
				bufferStatus = BufferOverFlow;
			}
			else
			{
				m_container.push_back(item);
				bufferStatus = m_container.full() ? BufferFull : bufferStatus;
			}
			item.reset();
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
			return bufferStatus;
		}

		/**
		* same as try_push, except that it also returns pointer to the front object. 
		* @param item: the object to add. It is usually a reference counted object. item.reset() is called when the function is returned. so one should not use item any more after the function returns. 
		* @param ppFrontItem there is no guarantee that the front object pointer is valid after return. 
		* please ensure no other thread is popping items when you are accessing the front item or when the this function is called. 
		*/
		BufferStatus try_push_get_front(value_type& item, value_type** ppFrontItem) 
		{
			boost_mutex_scoped_lock lock(m_mutex);
			BufferStatus bufferStatus = m_container.empty() ? BufferFirst : BufferNormal;
			if(m_container.full())
			{
				bufferStatus = BufferOverFlow;
			}
			else
			{
				m_container.push_back(item);
				*ppFrontItem = &(m_container.front());
				bufferStatus = m_container.full() ? BufferFull : bufferStatus;
			}
			item.reset();
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
			return bufferStatus;
		}

		/** add a data item to the back of the queue. 
		* if buffer is full, the more recent messages remain in the queue. 
		* @param data: the object to add. It is usually a reference counted object. item.reset() is called when the function is returned. so one should not use item any more after the function returns. 
		*/
		void push(value_type& data)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			m_container.push_back(data);
			data.reset();
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
		}

		/** add a data item to the front of the queue. 
		This ensures that message is always added even queue is full, where the recent message are dropped. 
		This function is only used to insert high priority command, otherwise use push() function instead. 
		* @param data: the object to add. It is usually a reference counted object. item.reset() is called when the function is returned. so one should not use item any more after the function returns. 
		*/
		void push_front(value_type & data)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			m_container.push_front(data);
			data.reset();
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
		}

		bool try_pop(value_type& popped_value)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			if(m_container.empty())
			{
				return false;
			}

			popped_value=m_container.front();
			m_container.pop_front();
			return true;
		}

		void wait_and_pop(value_type& popped_value)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			while(m_container.empty())
			{
				m_condition_variable.wait(lock);
			}
			popped_value=m_container.front();
			m_container.pop_front();
		}

		/** simply wait for the next message to arrive. the caller may be calling try_XXX() afterwards.
		* @param nMessageCount: if not negative, this function will immediately return 
		* when the message queue size not equal to this value.
		*/
		void wait(int nMessageCount = -1)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			if (nMessageCount >= 0 && (nMessageCount != (int)m_container.size() || nMessageCount>= (int)m_container.capacity()))
				return;
			m_condition_variable.wait(lock);
		}

		/**
		* @return: get a pointer to the object at given index, if exist, or NULL.
		* @note this is thread safe, however the returned object may be invalid if it is popped by another thread when you use it.
		*/
		value_type peek(size_type nIndex)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			if (nIndex < m_container.size())
			{
				return m_container.at(nIndex);
			}
			return value_type();
		}

		/** pop message at given index. usually we need to call peek() first.
		* @return true if popped. 
		*/
		bool try_pop_at(size_type nIndex, value_type& popped_value)
		{
			boost_mutex_scoped_lock lock(m_mutex);
			if (nIndex < m_container.size())
			{
				if (nIndex == 0)
				{
					popped_value = m_container.front();
				}
				else
				{
					popped_value = m_container[nIndex];
					for (size_type i = nIndex; i >= 1; i--)
					{
						m_container[i] = m_container[i-1];
					}
				}
				m_container.pop_front();
				return true;
			}
			return false;
		}

		/**
		* @return: get a pointer to the front object if exist, or NULL. 
		* @note this is thread safe, however the returned object may be invalid if it is popped by another thread when you use it. 
		*/
		value_type * try_front()
		{
			boost_mutex_scoped_lock lock(m_mutex);
			return m_container.empty() ? NULL : &(m_container.front());
		}

		/** try pop from the front of the queue and return the front object after the pop.
		* @param ppValueFront: get a pointer to the front object after the operation. 
		*	Please note the returned object may be invalid if it is popped by another thread when you use it. 
		* @return: return true if succeed, false if queue is empty. 
		* @note: thread safe
		*/
		bool try_next(value_type** ppValueFront) 
		{
			boost_mutex_scoped_lock lock(m_mutex);
			if(m_container.empty())
			{
				return false;
			}
			else
			{
				m_container.pop_front();
				if(ppValueFront!=0 &&  !m_container.empty())
				{
					*ppValueFront = &(m_container.front());
				}
				return true;
			}
		}

		bool empty() const
		{
			boost_mutex_scoped_lock lock(m_mutex);
			return m_container.empty();
		}
		bool full() const {
			boost_mutex_scoped_lock lock(m_mutex);
			return m_container.full();
		};

		/** Get the number of elements currently stored in the circular_buffer */
		size_type size() const
		{
			boost_mutex_scoped_lock lock(m_mutex);
			return m_container.size();
		}

		/** Get the number of elements that can be stored in the circular_buffer */
		size_type capacity() const
		{
			return m_container.capacity();
		}

		/** Set the max number of elements that can be stored in the circular_buffer */
		void set_capacity(size_type new_capacity) 
		{
			boost_mutex_scoped_lock lock(m_mutex);
			m_container.set_capacity(new_capacity);
		}
	};

	/**
	* Message queue implementation. 
	*/
	class CNPLMessageQueue : public concurrent_ptr_queue<NPLMessage_ptr>
	{
	public:
		CNPLMessageQueue();
		CNPLMessageQueue(int capacity);
		~CNPLMessageQueue();
	public:

	};
}

