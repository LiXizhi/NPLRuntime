// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

namespace cAudio
{
	//! Interface for recieving Capture Manager Events
	class ICaptureEventHandler
	{

	public:
		//! This function is called on capture device intialize.
		virtual void onInit() = 0;

		//! This function is called when the capture manager update's its internal buffers.
		virtual void onUpdate() = 0;

		//! This function is called on capture device shutdown.
		virtual void onRelease() = 0;

		//! This function is called when the user begins capturing audio.
		virtual void onBeginCapture() = 0;

		//! This function is called when the user ends a capture operation.
		virtual void onEndCapture() = 0;

		//! This function is called whe user requests data from the capture buffer.
		virtual void onUserRequestBuffer() = 0;

	};
};
