// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cMemoryTracker.h"

#if CAUDIO_USE_MEMORYTRACKER == 1

namespace cAudio
{

cMemoryTracker::cMemoryTracker()
{
#if CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS == 1
	outMemLog.open("cAudioMemoryLog.log");
#endif
}

cMemoryTracker::~cMemoryTracker()
{
#if CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS == 1
	outMemLog.close();
#endif
	DumpLeaks();
}

void cMemoryTracker::AddAllocation(void* pointer, size_t size, const char* filename, int line, const char* function)
{
	cAudioMutexBasicLock lock(Mutex);
	cTrackedMemoryBlock block;
	block.size = size;
	block.filename = filename;
	block.line = line;
	block.function = function;
	TrackedBlocks[pointer] = block;

#if CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS == 1
	MemStats.CurrentAllocationBytes += size;
	MemStats.TotalAllocationBytes += size;

	MemStats.CurrentNumAllocations += 1;
	MemStats.TotalNumAllocations += 1;

	MemStats.AllocationHighWaterMark = MemStats.CurrentAllocationBytes > MemStats.AllocationHighWaterMark ? MemStats.CurrentAllocationBytes : MemStats.AllocationHighWaterMark;
	MemStats.MaxNumAllocations = MemStats.CurrentNumAllocations > MemStats.MaxNumAllocations ? MemStats.CurrentNumAllocations : MemStats.MaxNumAllocations;
#endif

#if CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS == 1
	if(outMemLog.good())
	{
		outMemLog << "Allocation at " << pointer << " with size " << size << " bytes in " << filename << " (Line: " << line << ") function: " << function << std::endl;
		outMemLog.flush();
	}
#endif
}

void cMemoryTracker::RemoveAllocation(void* pointer)
{
	cAudioMutexBasicLock lock(Mutex);
	std::map<void*, cTrackedMemoryBlock>::iterator it = TrackedBlocks.find(pointer);
	if(it != TrackedBlocks.end())
	{
#if CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS == 1
		if(outMemLog.good())
		{
			outMemLog << "Deallocation of address " << pointer << " with size " << it->second.size << " bytes in " << it->second.filename << " (Line: " << it->second.line << ") function: " << it->second.function << std::endl;
			outMemLog.flush();
		}
#endif

#if CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS == 1
		size_t size = it->second.size;
		MemStats.CurrentAllocationBytes -= size;
		MemStats.CurrentNumAllocations -= 1;
#endif
		TrackedBlocks.erase(pointer);
	}
}

void cMemoryTracker::DumpLeaks()
{
	cAudioMutexBasicLock lock(Mutex);

	std::ofstream leakFile("cAudioMemoryLeaks.log");

#if CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS == 1
	//Dump Statistics
	leakFile << "Highest Amount of Allocated Memory: " << MemStats.AllocationHighWaterMark << " bytes." << std::endl;
	leakFile << "Memory Allocated at Shutdown: " << MemStats.CurrentAllocationBytes << " bytes." << std::endl;
	leakFile << "Total Allocated Memory: " << MemStats.TotalAllocationBytes << " bytes." << std::endl;
	
	leakFile << "Highest Number of Allocated Objects: " << MemStats.MaxNumAllocations << std::endl;
	leakFile << "Objects Allocated at Shutdown: " << MemStats.CurrentNumAllocations << std::endl;
	leakFile << "Total Objects Allocated: " << MemStats.TotalNumAllocations << std::endl;
	leakFile << std::endl;
#endif

	std::map<void*, cTrackedMemoryBlock>::iterator it;
	for(it = TrackedBlocks.begin(); it != TrackedBlocks.end(); it++)
	{
		if(leakFile.good())
		{
			leakFile << "Address:" << it->first << " Size:" << it->second.size << " in " << it->second.filename << " (Line: " << it->second.line << ") function: " << it->second.function << std::endl;
			leakFile.flush();
		}
	}

	if(TrackedBlocks.size() == 0)
	{
		leakFile << "No Leaks Detected!" << std::endl;
		leakFile.flush();
	}

	leakFile.close();
}

};

#endif

