#pragma once
#ifndef STATIC_PLUGIN_CAUDIOENGINE
#include "IParaAudioEngine.h"
#include <map>

namespace ParaEngine
{
	class CParaSimpleAudioSource;

	/** simple cross platform implementation of the IParaAudioEngine interface. */
	class CParaSimpleAudioEngine : public IParaAudioEngine
	{
	public:
		typedef std::map<std::string, CParaSimpleAudioSource*> AudioMap_Type;

		CParaSimpleAudioEngine();

		void PrepareAndCleanTempDirectory();

		virtual ~CParaSimpleAudioEngine();

		static CParaSimpleAudioEngine* GetInstance();
		
	public:
		virtual void Release();

		virtual bool initialize(const char* deviceName = 0x0, int outputFrequency = -1, int eaxEffectSlots = 4);

		virtual void shutDown();

		virtual void update();

		virtual IParaAudioSource* getSoundByName(const char* name);

		virtual void releaseAllSources();

		virtual void release(IParaAudioSource* source);

		virtual const char* getAvailableDeviceName(unsigned int index);

		virtual unsigned int getAvailableDeviceCount();

		virtual const char* getDefaultDeviceName();

		virtual IParaAudioSource* create(const char* name, const char* filename, bool stream = false);

		virtual IParaAudioSource* createFromMemory(const char* name, const char* data, size_t length, const char* extension);

		virtual IParaAudioSource* createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, ParaAudioFormats format);

		virtual void SetDistanceModel(ParaAudioDistanceModelEnum eDistModel);

		virtual void setPosition(const PARAVECTOR3& pos);

		virtual void setDirection(const PARAVECTOR3& dir);

		virtual void setUpVector(const PARAVECTOR3& up);

		virtual void setVelocity(const PARAVECTOR3& vel);

		virtual void setMasterVolume(const float& volume);

		virtual void move(const PARAVECTOR3& pos);

		virtual PARAVECTOR3 getPosition(void) const;

		virtual PARAVECTOR3 getDirection(void) const;

		virtual PARAVECTOR3 getUpVector(void) const;

		virtual PARAVECTOR3 getVelocity(void) const;

		virtual float getMasterVolume(void) const;

		virtual void SetCoordinateSystem(int nLeftHand);

	protected:
		std::string GetTempFileName(const char* filename, bool* bIsFileExist = NULL);

		AudioMap_Type m_audio_source_map;
		std::map<std::string, int32> m_temporary_ids;
		int m_nLastFileID;
		std::string m_sTempAudioDirectory;
	};
}

#endif