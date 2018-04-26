// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "IRefCounted.h"
#include "IAudioManager.h"
#include "IAudioCapture.h"
#include "ILogger.h"

namespace cAudio
{
	//! Interface for all plugins in cAudio.
	class IAudioPlugin : public IRefCounted
	{
	public:
		IAudioPlugin() { }
		~IAudioPlugin() { }

		//! Called to initialize the plugin.  Use this for any initial setup that needs to be done.
		/** 
		\param logger: Pointer to the logger singleton.
		\return True if plugin initialization was successful, false if there was an error. */
		virtual bool installPlugin(ILogger* logger) = 0;

		//! Called to retrieve a name to register this plugin under if the user didn't specify one (or the plugin was auto-loaded).
		/**
		\return Name of the plugin. */
		virtual const char* getPluginName() = 0;

		//! Called to shutdown the plugin and do any final cleanup.  This is the last function call cAudio will give you before your plugin is completely removed.
		virtual void uninstallPlugin() = 0;

		//! Called on creation of an Audio Playback Manager.
		/** Use this to register any decoders or event handlers that you'd like to add to the engine.
		\param manager: Pointer to the newly created manager. */
		virtual void onCreateAudioManager(IAudioManager* manager) { }

		//! Called on creation of an Audio Capture Manager.
		/** Use this to register any event handlers that you'd like to add to the engine.
		\param capture: Pointer to the newly created capture interface. */
		virtual void onCreateAudioCapture(IAudioCapture* capture) { }

		//! Called on destruction of an Audio Playback Manager.
		/** Use this to remove anything you added to the playback manger earlier.
		\param manager: Pointer to the soon to be destroyed playback interface. */
		virtual void onDestroyAudioManager(IAudioManager* manager) { }

		//! Called on destruction of an Audio Capture Manager.
		/** Use this to remove anything you added to a capture manger earlier.
		\param capture: Pointer to the soon to be destroyed capture interface. */
		virtual void onDestoryAudioCapture(IAudioCapture* capture) { }
	};
};