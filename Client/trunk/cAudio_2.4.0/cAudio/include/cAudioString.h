// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include "cAudioPlatform.h"
#include "cAudioMemory.h"
#include "cSTLAllocator.h"

#include <string>
#include <stdlib.h>

#ifdef CAUDIO_PLATFORM_WIN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#endif

namespace cAudio
{

#if defined(UNICODE) || defined(_UNICODE)
#	define _CTEXT(x)	L ## x
#	define cstrcmp		wcscmp
#	define cAudioChar	wchar_t
#	define cfopen		_wfopen
#else
#	define _CTEXT(x) x
#	define cstrcmp		strcmp
#	define cAudioChar	char
#	define cfopen		fopen
#endif

#if CAUDIO_REROUTE_STRING_ALLOCATIONS == 1
	typedef std::basic_string< cAudioChar, std::char_traits<cAudioChar>, cSTLAllocator<cAudioChar> > cAudioString;
#else
#	if defined(UNICODE) || defined(_UNICODE)
		typedef std::basic_string<cAudioChar> cAudioString;
#	else
	typedef std::string cAudioString;
#	endif
#endif

	// Fixed by LiXizhi 2021.10.05: DONOT do any utf8 to default encoding conversion. 

    static const char* toWINSTR(const char* str) 
    {
		return str;
    }

    static const char* toUTF8(const cAudioString& str)
    {
        return str.c_str();
    }

    static cAudioString fromUTF8(const char* str)
    {
        return cAudioString(str);
    }
};

