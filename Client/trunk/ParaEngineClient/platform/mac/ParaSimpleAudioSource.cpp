//-----------------------------------------------------------------------------
// Class: Simple Audio Engine
// Authors:	LiXizhi
// Company:	ParaEngine
// Date: 2014.10.14
// Desc: just a simple wrapper to cocos' simple audio engine.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifndef STATIC_PLUGIN_CAUDIOENGINE
//#include "cocos2d.h"
#include "ParaSimpleAudioEngine.h"
#include "ParaSimpleAudioSource.h"
#include "SimpleAudioEngine.h"
//using namespace CocosDenshion;
//USING_NS_CC;
using namespace ParaEngine;

ParaEngine::CParaSimpleAudioSource::CParaSimpleAudioSource(const char* sFileName, unsigned int pSource)
	: m_pSource(pSource), m_bLoop(false), m_bPlaying(false), m_nCurTick(0), m_nAudioFileState(AudioFileState_Unknown)
{
	if (sFileName != NULL)
		m_sFilename = sFileName;
}

ParaEngine::CParaSimpleAudioSource::~CParaSimpleAudioSource()
{
	release();
}

unsigned int ParaEngine::CParaSimpleAudioSource::GetSourcePointer()
{
	return m_pSource;
}

const std::string& ParaEngine::CParaSimpleAudioSource::GetFileName()
{
	return m_sFilename;
}

bool ParaEngine::CParaSimpleAudioSource::play()
{
	return play2d(m_bLoop);
}

bool ParaEngine::CParaSimpleAudioSource::play2d(const bool& toLoop /*= false*/)
{
	m_bLoop = toLoop;
	if (!m_bLoop)
	{

		m_pSource = SimpleAudioEngine::getInstance()->playEffect(GetFileName().c_str(), m_bLoop);
		//OUTPUT_LOG("playing audio %s id : %d\n", GetFileName().c_str(), (unsigned int)m_pSource);
	}
	else
	{
		// loop music is always a background music
		m_pSource = 0;
		SimpleAudioEngine::getInstance()->playBackgroundMusic(GetFileName().c_str(), m_bLoop);
		// SimpleAudioEngine::getInstance()->rewindBackgroundMusic();
	}

	m_bPlaying = true;
	m_nCurTick = 0;
	return true;
}

bool ParaEngine::CParaSimpleAudioSource::play3d(const PARAVECTOR3& position, const float& soundstr /*= 1.0*/, const bool& toLoop /*= false*/)
{
	m_bLoop = toLoop;

	return true;
}

void ParaEngine::CParaSimpleAudioSource::pause()
{
	stop();
}

void ParaEngine::CParaSimpleAudioSource::stop()
{
	if (m_pSource != 0)
	{
		SimpleAudioEngine::getInstance()->pauseEffect(m_pSource);
	}
	else
	{
		if (IsBackgroundMusic())
		{
			SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		}
	}
	m_bPlaying = false;
}

void ParaEngine::CParaSimpleAudioSource::loop(const bool& toLoop)
{
	m_bLoop = toLoop;
}

bool ParaEngine::CParaSimpleAudioSource::seek(const float& seconds, bool relative /*= false*/)
{
	if (!relative && seconds == 0.f)
	{
		if (IsBackgroundMusic())
		{
			if (isPlaying())
			{
				SimpleAudioEngine::getInstance()->rewindBackgroundMusic();
			}

		}
	}
	return true;
}

float ParaEngine::CParaSimpleAudioSource::getTotalAudioTime()
{
	return 0.f;
}

int ParaEngine::CParaSimpleAudioSource::getTotalAudioSize()
{
	return 0;
}

int ParaEngine::CParaSimpleAudioSource::getCompressedAudioSize()
{
	return 0;
}

float ParaEngine::CParaSimpleAudioSource::getCurrentAudioTime()
{
	return 0.f;
}

int ParaEngine::CParaSimpleAudioSource::getCurrentAudioPosition()
{
	return 0;
}

int ParaEngine::CParaSimpleAudioSource::getCurrentCompressedAudioPosition()
{
	return 0;
}

bool ParaEngine::CParaSimpleAudioSource::update()
{
	if (m_bPlaying)
	{
		m_nCurTick++;
		if (!IsBackgroundMusic())
		{
			if (m_pSource == 0 && (m_nCurTick % 10) == 0)
			{
				// just in case the audio is not preloaded well.
				m_pSource = SimpleAudioEngine::getInstance()->playEffect(GetFileName().c_str(), m_bLoop);

				if (m_pSource != 0)
					m_nCurTick = 0;
				else
				{
					// try 2 seconds, if still not loaded, stop the sound
					if (m_nCurTick > 60)
					{
						m_bPlaying = false;
					}
					return true;
				}
			}

			// since cocos' API can not query or get sound length,
			// we will assume a sound effect have 30 ticks(1 second) for non-looping files.
			if (m_nCurTick > 30)
			{
				m_bPlaying = false;
			}
		}
	}
	else
		m_nCurTick = 0;
	return true;
}

void ParaEngine::CParaSimpleAudioSource::release()
{
	// never unload in mobile version.
	/*if (m_pSource)
		SimpleAudioEngine::getInstance()->unloadEffect(GetFileName().c_str());*/
	m_bPlaying = false;
}

const bool ParaEngine::CParaSimpleAudioSource::isValid() const
{
	return true;
}

const bool ParaEngine::CParaSimpleAudioSource::isPlaying() const
{
	return m_bPlaying;
}

const bool ParaEngine::CParaSimpleAudioSource::isPaused() const
{
	return !m_bPlaying;
}

const bool ParaEngine::CParaSimpleAudioSource::isStopped() const
{
	return !m_bPlaying;
}

const bool ParaEngine::CParaSimpleAudioSource::isLooping() const
{
	return m_bLoop;
}

void ParaEngine::CParaSimpleAudioSource::setPosition(const PARAVECTOR3& position)
{

}

void ParaEngine::CParaSimpleAudioSource::setVelocity(const PARAVECTOR3& velocity)
{

}

void ParaEngine::CParaSimpleAudioSource::setDirection(const PARAVECTOR3& direction)
{

}

void ParaEngine::CParaSimpleAudioSource::setRolloffFactor(const float& rolloff)
{

}

void ParaEngine::CParaSimpleAudioSource::setStrength(const float& soundstrength)
{

}

void ParaEngine::CParaSimpleAudioSource::setMinDistance(const float& minDistance)
{

}

void ParaEngine::CParaSimpleAudioSource::setMaxDistance(const float& maxDistance)
{

}

void ParaEngine::CParaSimpleAudioSource::setPitch(const float& pitch)
{

}

void ParaEngine::CParaSimpleAudioSource::setVolume(const float& volume)
{

}

void ParaEngine::CParaSimpleAudioSource::setMinVolume(const float& minVolume)
{

}

void ParaEngine::CParaSimpleAudioSource::setMaxVolume(const float& maxVolume)
{

}

void ParaEngine::CParaSimpleAudioSource::setInnerConeAngle(const float& innerAngle)
{

}

void ParaEngine::CParaSimpleAudioSource::setOuterConeAngle(const float& outerAngle)
{

}

void ParaEngine::CParaSimpleAudioSource::setOuterConeVolume(const float& outerVolume)
{

}

void ParaEngine::CParaSimpleAudioSource::setDopplerStrength(const float& dstrength)
{

}

void ParaEngine::CParaSimpleAudioSource::setDopplerVelocity(const PARAVECTOR3& dvelocity)
{

}

void ParaEngine::CParaSimpleAudioSource::move(const PARAVECTOR3& position)
{

}

const PARAVECTOR3 ParaEngine::CParaSimpleAudioSource::getPosition() const
{
	return PARAVECTOR3(0, 0, 0);
}

const PARAVECTOR3 ParaEngine::CParaSimpleAudioSource::getVelocity() const
{
	return PARAVECTOR3(0, 0, 0);
}

const PARAVECTOR3 ParaEngine::CParaSimpleAudioSource::getDirection() const
{
	return PARAVECTOR3(0, 0, 0);
}

const float ParaEngine::CParaSimpleAudioSource::getRolloffFactor() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getStrength() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getMinDistance() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getMaxDistance() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getPitch() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getVolume() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getMinVolume() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getMaxVolume() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getInnerConeAngle() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getOuterConeAngle() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getOuterConeVolume() const
{
	return 0.f;
}

const float ParaEngine::CParaSimpleAudioSource::getDopplerStrength() const
{
	return 0.f;
}

const PARAVECTOR3 ParaEngine::CParaSimpleAudioSource::getDopplerVelocity() const
{
	return PARAVECTOR3(0, 0, 0);
}

void ParaEngine::CParaSimpleAudioSource::registerEventHandler(IAudioSourceEventHandler* handler)
{

}

void ParaEngine::CParaSimpleAudioSource::unRegisterAllEventHandlers()
{

}

void ParaEngine::CParaSimpleAudioSource::SetAudioFileState(CParaSimpleAudioSource::AudioFileState val)
{
	m_nAudioFileState = val;
}

CParaSimpleAudioSource::AudioFileState ParaEngine::CParaSimpleAudioSource::GetAudioFileState() const
{
	return m_nAudioFileState;
}
#endif
