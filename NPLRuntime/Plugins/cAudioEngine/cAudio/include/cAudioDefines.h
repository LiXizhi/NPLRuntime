// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioPlatform.h"

// CMake-generated build settings.
// %BUILD_DIR%/include to your include search path to find cAudioBuildSettings.h.
#include "cAudioBuildSettings.h"

//! Global define for the version of cAudio.
//! This is used primarily by plugins to make sure they are linking to the right library.
#define CAUDIO_VERSION "2.3.0"

#if !defined( CAUDIO_DEBUG )
	#if defined( DEBUG ) || defined( _DEBUG )
		#define CAUDIO_DEBUG
	#endif
#endif


#ifdef CAUDIO_PLATFORM_WIN
#	if CAUDIO_STATIC_LIB  == 0
#		define CAUDIO_API __declspec(dllexport)
#	else
#		define CAUDIO_API extern "C"
#	endif // CAUDIO_STATIC_LIB
#else
#   define CAUDIO_API
#endif

//! Define enables threading for the main update loop.  Disable to run the internal update loop yourself. NOTE: Internal threading should NOT be used if the library is not thread safe!
#if CAUDIO_MAKE_THREAD_SAFE == 1
	#define CAUDIO_USE_INTERNAL_THREAD
#endif 


// Fix LiXizhi: we are actually statically linked MP3 plugin, so iOS can enable CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
// #if !defined( CAUDIO_PLATFORM_IPHONE ) && CAUDIO_STATIC_LIB == 0
//! This define controls whether plugin support is added into the library, commenting it out will prevent any plugins from working
#	define CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
// #endif

//! Size of the internal buffer per source for audio data (total amount buffered is CAUDIO_SOURCE_BUFFER_SIZE * CAUDIO_SOURCE_NUM_BUFFERS)
#define CAUDIO_SOURCE_BUFFER_SIZE ( 1024 * 64 )
//! Number of internal buffers to cycle through per source (Note: using only 1 leads to choppy sound or premature ending of sources)
#define CAUDIO_SOURCE_NUM_BUFFERS 3

//! Max number of effects that can be attached to a single sound source
#define CAUDIO_SOURCE_MAX_EFFECT_SLOTS 4
