#pragma once

#include "IParaAudioEngine.h"
#include "cAudio.h"
#include <string>
#include <map>
#include <functional>
#include <limits.h>

namespace ParaEngine
{
	using namespace std;
	using namespace cAudio;
	class CParaAudioEngine;
	class ParaAudioCaptureBuffer;

	class CParaAudioCapture : public IParaAudioCapture
	{
	public:
		CParaAudioCapture();
		virtual ~CParaAudioCapture();

		virtual void Release();

		virtual bool initialize(const char* deviceName = 0x0, unsigned int frequency = 22050, ParaAudioFormats format = EAF_16BIT_MONO, unsigned int internalBufferSize = 8192) override;

		virtual bool isReady() override;


		virtual void updateCaptureBuffer(bool force = false) override;


		virtual void shutdown() override;


		virtual bool isUpdateThreadRunning() override;


		virtual const char* getDeviceName() override;


		virtual unsigned int getFrequency() override;


		virtual ParaAudioFormats getFormat() override;


		virtual unsigned int getInternalBufferSize() override;


		virtual unsigned int getSampleSize() override;


		virtual bool setDevice(const char* deviceName) override;


		virtual bool setFrequency(unsigned int frequency) override;


		virtual bool setFormat(ParaAudioFormats format) override;


		virtual bool setInternalBufferSize(unsigned int internalBufferSize) override;


		virtual bool beginCapture() override;


		virtual void stopCapture() override;


		virtual unsigned int getCapturedAudio(void* outputBuffer, unsigned int outputBufferSize) override;


		virtual ParaAudioCaptureBuffer* getCapturedAudioBuffer() override;


		virtual unsigned int getCurrentCapturedAudioSize() override;

		//! Returns the name of an available playback device.
		/** \param index: Specify which name to retrieve ( Range: 0 to getAvailableDeviceCount()-1 )
		\return Name of the selected device. */
		virtual const char* getAvailableDeviceName(unsigned int index);

		//! Returns the number of playback devices available for use.
		/** \return Number of playback devices available. */
		virtual unsigned int getAvailableDeviceCount();

		//! Returns the name of the default system playback device.
		/** \return Name of the default playback device. */
		virtual const char* getDefaultDeviceName();

		/** save current captured audio to a file, currently only ogg/wav file format are supported.
		* @param filename: writable full path, like "temp/capture.ogg"
		* @param baseQuality: value in range [0.1, 1].   0.1 is lowest quality, 1 is best quality.  0.4 is usual
		* @return the number of bytes written
		*/
		virtual unsigned int saveToFile(const char* filename, float baseQuality = 0.1f);

		/** save raw audio buffer to a file, currently only ogg/wav file format are supported.
		* @param filename: writable full path, like "temp/capture.ogg"
		* @param pBuffer:
		* @param baseQuality: value in range [0.1, 1].   0.1 is lowest quality, 1 is best quality.  0.4 is usual
		* @return the number of bytes written
		*/
		virtual unsigned int saveToFile(const char* filename, const char* pBuffer, int nSize, float baseQuality);
	private:
		IAudioCapture* m_pAudioCapture;
		IAudioDeviceList* m_deviceList;
		bool m_bIsCapturing;
	};
}
