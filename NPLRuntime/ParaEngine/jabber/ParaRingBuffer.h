#pragma once
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>
#include "util/mutex.h"
#include <queue>


namespace ParaEngine
{
	/**
	* CParaRingBuffer is normally used in a producer-consumer mode when producer threads produce items and store them in the container and consumer threads remove these items and process them. 
	* The bounded buffer has to guarantee that producers do not insert items into the container when the container is full, 
	* that consumers do not try to remove items when the container is empty, and that each produced item is consumed by exactly one consumer. 
	* @note: most functions are thread-safe, a light mutex is used in place. 
	*/
	template <class T>
	class CParaRingBuffer
	{
	public:
		typedef boost::circular_buffer<T> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::value_type value_type;

		enum BufferStatus
		{
			BufferOverFlow = 0,
			BufferFull = 1,
			BufferNormal = 2,
			BufferEmpty = 3,
			BufferFirst = 3,
		};

		explicit CParaRingBuffer(size_type capacity) : m_container(capacity) {}

	public:
		/** try push to back of the queue.
		* @param item: the object to add
		* @return: return buffer status after the item is added.  If BufferOverFlow, it means adding is failed; if BufferFirst, this is the first item added. 
		* if BufferFull, the buffer is full after inserting the new one, in other cases, BufferNormal is returned. 
		@note: thread safe
		*/
		BufferStatus try_push(const value_type& item) 
		{
			Lock lock(m_mutex);
			if(m_container.full())
			{
				return BufferOverFlow;
			}
			else
			{
				BufferStatus bufferStatus =  m_container.empty() ? BufferFirst : BufferNormal;
				m_container.push_back(item);
				return m_container.full() ? BufferFull : bufferStatus;
			}
		}

		/**
		* same as try_push, except that it also returns pointer to the front object. 
		* @param ppFrontItem there is no guarantee that the  front object pointer is valid after return. 
		* please ensure no other thread is popping items when you are accessing the front item. 
		*/
		BufferStatus try_push_get_front(const value_type& item, value_type** ppFrontItem) 
		{
			Lock lock(m_mutex);
			if(m_container.full())
			{
				return BufferOverFlow;
			}
			else
			{
				BufferStatus bufferStatus =  m_container.empty() ? BufferFirst : BufferNormal;
				m_container.push_back(item);
				*ppFrontItem = &(m_container.front());
				return m_container.full() ? BufferFull : bufferStatus;
			}
		}

		/** push to the front of the queue. this is usually for high priority messages. If queue is full, the last one will be removed. 
		* @param item: the object to add
		* @note: thread safe
		*/
		void push_front(const value_type& item) 
		{
			Lock lock(m_mutex);
			m_container.push_front(item);
		}

		/** try push to back of the queue.
		* @return: return buffer status after the item is added.  If BufferOverFlow, it means adding is failed; if BufferFirst, this is the first item added. 
		* if BufferFull, the buffer is full after inserting the new one, in other cases, BufferNormal is returned. 
		* @note: thread safe
		*/
		BufferStatus try_push_array(const value_type* pItems, int nCount) 
		{
			Lock lock(m_mutex);
			if(m_container.capacity()<nCount)
			{
				return BufferFull;
			}
			else
			{
				BufferStatus bufferStatus =  m_container.empty() ? BufferFirst : BufferNormal;
				for (int i=0; i<nCount;++i)
				{
					m_container.push_back(*(pItems+i));
				}
				return m_container.full() ? BufferFull : bufferStatus;
			}
		}

		/**
		* @return: get a pointer to the front object if exist, or NULL. 
		* @note this is thread safe, however the returned object may be invalid if it is pop when you use it. 
		*/
		value_type * try_front()
		{
			Lock lock(m_mutex);
			return m_container.empty() ? NULL : &(m_container.front());
		}

		/** try pop from the front of the queue.
		* @param ppValueFront: get a pointer to the front object after the operation. 
		@return: return true if succeed, false if queue is empty. 
		@note: thread safe
		*/
		bool try_pop(value_type& item) 
		{
			Lock lock(m_mutex);
			if(m_container.empty())
			{
				return false;
			}
			else
			{
				item = m_container.front();
				m_container.pop_front();
				return true;
			}
		}

		/** try pop from the front of the queue and return the front object after the pop.
		* @param ppValueFront: get a pointer to the front object after the operation. 
		@return: return true if succeed, false if queue is empty. 
		@note: thread safe
		*/
		bool try_next(value_type** ppItem) 
		{
			Lock lock(m_mutex);
			if(m_container.empty())
			{
				return false;
			}
			else
			{
				m_container.pop_front();
				if(ppItem!=0 &&  !m_container.empty())
				{
					*ppItem  = &(m_container.front());
				}
				return true;
			}
		}

		size_type size() const {
			Lock lock(m_mutex);
			return m_container.size();
		};
		bool empty() const {
			Lock lock(m_mutex);
			return m_container.empty();
		};
		bool full() const {
			Lock lock(m_mutex);
			return m_container.full();
		};
	public:
		container_type m_container;
		mutex m_mutex;
	};

	/**
	* it implements a producer/consumer(s) queue design pattern. 
	* One or more producers push(data) to the queue, and multiple consumers wait_and_pop(data) from the queue. 
	* Internally a fixed-sized ring buffer is used. if buffer is full, older messages are dropped without notification. 
	* all functions are thread-safe. 
	*/
	template<typename Data>
	class concurrent_queue
	{
	protected:
		typedef boost::circular_buffer<Data> container_type;
		typedef typename container_type::size_type size_type;
		typedef typename container_type::value_type value_type;

		mutable boost::mutex m_mutex;
		boost::condition_variable m_condition_variable;

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

		explicit concurrent_queue(size_type capacity) : m_container(capacity),m_use_event(true) {}
		
		/** whether to use event to inform consumer when new data items are added to the queue. Default to true.
		* if one uses polling,such as in the main game thread, there is no need to enable use_event. 
		* when enabled, there is a performance hit. 
		*/
		void SetUseEvent(bool bUseEvent)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			m_use_event = bUseEvent;
		}

		/** try push to back of the queue.
		* @param item: the object to add
		* @return: return buffer status after the item is added.  If BufferOverFlow, it means adding is failed; if BufferFirst, this is the first item added. 
		* if BufferFull, the buffer is full after inserting the new one, in other cases, BufferNormal is returned. 
		@note: thread safe
		*/
		BufferStatus try_push(value_type& item) 
		{
			boost::mutex::scoped_lock lock(m_mutex);
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
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
			return bufferStatus;
		}

		/** add a data item to the back of the queue. 
		if buffer is full, the more recent messages remain in the queue. 
		*/
		void push(value_type& data)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			m_container.push_back(data);
			lock.unlock();
			if(m_use_event)
				m_condition_variable.notify_one();
		}

		/** add a data item to the front of the queue. 
		This ensures that message is always added even queue is full, where the recent message are dropped. 
		This function is only used to insert high priority command, otherwise use push() function instead. 
		*/
		void push_front(value_type & data)
		{
			boost::mutex::scoped_lock lock(m_mutex);
			m_container.push_front(data);
			lock.unlock();
			m_condition_variable.notify_one();
		}

		bool empty() const
		{
			boost::mutex::scoped_lock lock(m_mutex);
			return m_container.empty();
		}

		bool try_pop(value_type& popped_value)
		{
			boost::mutex::scoped_lock lock(m_mutex);
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
			boost::mutex::scoped_lock lock(m_mutex);
			while(m_container.empty())
			{
				m_condition_variable.wait(lock);
			}
			popped_value=m_container.front();
			m_container.pop_front();
		}
	};
}
