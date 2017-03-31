#pragma once
#include "IParaAudioEngine.h"

namespace ParaEngine
{
	/** represents a single audio file being played or loaded. */
	class CParaSimpleAudioSource : public IParaAudioSource
	{
	public:
		enum AudioFileState
		{
			/** asset is always created in normal state. */
			AudioFileState_Ready,
			AudioFileState_LoadingFromDisk,
			AudioFileState_NotFound,
			AudioFileState_Unknown,
		};

		CParaSimpleAudioSource(const char* sFileName=NULL, unsigned int pSource = 0);
		virtual ~CParaSimpleAudioSource();

	public:
		unsigned int GetSourcePointer();
		const std::string& GetFileName();

		CParaSimpleAudioSource::AudioFileState GetAudioFileState() const;
		void SetAudioFileState(CParaSimpleAudioSource::AudioFileState val);

	public:

		virtual bool play();

		virtual bool play2d(const bool& toLoop = false);

		virtual bool play3d(const PARAVECTOR3& position, const float& soundstr = 1.0, const bool& toLoop = false);

		virtual void pause();

		virtual void stop();

		virtual void loop(const bool& toLoop);

		virtual bool seek(const float& seconds, bool relative = false);

		virtual float getTotalAudioTime();

		virtual int getTotalAudioSize();

		virtual int getCompressedAudioSize();

		virtual float getCurrentAudioTime();

		virtual int getCurrentAudioPosition();

		virtual int getCurrentCompressedAudioPosition();

		virtual bool update();

		virtual void release();

		virtual const bool isValid() const;

		virtual const bool isPlaying() const;

		virtual const bool isPaused() const;

		virtual const bool isStopped() const;

		virtual const bool isLooping() const;

		virtual void setPosition(const PARAVECTOR3& position);

		virtual void setVelocity(const PARAVECTOR3& velocity);

		virtual void setDirection(const PARAVECTOR3& direction);

		virtual void setRolloffFactor(const float& rolloff);

		virtual void setStrength(const float& soundstrength);

		virtual void setMinDistance(const float& minDistance);

		virtual void setMaxDistance(const float& maxDistance);

		virtual void setPitch(const float& pitch);

		virtual void setVolume(const float& volume);

		virtual void setMinVolume(const float& minVolume);

		virtual void setMaxVolume(const float& maxVolume);

		virtual void setInnerConeAngle(const float& innerAngle);

		virtual void setOuterConeAngle(const float& outerAngle);

		virtual void setOuterConeVolume(const float& outerVolume);

		virtual void setDopplerStrength(const float& dstrength);

		virtual void setDopplerVelocity(const PARAVECTOR3& dvelocity);

		virtual void move(const PARAVECTOR3& position);

		virtual const PARAVECTOR3 getPosition() const;

		virtual const PARAVECTOR3 getVelocity() const;

		virtual const PARAVECTOR3 getDirection() const;

		virtual const float getRolloffFactor() const;

		virtual const float getStrength() const;

		virtual const float getMinDistance() const;

		virtual const float getMaxDistance() const;

		virtual const float getPitch() const;

		virtual const float getVolume() const;

		virtual const float getMinVolume() const;

		virtual const float getMaxVolume() const;

		virtual const float getInnerConeAngle() const;

		virtual const float getOuterConeAngle() const;

		virtual const float getOuterConeVolume() const;

		virtual const float getDopplerStrength() const;

		virtual const PARAVECTOR3 getDopplerVelocity() const;

		virtual void registerEventHandler(IAudioSourceEventHandler* handler);

		virtual void unRegisterAllEventHandlers();
	public:
		/** only one bg music at a time. we will regard any looping audio as background music. */
		inline bool IsBackgroundMusic(){
			return m_bLoop;
		}
	protected:
		// the OpenAL source id
		unsigned int m_pSource;
		std::string m_sFilename;
		bool m_bLoop;
		bool m_bPlaying;
		uint32 m_nCurTick;

		AudioFileState m_nAudioFileState;
		
	};

}