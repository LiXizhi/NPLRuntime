#ifndef CMEMORYTRACKER_H_INCLUDED
#define CMEMORYTRACKER_H_INCLUDED

#include "../include/cAudioDefines.h"

#ifdef CAUDIO_USE_MEMORYTRACKER

#include <map>
#include "../Headers/cMutex.h"
#include <iostream>
#include <fstream>

namespace cAudio
{
	//! Tracks all allocations that go through cAudio's memory system and keeps statistics on it
	class cMemoryTracker
	{
	public:
		cMemoryTracker();
		~cMemoryTracker();

		static cMemoryTracker* Instance()
		{
			static cMemoryTracker Singleton;
			return &Singleton;
		}

		//! Adds an allocation to be tracked
		void AddAllocation(void* pointer, size_t size, const char* filename, int line, const char* function);
		
		//! Removes a previously tracked allocation (for when it has been deallocated)
		void RemoveAllocation(void* pointer);

	private:
		cAudioMutex Mutex;

		struct cTrackedMemoryBlock
		{
			size_t size;
			const char* filename;
			int line;
			const char* function;
		};

		std::map<void*, cTrackedMemoryBlock> TrackedBlocks;

#ifdef CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS
		struct Statistics
		{
			Statistics() : AllocationHighWaterMark(0), CurrentAllocationBytes(0), 
						   TotalAllocationBytes(0), MaxNumAllocations(0),
						   CurrentNumAllocations(0), TotalNumAllocations(0)
			{ }
			size_t AllocationHighWaterMark;
			size_t CurrentAllocationBytes;
			size_t TotalAllocationBytes;

			size_t MaxNumAllocations;
			size_t CurrentNumAllocations;
			size_t TotalNumAllocations;
		} MemStats;
#endif

		void DumpLeaks();

#ifdef CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS
		std::ofstream outMemLog;
#endif
	};
};

#endif

#endif //! CMEMORYTRACKER_H_INCLUDED