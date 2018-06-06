#ifndef CAUDIODEFINES_H
#define CAUDIODEFINES_H

#include "cAudioPlatform.h"

//! Global define for the version of cAudio.
//! This is used primarily by plugins to make sure they are linking to the right library.
#define CAUDIO_VERSION "2.1.0"

#if !defined( CAUDIO_DEBUG )
	#if defined( DEBUG ) || defined( _DEBUG )
		#define CAUDIO_DEBUG
	#endif
#endif

//////////////////////
// Windows Settings //
//////////////////////
#ifdef CAUDIO_PLATFORM_WIN

#ifndef CAUDIO_STATIC_LIB
	#define CAUDIO_API __declspec(dllexport)
#else
	#define CAUDIO_API extern "C"
#endif // CAUDIO_STATIC_LIB

//!Define for making the entire library Thread Safe, comment out to disable.  Will also disable internal threading by the library.
#define CAUDIO_MAKE_THREAD_SAFE

#ifdef CAUDIO_MAKE_THREAD_SAFE
	//! Define enables threading for the main update loop.  Disable to run the internal update loop yourself. NOTE: Internal threading should NOT be used if the library is not thread safe!
	#define CAUDIO_USE_INTERNAL_THREAD
#endif //CAUDIO_MAKE_THREAD_SAFE

//! This define controls whether the Ogg/Vorbis decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_OGG_DECODER
//! This define controls whether the RIFF/Wav decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_WAV_DECODER
//! This define controls whether the default filesystem data source is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_SOURCE
//! This define controls whether the default file logger (html) is compiled into the library
// #define CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER
//! This define controls whether the default console logger is compiled into the library
// #define CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER

//! This define controls whether plugin support is added into the library, commenting it out will prevent any plugins from working
#define CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
	//! Define enabled support for plugins as dynamic libs
	// #define CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
#endif //CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

//! Size of the internal buffer per source for audio data (total amount buffered is CAUDIO_SOURCE_BUFFER_SIZE * CAUDIO_SOURCE_NUM_BUFFERS)
#define CAUDIO_SOURCE_BUFFER_SIZE ( 1024 * 64 )
//! Number of internal buffers to cycle through per source (Note: using only 1 leads to choppy sound or premature ending of sources)
#define CAUDIO_SOURCE_NUM_BUFFERS 3

//! Comment out to remove all EFX support from the library
// #define CAUDIO_EFX_ENABLED

//! Max number of effects that can be attached to a single sound source
#define CAUDIO_SOURCE_MAX_EFFECT_SLOTS 4

//! Tells cAudio to use the C standard memory functions for allocations (memalloc and free)
#define CAUDIO_MEMORY_USE_STD

//! Tells cAudio to reroute memory allocations from stl containers into the defined memory provider for cAudio, otherwise the standard std::allocator is used.
#define CAUDIO_REROUTE_STL_ALLOCATIONS

//! Activates the internal memory tracker, which can be used to detect and locate memory leaks.
//#define CAUDIO_USE_MEMORYTRACKER

//! Tells the memory tracker to generate statistics on memory usage by cAudio
//#define CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS

//! Tells the memory tracker to log each and every allocation done by cAudio.  This can be very slow, use only if you are debugging an issue.
//#define CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS

#endif //CAUDIO_PLATFORM_WIN

///////////////////////
// Mac OS X Settings //
///////////////////////
#ifdef CAUDIO_PLATFORM_MAC

#define CAUDIO_API extern "C"

//!Define for making the entire library Thread Safe, comment out to disable.  Will also disable internal threading by the library.
#define CAUDIO_MAKE_THREAD_SAFE

#ifdef CAUDIO_MAKE_THREAD_SAFE
	//! Define enables threading for the main update loop.  Disable to run the internal update loop yourself. NOTE: Internal threading should NOT be used if the library is not thread safe!
	#define CAUDIO_USE_INTERNAL_THREAD
#endif //CAUDIO_MAKE_THREAD_SAFE

//! This define controls whether the Ogg/Vorbis decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_OGG_DECODER
//! This define controls whether the RIFF/Wav decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_WAV_DECODER
//! This define controls whether the default filesystem data source is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_SOURCE
//! This define controls whether the default file logger (html) is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER
//! This define controls whether the default console logger is compiled into the library
#define CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER

//! This define controls whether plugin support is added into the library, commenting it out will prevent any plugins from working
#define CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
	//! Define enabled support for plugins as dynamic libs
	#define CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
#endif //CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

//! Size of the internal buffer per source for audio data (total amount buffered is CAUDIO_SOURCE_BUFFER_SIZE * CAUDIO_SOURCE_NUM_BUFFERS)
#define CAUDIO_SOURCE_BUFFER_SIZE ( 1024 * 64 )
//! Number of internal buffers to cycle through per source (Note: using only 1 leads to choppy sound or premature ending of sources)
#define CAUDIO_SOURCE_NUM_BUFFERS 3

//! Max number of effects that can be attached to a single sound source
#define CAUDIO_SOURCE_MAX_EFFECT_SLOTS 4

//! Tells cAudio to use the C standard memory functions for allocations (memalloc and free)
#define CAUDIO_MEMORY_USE_STD

//! Tells cAudio to reroute memory allocations from stl containers into the defined memory provider for cAudio, otherwise the standard std::allocator is used.
//#define CAUDIO_REROUTE_STL_ALLOCATIONS

//! Activates the internal memory tracker, which can be used to detect and locate memory leaks.
//#define CAUDIO_USE_MEMORYTRACKER

//! Tells the memory tracker to generate statistics on memory usage by cAudio
//#define CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS

//! Tells the memory tracker to log each and every allocation done by cAudio.  This can be very slow, use only if you are debugging an issue.
//#define CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS

#endif //CAUDIO_PLATFORM_MAC

/////////////////////
// IPhone Settings //
/////////////////////
#ifdef CAUDIO_PLATFORM_IPHONE

#define CAUDIO_API extern "C"

//!Define for making the entire library Thread Safe, comment out to disable.  Will also disable internal threading by the library.
#define CAUDIO_MAKE_THREAD_SAFE

#ifdef CAUDIO_MAKE_THREAD_SAFE
	//! Define enables threading for the main update loop.  Disable to run the internal update loop yourself. NOTE: Internal threading should NOT be used if the library is not thread safe!
	#define CAUDIO_USE_INTERNAL_THREAD
#endif //CAUDIO_MAKE_THREAD_SAFE

//! This define controls whether the Ogg/Vorbis decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_OGG_DECODER
//! This define controls whether the RIFF/Wav decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_WAV_DECODER
//! This define controls whether the default filesystem data source is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_SOURCE
//! This define controls whether the default file logger (html) is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER
//! This define controls whether the default console logger is compiled into the library
#define CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER

//! Size of the internal buffer per source for audio data (total amount buffered is CAUDIO_SOURCE_BUFFER_SIZE * CAUDIO_SOURCE_NUM_BUFFERS)
#define CAUDIO_SOURCE_BUFFER_SIZE ( 1024 * 64 )
//! Number of internal buffers to cycle through per source (Note: using only 1 leads to choppy sound or premature ending of sources)
#define CAUDIO_SOURCE_NUM_BUFFERS 3

//! Tells cAudio to use the C standard memory functions for allocations (memalloc and free)
#define CAUDIO_MEMORY_USE_STD

//! Tells cAudio to reroute memory allocations from stl containers into the defined memory provider for cAudio, otherwise the standard std::allocator is used.
//#define CAUDIO_REROUTE_STL_ALLOCATIONS

//! Activates the internal memory tracker, which can be used to detect and locate memory leaks.
//#define CAUDIO_USE_MEMORYTRACKER

//! Tells the memory tracker to generate statistics on memory usage by cAudio
//#define CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS

//! Tells the memory tracker to log each and every allocation done by cAudio.  This can be very slow, use only if you are debugging an issue.
//#define CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS

#endif //CAUDIO_PLATFORM_IPHONE

////////////////////
// Linux Settings //
////////////////////
#ifdef CAUDIO_PLATFORM_LINUX

#define CAUDIO_API extern "C"

//!Define for making the entire library Thread Safe, comment out to disable.  Will also disable internal threading by the library.
#define CAUDIO_MAKE_THREAD_SAFE

#ifdef CAUDIO_MAKE_THREAD_SAFE
	//! Define enables threading for the main update loop.  Disable to run the internal update loop yourself. NOTE: Internal threading should NOT be used if the library is not thread safe!
	#define CAUDIO_USE_INTERNAL_THREAD
#endif //CAUDIO_MAKE_THREAD_SAFE

//! This define controls whether the Ogg/Vorbis decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_OGG_DECODER
//! This define controls whether the RIFF/Wav decoder is compiled into the library.
#define CAUDIO_COMPILE_WITH_WAV_DECODER
//! This define controls whether the default filesystem data source is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_SOURCE
//! This define controls whether the default file logger (html) is compiled into the library
#define CAUDIO_COMPILE_WITH_FILE_LOG_RECEIVER
//! This define controls whether the default console logger is compiled into the library
#define CAUDIO_COMPILE_WITH_CONSOLE_LOG_RECEIVER

//! This define controls whether plugin support is added into the library, commenting it out will prevent any plugins from working
#define CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
	//! Define enabled support for plugins as dynamic libs
	#define CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
#endif //CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

//! Size of the internal buffer per source for audio data (total amount buffered is CAUDIO_SOURCE_BUFFER_SIZE * CAUDIO_SOURCE_NUM_BUFFERS)
#define CAUDIO_SOURCE_BUFFER_SIZE ( 1024 * 64 )
//! Number of internal buffers to cycle through per source (Note: using only 1 leads to choppy sound or premature ending of sources)
#define CAUDIO_SOURCE_NUM_BUFFERS 3

//! Since linux has license issues with OpenAL EFX, we cannot release the header files necessary for it.  Therefore, we must disable EFX on linux.  Uncomment this define if you have EFX available on linux.
//#define CAUDIO_EFX_ENABLED

//! Max number of effects that can be attached to a single sound source
//#define CAUDIO_SOURCE_MAX_EFFECT_SLOTS 4

//! Tells cAudio to use the C standard memory functions for allocations (memalloc and free)
#define CAUDIO_MEMORY_USE_STD

//! Tells cAudio to reroute memory allocations from stl containers into the defined memory provider for cAudio, otherwise the standard std::allocator is used.
#define CAUDIO_REROUTE_STL_ALLOCATIONS

//! Activates the internal memory tracker, which can be used to detect and locate memory leaks.
//#define CAUDIO_USE_MEMORYTRACKER

//! Tells the memory tracker to generate statistics on memory usage by cAudio
//#define CAUDIO_MEMORYTRACKER_GENERATE_STATISTICS

//! Tells the memory tracker to log each and every allocation done by cAudio.  This can be very slow, use only if you are debugging an issue.
//#define CAUDIO_MEMORYTRACKER_LOG_ALL_ALLOCATIONS

#endif //CAUDIO_PLATFORM_LINUX

#endif //CAUDIODEFINES_H
