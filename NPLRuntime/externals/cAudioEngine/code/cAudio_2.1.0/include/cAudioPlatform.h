#ifndef CAUDIOPLATFORM_H_INCLUDED
#define CAUDIOPLATFORM_H_INCLUDED

#if !defined( CAUDIO_PLATFORM_WIN )
#	if defined( WIN32 ) || defined( _WINDOWS ) || defined(_WIN32)
#		define CAUDIO_PLATFORM_WIN
#	endif
#endif

#if !defined( CAUDIO_PLATFORM_MAC ) || !defined( CAUDIO_PLATFORM_IPHONE )
#   if defined( __APPLE__ ) || defined( __APPLE_CC__ ) || defined( MACOSX )
#		if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#			define CAUDIO_PLATFORM_IPHONE
#		else
#			define CAUDIO_PLATFORM_MAC
#		endif
#   endif
#endif

#if !defined( CAUDIO_PLATFORM_LINUX )
#	if defined(_UNIX) || defined(__linux)
#		define CAUDIO_PLATFORM_LINUX
#	endif
#endif

#ifndef CAUDIO_COMPILIER_GCC
#	if defined(__GNUC__) || defined(__GCCXML__)
#		define CAUDIO_COMPILIER_GCC
#	endif
#endif

#ifndef CAUDIO_COMPILIER_MSVC
#	if defined(_MSC_VER)
#		if _MSC_VER >= 1400
#			define CAUDIO_COMPILIER_MSVC
#		else
#			error This version of the MSVC compilier is not supported
#		endif
#	endif
#endif

#endif //! CAUDIOPLATFORM_H_INCLUDED