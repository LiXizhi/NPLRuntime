// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#pragma once

#include <al.h>
#include <alc.h>
#include "cMutex.h"
#include "cMemoryOverride.h"
#include "IAudioCapture.h"
#include "cSTLAllocator.h"
#include "cAudioString.h"
#include "IThread.h"

namespace cAudio
{
	class cAudioCapture : public IAudioCapture, public cMemoryOverride, public IThreadWorker
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

		virtual bool initialize(const char* deviceName = 0x0, unsigned int frequency = 22050, AudioFormats format = EAF_16BIT_MONO, unsigned int internalBufferSize = 8192);
		virtual bool isReady() { return Ready; }
		virtual void updateCaptureBuffer(bool force = false);
		virtual void shutdown();
		virtual bool isUpdateThreadRunning()
		{
			return (AudioThread != NULL && AudioThread->isRunning());
		}

		virtual const char* getDeviceName() { return toUTF8(DeviceName); }
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
		virtual AudioCaptureBuffer* getCapturedAudioBuffer();

		virtual unsigned int getCurrentCapturedAudioSize();
		void getAvailableDevices();

		virtual void registerEventHandler(ICaptureEventHandler* handler);
		virtual void unRegisterEventHandler(ICaptureEventHandler* handler);
		virtual void unRegisterAllEventHandlers();

	protected:
		virtual void run();

		cAudioMutex Mutex;

		//! Our update thread
		IThread* AudioThread;

		bool initOpenALDevice();
		void shutdownOpenALDevice();

		unsigned int Frequency;
		AudioFormats Format;
		unsigned int InternalBufferSize;
		int SampleSize;

		cAudioVector<char>::Type CaptureBuffer;
		cAudioList<ICaptureEventHandler*>::Type eventHandlerList;

		bool Ready;
		bool Capturing;

		cAudioString DeviceName;
		ALCdevice* CaptureDevice;

		bool checkError();
		ALenum convertAudioFormatEnum(AudioFormats format);
		void signalEvent(Events sevent);
	};
};