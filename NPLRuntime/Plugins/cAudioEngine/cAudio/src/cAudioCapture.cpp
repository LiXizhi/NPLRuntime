// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "cAudio.h"
#include "cAudioCapture.h"
#include "cUtils.h"
#include "cThread.h"
#include "cAudioSleep.h"
#include "cLogger.h"
#include "cPluginManager.h"

#include <string.h>
#include <iostream>
namespace cAudio
{
	cAudioCapture::cAudioCapture() : Frequency(22050), Format(EAF_16BIT_MONO), InternalBufferSize(8192),
									SampleSize(2), Ready(false), Capturing(false), 
									CaptureDevice(NULL), AudioThread(NULL)
	{
        
	}
	cAudioCapture::~cAudioCapture()
	{
		shutdown();
	}

	void cAudioCapture::run()
	{
		updateCaptureBuffer();
		cAudioSleep(1);
	}
	
	bool cAudioCapture::initOpenALDevice()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(CaptureDevice)
			shutdownOpenALDevice();

		// Fixed by LiXizhi, 2021.10.14, on some android devices, only buffer size 32768 is possible to alcCaptureOpenDevice. 
		InternalBufferSize = 32768;
		if(DeviceName.empty())
			CaptureDevice = alcCaptureOpenDevice(NULL, Frequency, convertAudioFormatEnum(Format), InternalBufferSize);
		else
			CaptureDevice = alcCaptureOpenDevice(toUTF8(DeviceName), Frequency, convertAudioFormatEnum(Format), InternalBufferSize);
		if(CaptureDevice)
		{
			DeviceName = fromUTF8(alcGetString(CaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER));
			Ready = true;
			checkError();
			getLogger()->logDebug("AudioCapture", "OpenAL Capture Device Opened.");

			return true;
		}
		checkError();

		return false;
	}

	void cAudioCapture::shutdownOpenALDevice()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Capturing)
			stopCapture();

		if(CaptureDevice)
		{
			alcCaptureCloseDevice(CaptureDevice);
			CaptureDevice = NULL;
			Ready = false;
			getLogger()->logDebug("AudioCapture", "OpenAL Capture Device Closed.");
			signalEvent(ON_RELEASE);
		}
		checkError();
		CaptureBuffer.clear();
	}

	void cAudioCapture::shutdown()
	{
		cAudioMutexBasicLock lock(Mutex);

		if (AudioThread)
		{
			AudioThread->join();
			delete AudioThread;
			AudioThread = NULL;
		}

		shutdownOpenALDevice();
		signalEvent(ON_RELEASE);
	}

	void cAudioCapture::updateCaptureBuffer(bool force)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Capturing && CaptureDevice && Ready)
		{
			int AvailableSamples = 0;
			alcGetIntegerv(CaptureDevice, ALC_CAPTURE_SAMPLES, 1, &AvailableSamples);
			const unsigned int availbuffersize = AvailableSamples * SampleSize;

			//If the samples in the OpenAL buffer are more than half of its max size, grab them
			// if(availbuffersize > InternalBufferSize / 2 || force)
			{
				//Fixes a bug with the capture being forced, but no data being available
				if(availbuffersize > 0)
				{
					const unsigned int oldBufferSize = CaptureBuffer.size();
					CaptureBuffer.resize(oldBufferSize + availbuffersize, 0);
					alcCaptureSamples(CaptureDevice, &CaptureBuffer[oldBufferSize], AvailableSamples);
					checkError();
					getLogger()->logDebug("AudioCapture", "Captured %i bytes of audio data.", availbuffersize);
					signalEvent(ON_UPDATE);
				}
			}
		}
	}

	bool cAudioCapture::beginCapture()
	{
		cAudioMutexBasicLock lock(Mutex);
	
		if(!Capturing)
		{
#ifdef __EMSCRIPTEN__
			shutdownOpenALDevice();
			initOpenALDevice();
#endif
			CaptureBuffer.clear();
			if(CaptureDevice && Ready)
			{
				alcCaptureStart(CaptureDevice);
				if (checkError())
				{
					// device may be disconnected. we will try initialize it again 
					shutdownOpenALDevice();
					initOpenALDevice();

					if (CaptureDevice && Ready)
					{
						alcCaptureStart(CaptureDevice);
						checkError();
					}
				}
				Capturing = true;
				getLogger()->logDebug("AudioCapture", "OpenAL Capture Started.");
				signalEvent(ON_BEGINCAPTURE);
			}
			
			return Capturing;
		}
		checkError();
		return false;
	}

	void cAudioCapture::stopCapture()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(CaptureDevice && Ready)
		{
			alcCaptureStop(CaptureDevice);
			updateCaptureBuffer(true);
			checkError();
			getLogger()->logDebug("AudioCapture", "OpenAL Capture Stopped.");
			signalEvent(ON_ENDCAPTURE);
		}
		Capturing = false;
	}

	AudioCaptureBuffer* cAudioCapture::getCapturedAudioBuffer()
	{
		AudioCaptureBuffer* buffer = new AudioCaptureBuffer(CaptureBuffer.size());
		getCapturedAudio(buffer->getWriteBuffer(), buffer->getLength());
		return buffer;
	}

	unsigned int cAudioCapture::getCapturedAudio(void* outputBuffer, unsigned int outputBufferSize)
	{
		cAudioMutexBasicLock lock(Mutex);
		unsigned int internalBufferSize = CaptureBuffer.size();
		if(outputBuffer && outputBufferSize > 0 && internalBufferSize > 0)
		{
			int sizeToCopy = (outputBufferSize >= internalBufferSize) ? internalBufferSize : outputBufferSize;
			memcpy(outputBuffer, &CaptureBuffer[0], sizeToCopy);
			CaptureBuffer.erase(CaptureBuffer.begin(), CaptureBuffer.begin()+sizeToCopy);

			getLogger()->logDebug("AudioCapture", "Copied out %i bytes of data out of %i bytes in the buffer at user request.", sizeToCopy, internalBufferSize);
			signalEvent(ON_USERREQUESTEDBUFFER);
			return sizeToCopy;
		}
		return 0;
	}

	unsigned int cAudioCapture::getCurrentCapturedAudioSize()
	{
		cAudioMutexBasicLock lock(Mutex);
		return CaptureBuffer.size();
	}

	bool cAudioCapture::setFrequency(unsigned int frequency)
	{
		cAudioMutexBasicLock lock(Mutex);
		Frequency = frequency;
		shutdownOpenALDevice();
		return initOpenALDevice();
	}

	bool cAudioCapture::setFormat(AudioFormats format)
	{
		cAudioMutexBasicLock lock(Mutex);
		Format = format;
		if(Format == EAF_8BIT_MONO)
			SampleSize = 1;
		else if(Format == EAF_8BIT_STEREO)
			SampleSize = 2;
		else if(Format == EAF_16BIT_MONO)
			SampleSize = 2;
		else
			SampleSize = 4;

		shutdownOpenALDevice();
		return initOpenALDevice();
	}

	bool cAudioCapture::setInternalBufferSize(unsigned int internalBufferSize)
	{
		cAudioMutexBasicLock lock(Mutex);
		InternalBufferSize = internalBufferSize;

		shutdownOpenALDevice();
		return initOpenALDevice();
	}

	bool cAudioCapture::setDevice(const char* deviceName)
	{
		cAudioMutexBasicLock lock(Mutex);
		DeviceName = fromUTF8(deviceName);

		shutdownOpenALDevice();
		return initOpenALDevice();
	}

	bool cAudioCapture::initialize(const char* deviceName, unsigned int frequency, AudioFormats format, unsigned int internalBufferSize)
	{
		cAudioMutexBasicLock lock(Mutex);
		DeviceName = fromUTF8(deviceName);
		Frequency = frequency;
		InternalBufferSize = internalBufferSize;

		Format = format;
		if(Format == EAF_8BIT_MONO)
			SampleSize = 1;
		else if(Format == EAF_8BIT_STEREO)
			SampleSize = 2;
		else if(Format == EAF_16BIT_MONO)
			SampleSize = 2;
		else
			SampleSize = 4;

		shutdownOpenALDevice();
		signalEvent(ON_INIT);
		bool isInit = initOpenALDevice();

#ifdef CAUDIO_USE_INTERNAL_THREAD
		if(isInit)
		{ 
			if (!AudioThread)
			{
				AudioThread = new cAudioThread(this);
			}
			AudioThread->start();
		}
#endif
		return isInit;
	}

	bool cAudioCapture::checkError()
	{
		if(CaptureDevice)
		{
			int error = alcGetError(CaptureDevice);
			if (error != AL_NO_ERROR)
			{
				const char* errorString = alGetString(error);
				getLogger()->logError("AudioCapture", "OpenAL Error: %s.", errorString);
				return true;
			}
		}
		return false;
	}

	ALenum cAudioCapture::convertAudioFormatEnum(AudioFormats format)
	{
		switch(format)
		{
		case EAF_8BIT_MONO:
			return AL_FORMAT_MONO8;
		case EAF_16BIT_MONO:
			return AL_FORMAT_MONO16;
		case EAF_8BIT_STEREO:
			return AL_FORMAT_STEREO8;
		case EAF_16BIT_STEREO:
			return AL_FORMAT_STEREO16;
		default:
			return AL_FORMAT_MONO8;
		};
	}

	void cAudioCapture::registerEventHandler(ICaptureEventHandler* handler)
	{
		if(handler)
		{
			eventHandlerList.push_back(handler);
		}
	}

	void cAudioCapture::unRegisterEventHandler(ICaptureEventHandler* handler)
	{
		if(handler)
		{
			eventHandlerList.remove(handler);
		}
	}

	void cAudioCapture::unRegisterAllEventHandlers()
	{
		eventHandlerList.clear();
	}

	void cAudioCapture::signalEvent(Events sevent)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioList<ICaptureEventHandler*>::Type::iterator it = eventHandlerList.begin();

		if(it != eventHandlerList.end()){

			switch(sevent){

				case ON_INIT: 
					
					for(; it != eventHandlerList.end(); it++){
						(*it)->onInit();
					}

					break;
				
				case ON_UPDATE:

					for(; it != eventHandlerList.end(); it++){
						(*it)->onUpdate();
					}

					break;

				case ON_RELEASE:

					for(; it != eventHandlerList.end(); it++){
						(*it)->onRelease();
					}

					break;

				case ON_BEGINCAPTURE:

					for(; it != eventHandlerList.end(); it++){
						(*it)->onBeginCapture();
					}


					break;

				case ON_ENDCAPTURE:

					for(; it != eventHandlerList.end(); it++){
						(*it)->onEndCapture();
					}

					break;

				case ON_USERREQUESTEDBUFFER:

					for(; it != eventHandlerList.end(); it++){
						(*it)->onUserRequestBuffer();
					}

					break;
			}
		}
	}
};

