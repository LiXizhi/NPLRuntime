// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IAudioPlugin.h"
#include "cAudioDefines.h"

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
namespace cAudio
{
	//! Interface for the plugin capabilities of cAudio.
	class IPluginManager
	{
	public:
		IPluginManager() { }
		~IPluginManager() { }

		//! Installs a plugin using a statically linked plugin implementation.
		/**
		\param plugin: Pointer to a plugin implementation to install.
		\param name: Optional name for the plugin.  Pass NULL to have the plugin use its default name. 
		\return True if plugin was installed successfully. */
		virtual bool installPlugin(IAudioPlugin* plugin, const char* name = NULL) = 0;

		//! Installs a plugin from a dynamically linked library on your hard drive.
		/**
		\param filename: Path to the dll/so/dynlib on your hard drive.
		\param name: Optional name for the plugin.  Pass NULL to have the plugin use its default name. 
		\return True if plugin was installed successfully. */
		virtual bool installPlugin(const char* filename, const char* name = NULL) = 0;

		//! Checks for the existance of a plugin with the supplied name.
		/**
		\param name: Name of the plugin to check for. */
		virtual bool checkForPlugin(const char* name) = 0;

		//! Returns a plugin interface for a plugin with the supplied name.
		/**
		\param name: Name of the plugin to check return.
		\return A pointer to the plugin interface or NULL if it could not be found. */
		virtual IAudioPlugin* getPlugin(const char* name) = 0;

		//! Returns the total number of installed plugins.
		virtual unsigned int getPluginCount() = 0;

		//! Removes a plugin installed via statically linked reference.
		/**
		\param plugin: Pointer to the plugin implementation to uninstall. */
		virtual void uninstallPlugin(IAudioPlugin* plugin) = 0;

		//! Removes a plugin with the specified name.
		/**
		\param name: Name of the plugin to uninstall. */
		virtual void uninstallPlugin(const char* name) = 0;
	};

	//! Gets the interface to the plugin manager.
	/** Note: This is the only way to get access to the plugin capabilities of cAudio.
	\return A pointer to the manager.
	*/
	CAUDIO_API IPluginManager* getPluginManager();
};

#endif //! CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
