// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "cMutex.h"
#include "IPluginManager.h"
#include "IAudioPlugin.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

#ifdef CAUDIO_PLATFORM_WIN
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>

struct HINSTANCE__;
typedef struct HINSTANCE__* hInstance;

#	define DYNLIB_HANDLE hInstance
#	define DYNLIB_LOAD( a ) LoadLibraryEx( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH )
#	define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#	define DYNLIB_UNLOAD( a ) !FreeLibrary( a )
// #elif defined(CAUDIO_PLATFORM_MAC) || defined(CAUDIO_PLATFORM_LINUX)
#else
#   include <dlfcn.h>
#	define DYNLIB_HANDLE void*
#	define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL)
#	define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#	define DYNLIB_UNLOAD( a ) dlclose( a )
#endif

namespace cAudio
{
	class cPluginManager : public IPluginManager
	{
	public:
		cPluginManager();
		~cPluginManager();

		static cPluginManager* Instance()
		{
			static cPluginManager theInstance;
			return &theInstance;
		}

		virtual bool installPlugin(IAudioPlugin* plugin, const char* name);
		virtual bool installPlugin(const char* filename, const char* name);

		virtual bool checkForPlugin(const char* name);
		virtual IAudioPlugin* getPlugin(const char* name);
		virtual unsigned int getPluginCount();
		cAudioVector<IAudioPlugin*>::Type getPluginList();

		virtual void uninstallPlugin(IAudioPlugin* plugin);
		virtual void uninstallPlugin(const char* name);

		void autoLoadPlugins();
	protected:
		cAudioString getError();

		cAudioMap<cAudioString, IAudioPlugin*>::Type RegisteredPlugins;
		typedef cAudioMap<cAudioString, IAudioPlugin*>::Type::iterator RegisteredPluginsIterator;
#if !defined(CAUDIO_PLATFORM_IPHONE)
		cAudioMap<IAudioPlugin*, DYNLIB_HANDLE>::Type DynamicallyLoadedPlugins;
		typedef cAudioMap<IAudioPlugin*, DYNLIB_HANDLE>::Type::iterator DynamicallyLoadedPluginsIterator;
#endif
	};
};

#endif //! CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT