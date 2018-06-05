// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cStandardMemoryProvider.h"
#include "cMemoryTracker.h"
#include <stdlib.h>
#include <limits>

#ifdef max
	#undef max
#endif

#ifdef min
	#undef min
#endif

namespace cAudio
{
	void* cStandardMemoryProvider::Allocate(size_t size, const char* filename, int line, const char* function)
	{
		void* ptr = malloc(size);
#if CAUDIO_USE_MEMORYTRACKER == 1
		cMemoryTracker::Instance()->AddAllocation(ptr, size, filename, line, function);
#endif
		return ptr;
	}

	void cStandardMemoryProvider::Free(void* pointer)
	{
		if(pointer)
		{
#if CAUDIO_USE_MEMORYTRACKER == 1
			cMemoryTracker::Instance()->RemoveAllocation(pointer);
#endif
			free(pointer);
		}
	}

	size_t cStandardMemoryProvider::getMaxAllocationSize()
	{
		return std::numeric_limits<size_t>::max();
	}
};