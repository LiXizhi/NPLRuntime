// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudioMemory.h"
#include "cStandardMemoryProvider.h"

namespace cAudio
{
	CAUDIO_API IMemoryProvider* getMemoryProvider()
	{
//To use your own memory provider, add it in here and set its name to memoryProvider
#if CAUDIO_MEMORY_USE_STD == 1
		static cStandardMemoryProvider memoryProvider;
		return &memoryProvider;
#else
#		error "No custom memory provider enabled here!"
#endif
	}
};