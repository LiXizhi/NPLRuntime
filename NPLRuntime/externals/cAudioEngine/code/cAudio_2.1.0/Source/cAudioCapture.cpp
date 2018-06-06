// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "../Headers/cAudioCapture.h"
#include "../Headers/cUtils.h"
#include "../Headers/cThread.h"
#include "../include/cAudioSleep.h"
#include "../Headers/cLogger.h"
#include "../Headers/cPluginManager.h"

#include <string.h>

namespace cAudio
{
	static bool RunAudioCaptureThread(false);

	//Note: OpenAL is threadsafe, so a mutex only needs to protect the class state
#ifdef CAUDIO_USE_INTERNAL_THREAD
	static cAudioMutex AudioCaptureObjectsMutex;
	static cAudioSet<IAudioCapture*>::Type AudioCaptureObjects;

	CAUDIO_DECLARE_THREAD_FUNCTION(AudioCaptureUpdateThread)
	{
		while(RunAudioCaptureThread)
		{
			AudioCaptureObjectsMutex.lock();
			cAudioSet<IAudioCapture*>::Type::iterator it;
			for ( it=AudioCaptureObjects.begin() ; it != AudioCaptureObjects.end(); it++ )
			{
				(*it)->updateCaptureBuffer();
			}
			AudioCaptureObjectsMutex.unlock();
			cAudioSleep(1);
		}
		return 0;
	}
#endif

	cAudioCapture::cAudioCapture() : Frequency(22050), Format(EAF_16BIT_MONO), InternalBufferSize(8192),
									SampleSize(2), Supported(false), Ready(false), Capturing(false), 
									CaptureDevice(NULL)
	{
		checkCaptureExtension();
		getAvailableDevices();
	}
	cAudioCapture::~cAudioCapture()
	{
		shutdown();
	}

	bool cAudioCapture::checkCaptureExtension()
	{
		cAudioMutexBasicLock lock(Mutex);
		// Check for Capture Extension support
		Supported = ( alcIsExtensionPresent(NULL, "ALC_EXT_CAPTURE") == AL_TRUE );
		return Supported;
	}

	bool cAudioCapture::initOpenALDevice()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Supported)
		{
			if(CaptureDevice)
				shutdownOpenALDevice();
			if(DeviceName.empty())
				CaptureDevice = alcCaptureOpenDevice(NULL, Frequency, convertAudioFormatEnum(Format), InternalBufferSize / SampleSize);
			else
				CaptureDevice = alcCaptureOpenDevice(DeviceName.c_str(), Frequency, convertAudioFormatEnum(Format), InternalBufferSize / SampleSize);
			if(CaptureDevice)
			{
				DeviceName = alcGetString(CaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER);
				Ready = true;
				checkError();
				getLogger()->logDebug("AudioCapture", "OpenAL Capture Device Opened.");

				return true;
			}
		}
		checkError();
		return false;
	}

	void cAudioCapture::shutdownOpenALDevice()
	{
		cAudioMutexBasicLock lock(Mutex);
		if(Supported)
		{
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
	}

	void cAudioCapture::shutdown()
	{
		cAudioMutexBasicLock lock(Mutex);
		shutdownOpenALDevice();
		signalEvent(ON_RELEASE);
	}

	void cAudioCapture::getAvailableDevices()
	{
		// Get list of available Capture Devices
		cAudioMutexBasicLock lock(Mutex);
		if( alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE )
		{
			const char* deviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
			if (deviceList)
			{
				while(*deviceList)
				{
					cAudioString device(deviceList);
					AvailableDevices.push_back(device);
					deviceList += strlen(deviceList) + 1;
				}
			}

			// Get the name of the 'default' capture device
			DefaultDevice = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
		}
	}

	const char* cAudioCapture::getAvailableDeviceName(unsigned int index)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(!AvailableDevices.empty())
		{
			//Bounds check
			if( index > (AvailableDevices.size()-1) ) index = (AvailableDevices.size()-1);
			const char* deviceName = AvailableDevices[index].c_str();
			return deviceName;
		}
		return "";
	}

	unsigned int cAudioCapture::getAvailableDeviceCount()
	{
		cAudioMutexBasicLock lock(Mutex);
		return AvailableDevices.size();
	}

	const char* cAudioCapture::getDefaultDeviceName()
	{
		cAudioMutexBasicLock lock(Mutex);
		return DefaultDevice.empty() ? "" : DefaultDevice.c_str();
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
			if(availbuffersize > InternalBufferSize / 2 || force)
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
			CaptureBuffer.clear();
			if(CaptureDevice && Ready)
			{
				alcCaptureStart(CaptureDevice);
				Capturing = true;
				getLogger()->logDebug("AudioCapture", "OpenAL Capture Started.");
				signalEvent(ON_BEGINCAPTURE);
			}
			checkError();
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
		DeviceName = safeCStr(deviceName);

		shutdownOpenALDevice();
		return initOpenALDevice();
	}

	bool cAudioCapture::initialize(const char* deviceName, unsigned int frequency, AudioFormats format, unsigned int internalBufferSize)
	{
		cAudioMutexBasicLock lock(Mutex);
		DeviceName = safeCStr(deviceName);
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
		return initOpenALDevice();
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

	CAUDIO_API IAudioCapture* createAudioCapture(bool initializeDefault)
	{
		cAudioCapture* capture = CAUDIO_NEW cAudioCapture;
		if(capture)
		{
			if(initializeDefault)
				capture->initialize();

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onCreateAudioCapture(capture);
			}
#endif

#ifdef CAUDIO_USE_INTERNAL_THREAD
			AudioCaptureObjectsMutex.lock();
			AudioCaptureObjects.insert(capture);

			//First time launch of thread
			if(!RunAudioCaptureThread && AudioCaptureObjects.size() > 0)
				RunAudioCaptureThread = (cAudioThread::SpawnThread(AudioCaptureUpdateThread, NULL) == 0);
			AudioCaptureObjectsMutex.unlock();
#endif
		}
		return capture;
	}

	CAUDIO_API void destroyAudioCapture(IAudioCapture* capture)
	{
		if(capture)
		{
#ifdef CAUDIO_USE_INTERNAL_THREAD
			AudioCaptureObjectsMutex.lock();
			AudioCaptureObjects.erase(capture);

			//Kill the thread if there are no objects to process anymore
			if(RunAudioCaptureThread && AudioCaptureObjects.empty())
				RunAudioCaptureThread = false;
			AudioCaptureObjectsMutex.unlock();
#endif

#ifdef CAUDIO_COMPILE_WITH_PLUGIN_SUPPORT
			cAudioVector<IAudioPlugin*>::Type plugins = cPluginManager::Instance()->getPluginList();
			for(unsigned int i = 0; i < plugins.size(); ++i)
			{
				plugins[i]->onDestoryAudioCapture(capture);
			}
#endif

			CAUDIO_DELETE capture;
			capture = NULL;
		}
	}

	CAUDIO_API bool isAudioCaptureThreadRunning()
	{
		return RunAudioCaptureThread;
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
					
					for(it; it != eventHandlerList.end(); it++){
						(*it)->onInit();
					}

					break;
				
				case ON_UPDATE:

					for(it; it != eventHandlerList.end(); it++){
						(*it)->onUpdate();
					}

					break;

				case ON_RELEASE:

					for(it; it != eventHandlerList.end(); it++){
						(*it)->onRelease();
					}

					break;

				case ON_BEGINCAPTURE:

					for(it; it != eventHandlerList.end(); it++){
						(*it)->onBeginCapture();
					}


					break;

				case ON_ENDCAPTURE:

					for(it; it != eventHandlerList.end(); it++){
						(*it)->onEndCapture();
					}

					break;

				case ON_USERREQUESTEDBUFFER:

					for(it; it != eventHandlerList.end(); it++){
						(*it)->onUserRequestBuffer();
					}

					break;
			}
		}
	}
};

