// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include "EAudioFormats.h"
#include "cAudioDefines.h"
#include "ICaptureEventHandler.h"

namespace cAudio
{
	// Is responsible to create/destroy a capture buffer
	class  AudioCaptureBuffer;

	//! Interface for capturing operations in the cAudio Engine.
	class IAudioCapture
	{
	public:
		IAudioCapture() { }
		virtual ~IAudioCapture() { }

		//! Initializes the capture device to the selected settings.
		/** Note that calling this will cause the capture device to be reinitialized.  Calling while in use will clear the internal audio buffer.
		\param deviceName: Name of the audio device to capture audio from, pass NULL to specify the default one.
		\param frequency: Frequency that the captured audio will be captured at in hertz.
		\param format: Format of the captured audio.
		\param internalBufferSize: Size of the internal OpenAL buffer used to store the captured audio between calls to getCapturedAudio() in bytes.
		\return True on success, False if the capture device failed to initialize.
		*/
		virtual bool initialize(const char* deviceName = 0x0, unsigned int frequency = 22050, AudioFormats format = EAF_16BIT_MONO, unsigned int internalBufferSize = 8192) = 0;

		//! Returns true if the capture device is ready to be used.  False may indicate an error with the current settings. 
		virtual bool isReady() = 0;
		//! Grabs samples from the OpenAL buffer into the capture buffer if the OpenAL buffer has reached half full.  Should be run once every audio frame, unless threading is enabled.
		/** \param force: Force capturing data from the buffer, even if the buffer is not half full. */
		virtual void updateCaptureBuffer(bool force = false) = 0;
		//! Shuts down the capture device, clearing the internal buffer and setting the audio capture into an uninitialized state.  You must call initialize() again in order to reuse this object.
		virtual void shutdown() = 0;

		//! Returns if the thread used to update all Audio Capture Objects is running.
		/** Note: Will always return false if threading is disabled.
		The library automatically shuts down the thread if no Audio Capture objects exist and will restart the thread on creation of a new object.
		\return True if the thread is currently running, false otherwise.
		*/
		virtual bool isUpdateThreadRunning() = 0;

		//! Returns the name of the audio device being used to capture audio.
		virtual const char* getDeviceName() = 0;
		//! Returns the frequency that the captured audio will be at.
		virtual unsigned int getFrequency() = 0;
		//! Returns the format of the captured audio.
		virtual AudioFormats getFormat() = 0;
		//! Returns the internal OpenAL buffer size in bytes.
		/** \return Size of the buffer in bytes. */
		virtual unsigned int getInternalBufferSize() = 0;
		//! Returns the size of a "sample" of audio data.  Useful for making sure you grab audio data at sample boundaries.
		/** \return Size of a sample in bytes. */
		virtual unsigned int getSampleSize() = 0;

		//! Sets the audio device .  Will cause the capture device to be reinitialized.  Calling while in use will clear the internal audio buffer.
		/** 
		\param deviceName: Name of the audio device to capture audio from, pass NULL to specify the default one.
		\return True on success, False if the capture device failed to initialize. */
		virtual bool setDevice(const char* deviceName) = 0;
		//! Sets the frequency that the captured audio will be at.  Will cause the capture device to be reinitialized.  Calling while in use will clear the internal audio buffer.
		/** 
		\param frequency: Frequency that the captured audio will be captured at in hertz.
		\return True on success, False if the capture device failed to initialize. */
		virtual bool setFrequency(unsigned int frequency) = 0;
		//! Sets the format that the captured audio will be at.  Will cause the capture device to be reinitialized.  Calling while in use will clear the internal audio buffer.
		/** 
		\param format: Format of the captured audio.
		\return True on success, False if the capture device failed to initialize. */
		virtual bool setFormat(AudioFormats format) = 0;
		//! Sets the internal buffer size that OpenAL will use to store captured audio between calls to getCapturedAudio() in bytes.  Will cause the capture device to be reinitialized.  Calling while in use will clear the internal audio buffer.
		/** 
		\param internalBufferSize: Size of the internal OpenAL buffer in bytes.
		\return True on success, False if the capture device failed to initialize. */
		virtual bool setInternalBufferSize(unsigned int internalBufferSize) = 0;

		//! Starts capturing audio data to an internal buffer.  Will clear any old data in the buffer.
		/** \return True if capture was successfully started. */
		virtual bool beginCapture() = 0;
		//! Stops capturing audio data to an internal buffer.
		virtual void stopCapture() = 0;
		//! Allows access to the audio data in the internal capture buffer.
		/** Can be called at any time to retrieve recorded audio.  It is recommended that you call it every so often with long recordings to prevent the internal buffer from growing too large.
		Once successfully retrieved, the captured audio will be deleted from the internal buffer.
		\param outputBuffer: Pointer to an output array to copy audio data to.
		\param outputBufferSize: Size of the output array in bytes.
		\return Size in bytes of the data actually copied to the output buffer.
		*/
		virtual unsigned int getCapturedAudio(void* outputBuffer, unsigned int outputBufferSize) = 0;

		//! this method is the same as getCapturedAudio but it returns an managed CaptureBuffer
		virtual AudioCaptureBuffer* getCapturedAudioBuffer() = 0;

		//! Returns the current size of the internal audio buffer in bytes.
		virtual unsigned int getCurrentCapturedAudioSize() = 0;

		//!Registers a new event handler to this manager.
		/** \param handler: Pointer to an event handler to attach. */
		virtual void registerEventHandler(ICaptureEventHandler* handler) = 0;
		//!Removes the specified event handler from this manager.
		/** \param handler: Pointer to an event handler to remove. */
		virtual void unRegisterEventHandler(ICaptureEventHandler* handler) = 0;
		//!Removes all event handlers attached to this manager.
		virtual void unRegisterAllEventHandlers() = 0;	
	};

	// Is responsible to create/destroy a capture buffer
	class  AudioCaptureBuffer
	{
	public:
		AudioCaptureBuffer(size_t inlength)
		{
			length = inlength;
			buffer = new char[length];
		}

		AudioCaptureBuffer(const AudioCaptureBuffer& p)
		{
			buffer = p.buffer;
			length = p.length;
		}

		~AudioCaptureBuffer()
		{
			delete buffer;
			buffer = NULL;
		}

		const char* getReadBuffer() const
		{
			return buffer;
		}

		char* getWriteBuffer()
		{
			return buffer;
		}

		size_t getLength() const
		{
			return length;
		}

	private:
		char* buffer;
		size_t length;
	};
};