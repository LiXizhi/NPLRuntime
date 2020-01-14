#include "../include/cAudioMemory.h"
#include "../Headers/cStandardMemoryProvider.h"

namespace cAudio
{
	CAUDIO_API IMemoryProvider* getMemoryProvider()
	{
#ifdef CAUDIO_MEMORY_USE_STD
		static cStandardMemoryProvider memoryProvider;
#endif
		//To use your own memory provider, add it in here and set its name to memoryProvider

		return &memoryProvider;
	}
};