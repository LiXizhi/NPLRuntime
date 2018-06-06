#include "../Headers/cPluginManager.h"
#include "../Headers/cUtils.h"
#include "../include/cAudioPlatform.h"
#include "../include/cAudioDefines.h"
#include "../include/ILogger.h"

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT

namespace cAudio
{

#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
typedef IAudioPlugin* (*GetPluginModule)(const char* version);
#endif

cPluginManager::cPluginManager()
{
#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
	autoLoadPlugins();
#endif
}

cPluginManager::~cPluginManager()
{
#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
	DynamicallyLoadedPluginsIterator it;
	for(it = DynamicallyLoadedPlugins.begin(); it != DynamicallyLoadedPlugins.end(); it++)
	{
		//Found a plugin we loaded from the filesystem, unload it and delete the plugin
		it->first->drop();
		if(DYNLIB_UNLOAD(it->second))
		{
			//Could be an error, not reporting it for now
		}
	}
#endif
}

bool cPluginManager::installPlugin(IAudioPlugin* plugin, const char* name)
{
	if(plugin)
	{
		cAudioString theName = safeCStr(name);
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
#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
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

#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
		DynamicallyLoadedPluginsIterator it2 = DynamicallyLoadedPlugins.find(plugin);
		if(it2 != DynamicallyLoadedPlugins.end())
		{
			//Found a plugin we loaded from the filesystem, unload it and delete the plugin
			it2->first->drop();
			if(DYNLIB_UNLOAD(it2->second))
			{
				//Could be an error, not reporting it for now
			}
			DynamicallyLoadedPlugins.erase(it2->first);
		}
#endif
	}
}

void cPluginManager::uninstallPlugin(const char* name)
{
	if(RegisteredPlugins.find(name) != RegisteredPlugins.end())
	{
		uninstallPlugin(RegisteredPlugins[name]);
	}
}

#ifdef CAUDIO_COMPILE_WITH_DYNAMIC_PLUGIN_SUPPORT
void cPluginManager::autoLoadPlugins()
{
	cAudioVector<cAudioString>::Type fileList = getFilesInDirectory(".");
	for(int i=0; i<fileList.size(); ++i)
	{
		if(fileList[i].substr(0, 4) == "cAp_")
		{
#ifdef CAUDIO_PLATFORM_WIN
			if(fileList[i].substr(fileList[i].length()-4, 4) == ".dll")
#endif
#ifdef CAUDIO_PLATFORM_LINUX
			if(fileList[i].substr(fileList[i].length()-3, 3) == ".so")
#endif
#ifdef CAUDIO_PLATFORM_MAC
			if(fileList[i].substr(fileList[i].length()-6, 6) == ".dylib")
#endif
			{
				//Found a plugin, load it
				installPlugin(cAudioString("./" + fileList[i]).c_str(), NULL);
			}
		}
	}
}
#endif

CAUDIO_API IPluginManager* getPluginManager()
{
	return cPluginManager::Instance();
}

};

#endif //! CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
