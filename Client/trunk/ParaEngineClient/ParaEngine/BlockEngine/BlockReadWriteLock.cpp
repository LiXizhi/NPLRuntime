//-----------------------------------------------------------------------------
// Class:	Block Manager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.27
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "os_calls.h"
#include "BlockReadWriteLock.h"

using namespace ParaEngine;

ParaEngine::BlockReadWriteLock::BlockReadWriteLock()
	:m_nActiveReaders(0), m_nWaitingWriters(0), m_activeWriterId(0), m_nWaitingReaders(0), m_writelock_recursive_depth(0), m_readlock_recursive_depth(0)
{

}

void ParaEngine::BlockReadWriteLock::BeginRead()
{
	std::unique_lock<std::mutex> Lock_(m_mutex);

	// check if we can read. 
	if (!StartReadFromNewReader()) 
	{
		// detect if BeginRead is called within BeginWrite
		if (m_activeWriterId != 0 && m_activeWriterId == GetCurrentThreadId())
		{
			--m_nWaitingReaders;
			++m_readlock_recursive_depth;
			return;
		}
			

		while (true) {
			// wait notification. 
			m_reader_signal.wait(Lock_);
			// check again if we can read
			// we will favor writer. when read must wait until all writers are released. 
			if (StartReadFromWaitingReader())
				return;
		}
	}
}

void ParaEngine::BlockReadWriteLock::EndRead()
{
	std::unique_lock<std::mutex> Lock_(m_mutex);
	if (m_readlock_recursive_depth > 0)
	{
		--m_readlock_recursive_depth;
		// this is a read lock inside write lock. 
		return;
	}

	if (--m_nActiveReaders == 0 && m_nWaitingWriters > 0)
	{
		Lock_.unlock();
		m_writer_signal.notify_one();
	}
}


size_t ParaEngine::BlockReadWriteLock::GetCurrentThreadId()
{
	size_t nWriterId = ParaEngine::GetThisThreadID();
	return nWriterId;
}

void ParaEngine::BlockReadWriteLock::BeginWrite(size_t nWriterId /*= 0*/)
{
	if (nWriterId == 0)
	{
		nWriterId = GetCurrentThreadId();
	}

	std::unique_lock<std::mutex> Lock_(m_mutex);

	if (m_activeWriterId == nWriterId)
	{
		PE_ASSERT(m_writelock_recursive_depth > 0);
		m_writelock_recursive_depth++;
		return;
	}
	// check write access     
	if (!StartWriteFromNewWriter(nWriterId))
	{
		while (true)
		{
			m_writer_signal.wait(Lock_);
			if (StartWriteFromWaitingWriter(nWriterId))
			{
				break;
			}
		}
	}
	m_writelock_recursive_depth = 1;
}

void ParaEngine::BlockReadWriteLock::EndWrite()
{
	std::unique_lock<std::mutex> Lock_(m_mutex);
	--m_writelock_recursive_depth;
	if (m_writelock_recursive_depth>0)
	{
		return;
	}

	m_activeWriterId = 0;
	if (m_nWaitingReaders > 0 && AllowReader())
	{
		Lock_.unlock();
		// notify all readers
		m_reader_signal.notify_all();
	}
	else if (m_nWaitingWriters > 0)
	{
		Lock_.unlock();
		// notify one writer
		m_writer_signal.notify_one();
	}
}

bool ParaEngine::BlockReadWriteLock::StartReadFromNewReader()
{
	bool pass = StartRead();
	if (!pass)
		++m_nWaitingReaders;
	return pass;
}

bool ParaEngine::BlockReadWriteLock::StartReadFromWaitingReader()
{
	bool pass = StartRead();
	if (pass)
		--m_nWaitingReaders;
	return pass;
}

bool ParaEngine::BlockReadWriteLock::StartRead()
{
	bool bAllowRead = AllowReader();
	if (bAllowRead)
		++m_nActiveReaders;
	return bAllowRead;
}

bool ParaEngine::BlockReadWriteLock::AllowReader()
{
	// only allow reader when there is no active writer or waiting writers. 
	return m_activeWriterId == 0 && m_nWaitingWriters == 0;
}

bool ParaEngine::BlockReadWriteLock::StartWriteFromNewWriter(size_t nWriterId)
{
	bool pass = StartWrite(nWriterId);
	if (!pass)
		++m_nWaitingWriters;
	return pass;
}

bool ParaEngine::BlockReadWriteLock::StartWrite(size_t nWriterId)
{
	bool allowWrite = (m_activeWriterId == 0 && m_nActiveReaders == 0);
	if (allowWrite)
	{
		m_activeWriterId = nWriterId;
	}
	return allowWrite;
}

bool ParaEngine::BlockReadWriteLock::StartWriteFromWaitingWriter(size_t nWriterId)
{
	bool pass = StartWrite(nWriterId);
	if (pass)
		--m_nWaitingWriters;
	return pass;
}

bool ParaEngine::BlockReadWriteLock::HasWaitingWriters()
{
	return m_nWaitingWriters != 0;
}

bool ParaEngine::BlockReadWriteLock::HasWriterLock()
{
	return m_activeWriterId != 0;
}

bool ParaEngine::BlockReadWriteLock::HasWaitingWritersAndSingleReader()
{
	return m_nWaitingWriters != 0 && m_nActiveReaders==1;
}

bool ParaEngine::BlockReadWriteLock::IsCurrentThreadHasWriterLock()
{
	return m_activeWriterId != 0 && m_activeWriterId == GetCurrentThreadId();
}

uint32 ParaEngine::BlockReadWriteLock::GetWritelockRecursiveDepth() const
{
	return m_writelock_recursive_depth;
}

