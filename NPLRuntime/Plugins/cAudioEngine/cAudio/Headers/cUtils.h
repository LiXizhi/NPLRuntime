// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cAudioPlatform.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"

#ifdef CAUDIO_PLATFORM_WIN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#endif

#ifdef CAUDIO_PLATFORM_LINUX
#	include <dirent.h> 
#	include <stdio.h> 
#	include <cstring>
#endif

namespace cAudio
{

//! Grabs the current extention of a given string.
static cAudioString getExt(const cAudioString& filename)
{
	if(filename.find_last_of(_CTEXT(".")) == cAudioString::npos) return filename;
	return filename.substr(filename.find_last_of(_CTEXT(".")) + 1, filename.length()-filename.find_last_of(_CTEXT("."))-1);
}

//! Returns a list of files/directories in the supplied directory.  Used internally for auto-installation of plugins.
static cAudioVector<cAudioString>::Type getFilesInDirectory(cAudioString path)
{
	cAudioVector<cAudioString>::Type FileList;
#ifdef CAUDIO_PLATFORM_WIN
	cAudioString search = path + _CTEXT("\\") + cAudioString(_CTEXT("*.*"));
	WIN32_FIND_DATA info;
	HANDLE h = FindFirstFile(search.c_str(), &info);
	if (h != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(cstrcmp(info.cFileName, _CTEXT(".")) == 0 || cstrcmp(info.cFileName, _CTEXT("..")) == 0))
			{
				FileList.push_back(info.cFileName);
			}
		} while (FindNextFile(h, &info));
		FindClose(h);
	}
#endif

#ifdef CAUDIO_PLATFORM_LINUX
	DIR           *d;
	struct dirent *dir;
	d = opendir(path.c_str());
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if( strcmp( dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) { continue; }
			if( dir->d_type == DT_DIR ) continue;
			FileList.push_back(dir->d_name);
		}

		closedir(d);
	}
#endif

	return FileList;
}

};
