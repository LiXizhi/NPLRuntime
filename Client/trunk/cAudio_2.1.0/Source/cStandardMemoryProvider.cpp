#include "../Headers/cStandardMemoryProvider.h"
#include "../Headers/cMemoryTracker.h"
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
#ifdef CAUDIO_USE_MEMORYTRACKER
		cMemoryTracker::Instance()->AddAllocation(ptr, size, filename, line, function);
#endif
		return ptr;
	}

	void cStandardMemoryProvider::Free(void* pointer)
	{
		if(pointer)
		{
#ifdef CAUDIO_USE_MEMORYTRACKER
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