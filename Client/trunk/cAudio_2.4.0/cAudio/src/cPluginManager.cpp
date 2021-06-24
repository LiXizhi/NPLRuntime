// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cPluginManager.h"
#include "cUtils.h"
#include "cAudioPlatform.h"
#include "cAudioDefines.h"
#include "ILogger.h"
#include "cAudio.h"

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

namespace cAudio
{
	typedef IAudioPlugin* (*GetPluginModule)(const char* version);

	cPluginManager::cPluginManager()
	{
		autoLoadPlugins();
	}

	cPluginManager::~cPluginManager()
	{
#if !defined(CAUDIO_PLATFORM_IPHONE)
		DynamicallyLoadedPluginsIterator it;
		for(it = DynamicallyLoadedPlugins.begin(); it != DynamicallyLoadedPlugins.end(); it++)
		{
			//Found a plugin we loaded from the filesystem, unload it and delete the plugin
			it->first->drop();
			if(DYNLIB_UNLOAD(it->second))
			{
				getLogger()->logError("cPluginManager", "Plugin Error: %s.", toUTF8(getError()));
			}
		}
#endif
	}

	bool cPluginManager::installPlugin(IAudioPlugin* plugin, const char* name)
	{
		if(plugin)
		{
			cAudioString theName = fromUTF8(name);
			if(theName.empty())
				theName = plugin->getPluginName();

			if(plugin->installPlugin(getLogger()))
			{
				RegisteredPlugins[theName] = plugin;
				return true;
			}
		}
		return false;
	}

	bool cPluginManager::installPlugin(const char* filename, const char* name)
	{
#if !defined(CAUDIO_PLATFORM_IPHONE)
		DYNLIB_HANDLE m_hInst = DYNLIB_LOAD(filename);
		if(m_hInst)
		{
			GetPluginModule moduleFunc = (GetPluginModule)DYNLIB_GETSYM(m_hInst, "GetPluginModule");

			if(moduleFunc)
			{
				IAudioPlugin* plugin = moduleFunc(CAUDIO_VERSION);

				if(plugin)
				{
					DynamicallyLoadedPlugins[plugin] = m_hInst;

					return installPlugin(plugin, name);
				}
			}
			else
				getLogger()->logError("cPluginManager", "installPlugin Error: %s.", toUTF8(getError()));
		}
#endif
		return false;
	}

	bool cPluginManager::checkForPlugin(const char* name)
	{
		return (RegisteredPlugins.find(name) != RegisteredPlugins.end());
	}

	IAudioPlugin* cPluginManager::getPlugin(const char* name)
	{
		if(RegisteredPlugins.find(name) != RegisteredPlugins.end())
		{
			return RegisteredPlugins[name];
		}
		return NULL;
	}

	unsigned int cPluginManager::getPluginCount()
	{
		return RegisteredPlugins.size();
	}

	cAudioVector<IAudioPlugin*>::Type cPluginManager::getPluginList()
	{
		cAudioVector<IAudioPlugin*>::Type list;
		RegisteredPluginsIterator it;
		for(it = RegisteredPlugins.begin(); it != RegisteredPlugins.end(); it++)
		{
			list.push_back(it->second);
		}
		return list;
	}

	void cPluginManager::uninstallPlugin(IAudioPlugin* plugin)
	{
#if !defined(CAUDIO_PLATFORM_IPHONE)
		if(plugin)
		{
			RegisteredPluginsIterator it;
			for(it = RegisteredPlugins.begin(); it != RegisteredPlugins.end(); it++)
			{
				if(it->second == plugin)
				{
					RegisteredPlugins.erase(it->first);
					break;
				}
			}

			DynamicallyLoadedPluginsIterator it2 = DynamicallyLoadedPlugins.find(plugin);
			if(it2 != DynamicallyLoadedPlugins.end())
			{
				//Found a plugin we loaded from the filesystem, unload it and delete the plugin
				it2->first->drop();
				if(DYNLIB_UNLOAD(it2->second))
				{
					getLogger()->logError("cPluginManager", "Plugin Error: %s.", toUTF8(getError()));
				}
				DynamicallyLoadedPlugins.erase(it2->first);
			}
		}
#endif
	}

	void cPluginManager::uninstallPlugin(const char* name)
	{
		if(RegisteredPlugins.find(name) != RegisteredPlugins.end())
		{
			uninstallPlugin(RegisteredPlugins[name]);
		}
	}

	void cPluginManager::autoLoadPlugins()
	{
#if !defined(CAUDIO_PLATFORM_IPHONE)
		cAudioVector<cAudioString>::Type fileList = getFilesInDirectory(".");
		for(size_t i=0; i<fileList.size(); ++i)
		{
			if(fileList[i].substr(0, 4) == _CTEXT("cAp_") ||fileList[i].substr(0, 7) == _CTEXT("libcAp_") )
			{
#ifdef CAUDIO_PLATFORM_WIN
				if(fileList[i].substr(fileList[i].length()-4, 4) == _CTEXT(".dll"))
#elif defined(CAUDIO_PLATFORM_LINUX)
				if(fileList[i].substr(fileList[i].length()-3, 3) == ".so")
#elif defined(CAUDIO_PLATFORM_MAC)
				if(fileList[i].substr(fileList[i].length()-6, 6) == ".dylib")
#endif
				{
					//Found a plugin, load it
					installPlugin(toUTF8(cAudioString(_CTEXT("./") + fileList[i])), fileList[i].c_str());
				}
			}
		}
#endif
	}

	cAudioString cPluginManager::getError() 
	{
#ifdef CAUDIO_PLATFORM_WIN 
		LPVOID lpMsgBuf; 
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, 
			GetLastError(), 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			(LPTSTR) &lpMsgBuf, 
			0, 
			NULL 
			);

		cAudioString ret = fromUTF8((char*)lpMsgBuf);
		// remove line break
		ret = ret.substr(0, ret.length()-2);
		LocalFree(lpMsgBuf);
		return ret;

#elif defined(CAUDIO_PLATFORM_MAC) || defined(CAUDIO_PLATFORM_LINUX)
		const char* error = dlerror();

		return error != NULL ? cAudioString(error) : cAudioString("");
#else
		return cAudioString("");
#endif
	}

	CAUDIO_API IPluginManager* getPluginManager()
	{
		return cPluginManager::Instance();
	}

};

#endif //! CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
