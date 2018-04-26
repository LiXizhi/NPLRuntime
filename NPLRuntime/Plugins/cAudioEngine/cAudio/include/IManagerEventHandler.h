// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

namespace cAudio
{
	//! Interface for event handlers for playback manager events.
	class IManagerEventHandler
	{
	public:
		//! This function is called on manager initialization.
		virtual void onInit() = 0;

		//! This function is called on every manager update.
		virtual void onUpdate() = 0;

		//! This function is called on manager shutdown.
		virtual void onRelease() = 0;

		//! This function is called on when an audio source is created.
		virtual void onSourceCreate() = 0;

		//! This function is called when an audio decoder is registered.
		virtual void onDecoderRegister() = 0;

		//! This function is called when a data source is registered.
		virtual void onDataSourceRegister() = 0;
	};
};

