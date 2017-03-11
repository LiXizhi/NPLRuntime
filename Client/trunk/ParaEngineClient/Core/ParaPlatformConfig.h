#pragma once

// define supported target platform macro which CC uses.
#define PARA_PLATFORM_UNKNOWN            0
#define PARA_PLATFORM_IOS                1
#define PARA_PLATFORM_ANDROID            2
#define PARA_PLATFORM_WIN32              3
#define PARA_PLATFORM_MARMALADE          4
#define PARA_PLATFORM_LINUX              5
#define PARA_PLATFORM_BADA               6
#define PARA_PLATFORM_BLACKBERRY         7
#define PARA_PLATFORM_MAC                8
#define PARA_PLATFORM_NACL               9
#define PARA_PLATFORM_EMSCRIPTEN        10
#define PARA_PLATFORM_TIZEN             11
#define PARA_PLATFORM_QT5               12
#define PARA_PLATFORM_WP8               13
#define PARA_PLATFORM_WINRT             14

// Determine target platform by compile environment macro.
#define PARA_TARGET_PLATFORM             PARA_PLATFORM_UNKNOWN

// mac
#if defined(PLATFORM_MAC)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_MAC
#endif

// iphone
#if defined(PLATFORM_IPHONE)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_IOS
#endif

// android
#if defined(ANDROID)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_ANDROID
#endif

// win32
#if defined(WIN32)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_WIN32
#endif

// Linux
#if defined(LINUX) || defined(__linux__)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_LINUX
#endif

// marmalade
#if defined(MARMALADE)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM         PARA_PLATFORM_MARMALADE
#endif

// WinRT (Windows Store App)
#if defined(WINRT)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM			PARA_PLATFORM_WINRT
#endif

// WP8 (Windows Phone 8 App)
#if defined(WP8)
#undef  PARA_TARGET_PLATFORM
#define PARA_TARGET_PLATFORM			PARA_PLATFORM_WP8
#endif


// check user set platform
#if ! PARA_TARGET_PLATFORM
#error  "Cannot recognize the target platform; are you targeting an unsupported platform?"
#endif

// choose renderer
#if defined(PARAENGINE_MOBILE)
#define USE_OPENGL_RENDERER
#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_WIN32) && defined(PARAENGINE_CLIENT)
#define USE_DIRECTX_RENDERER


#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC) && defined(MAC_CLIENT)

#define USE_OPENGL_RENDERER
#else
#define USE_NULL_RENDERER
#endif

#ifndef WIN32
/** this will possibly fix the "file size limit exceeded" (signal 25) on linux ext3
* plus: ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64) should be defined on linux cmake config.
*/
#define _FILE_OFFSET_BITS 64 /* Linux, Solaris and HP-UX */
#define _LARGE_FILES 1 /* AIX */
#define _LARGEFILE64_SOURCE
#endif

#if defined(WIN32) && !defined(_WIN64) && !defined(PARAENGINE_MOBILE) && defined(PARAENGINE_CLIENT)  && !defined(NPLRUNTIME)
/** whether we should support build-in adobe flash player dll via activeX control. */
#define USE_FLASH_MANAGER
/** whether to compile with freeimage.dll support, mostly for loading PNG files */
#define USE_FREEIMAGE
#endif
