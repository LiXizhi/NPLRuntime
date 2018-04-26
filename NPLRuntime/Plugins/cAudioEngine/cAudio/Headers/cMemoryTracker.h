// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"

#if CAUDIO_USE_MEMORYTRACKER == 1

#include <map>
#include "cMutex.h"
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

#if CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS == 1
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

#if CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS == 1
		std::ofstream outMemLog;
#endif
	};
};

#endif