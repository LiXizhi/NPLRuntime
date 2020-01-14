// Copyright (c) 2008-2010 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#ifndef CAUDIOCAPTURE_H
#define CAUDIOCAPTURE_H

#include "../include/IAudioCapture.h"
#include <AL/al.h>
#include <AL/alc.h>
#include "../Headers/cMutex.h"
#include "../Headers/cMemoryOverride.h"
#include "../Headers/cSTLAllocator.h"

namespace cAudio
{
	class cAudioCapture : public IAudioCapture, public cMemoryOverride
	{
	public:
		
		enum Events{
			ON_INIT,
			ON_UPDATE,
			ON_RELEASE,
			ON_BEGINCAPTURE,
			ON_ENDCAPTURE,
			ON_USERREQUESTEDBUFFER,
		};

		cAudioCapture();
		~cAudioCapture();

		//! Checks to see if capturing audio is supported by OpenAL
		bool checkCaptureExtension();

		virtual bool initialize(const char* deviceName = 0x0, unsigned int frequency = 22050, AudioFormats format = EAF_16BIT_MONO, unsigned int internalBufferSize = 8192);
		virtual bool isSupported() { return Supported; }
		virtual bool isReady() { return Ready; }
		virtual void updateCaptureBuffer(bool force = false);
		virtual void shutdown();

		virtual const char* getAvailableDeviceName(unsigned int index);
		virtual unsigned int getAvailableDeviceCount();
		virtual const char* getDefaultDeviceName();

		virtual const char* getDeviceName() { return DeviceName.c_str(); }
		virtual unsigned int getFrequency() { return Frequency; }
		virtual AudioFormats getFormat() { return Format; }
		virtual unsigned int getInternalBufferSize() { return InternalBufferSize; }
		virtual unsigned int getSampleSize() { return SampleSize; }

		virtual bool setDevice(const char* deviceName);
		virtual bool setFrequency(unsigned int frequency);
		virtual bool setFormat(AudioFormats format);
		virtual bool setInternalBufferSize(unsigned int internalBufferSize);

		virtual bool beginCapture();
		virtual void stopCapture();
		virtual unsigned int getCapturedAudio(void* outputBuffer, unsigned int outputBufferSize);

		virtual unsigned int getCurrentCapturedAudioSize();
		void getAvailableDevices();

		virtual void registerEventHandler(ICaptureEventHandler* handler);
		virtual void unRegisterEventHandler(ICaptureEventHandler* handler);
		virtual void unRegisterAllEventHandlers();

	protected:
		cAudioMutex Mutex;

		bool initOpenALDevice();
		void shutdownOpenALDevice();

		unsigned int Frequency;
		AudioFormats Format;
		unsigned int InternalBufferSize;
		int SampleSize;

		cAudioVector<char>::Type CaptureBuffer;
		cAudioVector<cAudioString>::Type AvailableDevices;
		cAudioString DefaultDevice;
		cAudioList<ICaptureEventHandler*>::Type eventHandlerList;

		bool Supported;
		bool Ready;
		bool Capturing;

		cAudioString DeviceName;
		ALCdevice* CaptureDevice;

		bool checkError();
		ALenum convertAudioFormatEnum(AudioFormats format);
		void signalEvent(Events sevent);
	};
};

#endif //! CAUDIOCAPTURE_H