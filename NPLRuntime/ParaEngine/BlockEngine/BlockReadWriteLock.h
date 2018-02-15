#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ParaEngine
{
	/** multiple shared readers and multiple exclusive writers
	* it favors writers over readers. All lock functions are recursive of its kind.
	* Within write lock, there can be read lock; but within read lock, there can NOT be write lock. 
	* Upgrade to write lock is not supported yet, but is possible. 
	* 
	* This is synchronization object between main thread, the light calculation and the block IO thread. 
	* This is a special read/write lock class tailored for the situation in block rendering. 
	* it favors writers over readers. 
	* I have referred to Doug Lea's WriterPreferenceReadWriteLock class 
	*/
	class BlockReadWriteLock
	{
	public:
		BlockReadWriteLock();

	public:
		/** recursive calls is possible and will be regarded as new readers at minimum cost. */
		void BeginRead();
		void EndRead();

		/* recursive calls with same id is supported. internally it uses a counter. 
		@param nWriterThreadId: usually thread id. if 0, it will be a hash value of the current thread id. */
		void BeginWrite(size_t nWriterThreadId = 0);
		void EndWrite();

		size_t  GetCurrentThreadId();
		/** this function just check writer count WITHOUT using a lock. 
		* it is fast to call, but never rely on the result. The reader can use this function as a hint of whether writer is hungry now, and release lock for it.  
		*/
		bool HasWaitingWriters();
		/* if one reader has priority over other readers, then it can call this function to decide whether to continue or not. */
		bool HasWaitingWritersAndSingleReader();

		/* check if a writer lock is being held by any of the thread. */
		bool HasWriterLock();

		/** whether the current thread is holding the writer lock. */
		bool IsCurrentThreadHasWriterLock();

		// we will allow recursive write lock on the same thread. 
		uint32 GetWritelockRecursiveDepth() const;
	protected:
		bool StartReadFromNewReader();
		bool StartReadFromWaitingReader();
		bool StartRead();
		bool AllowReader();
	protected:
		bool StartWriteFromNewWriter(size_t nWriterId);
		bool StartWrite(size_t nWriterId);
		bool StartWriteFromWaitingWriter(size_t nWriterId);
		
	protected:
		// current number of readers, must be 0 when writers can take control. 
		uint32 m_nActiveReaders;
		// not useful, only used for debugging purposes
		uint32 m_nWaitingReaders;
		// number of writers waiting. this variable in introduced, because we wants to favor writer over reader. 
		uint32 m_nWaitingWriters;
		// the current writer id: 0 means no writer. main thread has id==1, the light thread id is 2, IO thread id is 3. 
		size_t m_activeWriterId;
		
		// we will allow recursive write lock on the same thread. 
		uint32 m_writelock_recursive_depth;
		
		// only used with locks that is downgraded from write lock.
		uint32 m_readlock_recursive_depth;
		std::condition_variable m_reader_signal;
		std::condition_variable m_writer_signal;

		// represent this object
		std::mutex m_mutex;
	};

	/* this is the reverse of Scoped_Writelock. It allows a writer thread to temporary unlock. */
	template <typename Mutex = BlockReadWriteLock>
	class Scoped_WriterUnlock
	{
	public:
		bool IsValidLock() const { return m_bIsValidLock; }
		Scoped_WriterUnlock(Mutex& m)
			: mutex_(m), m_bIsValidLock(false), m_nWriterLockRecursiveDepth(0)
		{
			if (mutex_.HasWriterLock())
			{
				if (mutex_.IsCurrentThreadHasWriterLock())
				{
					m_bIsValidLock = true;
					m_nWriterLockRecursiveDepth = mutex_.GetWritelockRecursiveDepth();
					for (int i = 0; i < m_nWriterLockRecursiveDepth; ++i)
					{
						mutex_.EndWrite();
					}
				}
			}
		}
		~Scoped_WriterUnlock()
		{
			if (IsValidLock())
			{
				for (int i = 0; i < m_nWriterLockRecursiveDepth; ++i)
				{
					mutex_.BeginWrite();
				}
			}
		}
	protected:
		// The underlying mutex.
		Mutex& mutex_;
		bool m_bIsValidLock;
		int32 m_nWriterLockRecursiveDepth;
	};


	/** simple scoped read lock function */
	template <typename Mutex = BlockReadWriteLock>
	class Scoped_ReadLock
	{
	public:
		// Constructor acquires the lock.
		Scoped_ReadLock(Mutex& m)
			: mutex_(m)
		{
			mutex_.BeginRead();
			locked_ = true;
		}

		// Destructor releases the lock.
		~Scoped_ReadLock()
		{
			if (locked_)
				mutex_.EndRead();
		}

		// Explicitly acquire the lock.
		void lock()
		{
			if (!locked_)
			{
				mutex_.BeginRead();
				locked_ = true;
			}
		}

		// Explicitly release the lock.
		void unlock()
		{
			if (locked_)
			{
				mutex_.EndRead();
				locked_ = false;
			}
		}

		// Test whether the lock is held.
		bool locked() const
		{
			return locked_;
		}

		// Get the underlying mutex.
		Mutex& mutex()
		{
			return mutex_;
		}

	private:
		// The underlying mutex.
		Mutex& mutex_;

		// Whether the mutex is currently locked or unlocked.
		bool locked_;
	};

	/** simple scoped write lock function */
	template <typename Mutex = BlockReadWriteLock>
	class Scoped_WriteLock
	{
	public:
		// Constructor acquires the lock.
		Scoped_WriteLock(Mutex& m)
			: mutex_(m)
		{
			mutex_.BeginWrite();
			locked_ = true;
		}

		// Destructor releases the lock.
		~Scoped_WriteLock()
		{
			if (locked_)
				mutex_.EndWrite();
		}

		// Explicitly acquire the lock.
		void lock()
		{
			if (!locked_)
			{
				mutex_.BeginWrite();
				locked_ = true;
			}
		}

		// Explicitly release the lock.
		void unlock()
		{
			if (locked_)
			{
				mutex_.EndWrite();
				locked_ = false;
			}
		}

		// Test whether the lock is held.
		bool locked() const
		{
			return locked_;
		}

		// Get the underlying mutex.
		Mutex& mutex()
		{
			return mutex_;
		}

	private:
		// The underlying mutex.
		Mutex& mutex_;

		// Whether the mutex is currently locked or unlocked.
		bool locked_;
	};
}