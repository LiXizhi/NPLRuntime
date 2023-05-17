// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioDefines.h"
#include "cAudioPlatform.h"
#include "cAudioMemory.h"
#include "cSTLAllocator.h"
#include <vector>

#ifndef DEFAULT_FILE_ENCODING
#if WIN32
#define DEFAULT_FILE_ENCODING		CP_UTF8
#endif
#endif

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

#if defined(UNICODE) || defined(_UNICODE) || defined(DEFAULT_FILE_ENCODING)
#	define _CTEXT(x)	L ## x
#	define cstrcmp		wcscmp
#	define cAudioChar	wchar_t
#	define cfopen		_wfopen
#   define freadMode    L"rb"
#   define fwriteMode    L"wb"
#   define cFindFirstFile FindFirstFileW
#   define cFindNextFile FindNextFileW
#   define cWIN32_FIND_DATA WIN32_FIND_DATAW
#else
#	define _CTEXT(x) x
#	define cstrcmp		strcmp
#	define cAudioChar	char
#	define cfopen		fopen
#   define freadMode    "rb"
#   define fwriteMode    "wb"
#   define cFindFirstFile FindFirstFile
#   define cFindNextFile FindNextFile
#   define cWIN32_FIND_DATA WIN32_FIND_DATA
#endif

#if CAUDIO_REROUTE_STRING_ALLOCATIONS == 1
	typedef std::basic_string< cAudioChar, std::char_traits<cAudioChar>, cSTLAllocator<cAudioChar> > cAudioString;
#else
#if defined(UNICODE) || defined(_UNICODE) || defined(DEFAULT_FILE_ENCODING)
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

	const wchar_t* cMultiByteToWideChar(const char* name, unsigned int nCodePage, size_t* outLen = nullptr);
	const char* cWideCharToMultiByte(const wchar_t* name, unsigned int nCodePage, size_t* outLen = nullptr);

	static const char* toUTF8(const cAudioString& str)
	{
#if defined(UNICODE) || defined(_UNICODE) || defined(DEFAULT_FILE_ENCODING)
		return cWideCharToMultiByte(str.c_str(), DEFAULT_FILE_ENCODING);
#else
        static std::string g_str = str;
		return g_str.c_str();
#endif
	}

	static cAudioString fromUTF8(const char* str)
	{
#if defined(UNICODE) || defined(_UNICODE) || defined(DEFAULT_FILE_ENCODING)
		LPCWSTR str16 = cMultiByteToWideChar(str, DEFAULT_FILE_ENCODING);
		return cAudioString(str16);
#else
		return cAudioString(str);
#endif
	}

};

