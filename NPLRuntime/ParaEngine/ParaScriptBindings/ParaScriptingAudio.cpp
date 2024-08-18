//-----------------------------------------------------------------------------
// Class:	ParaAudio
// Authors:	LiXizhi
// Emails:	lixizhi@paraengine.com
// Company: ParaEngine Co.
// Date:	2010.6.27
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaScriptingAudio.h"
#include "AudioEngine2.h"
#include "util/MidiMsg.h"
#include "MCIController.h"

using namespace ParaEngine;
using namespace ParaScripting;

bool ParaAudio::IsAudioEngineEnabled()
{
	return CAudioEngine2::GetInstance()->IsAudioEngineEnabled();
}

void ParaAudio::EnableAudioEngine(bool bEnable)
{
	CAudioEngine2::GetInstance()->EnableAudioEngine(bEnable);
}

void ParaAudio::SetVolume(float fVolume)
{
	CAudioEngine2::GetInstance()->SetGlobalVolume(fVolume);
	CMidiMsg::GetSingleton().SetVolumeFloat(fVolume);
}

float ParaAudio::GetVolume()
{
	return CAudioEngine2::GetInstance()->GetGlobalVolume();
}

int ParaScripting::ParaAudio::PlayMidiMsg(DWORD dwMsg)
{
	return CMidiMsg::GetSingleton().PlayMidiMsg(dwMsg);
}

int ParaScripting::ParaAudio::StopMidiMsg(int channel)
{
#ifdef PARAENGINE_CLIENT
	return CMidiMsg::GetSingleton().StopMidiMsg(channel);
#else
	return 0;
#endif
}

bool ParaAudio::PlayWaveFile(const char* szWavePath, int nLoopCount)
{
	const std::string filename = szWavePath;
	if (CParaFile::GetFileExtension(filename) == "mid")
	{
		CMidiMsg::PlayMidiFile(filename);
	}
	else
	{
		// stop last midi sound
		if (filename.empty()) {
			CMidiMsg::PlayMidiFile(filename);
			return true;
		}

		if (FAILED(CAudioEngine2::GetInstance()->PlayWaveFile(szWavePath, nLoopCount > 0)))
		{
			return false;
		}
	}
	return true;
}

bool ParaAudio::PlayWaveFile1(const char* szWavePath)
{
	return PlayWaveFile(szWavePath, 0);
}

bool ParaAudio::StopWaveFile(const char* szWavePath, bool bImmediateStop)
{
	if (FAILED(CAudioEngine2::GetInstance()->StopWaveFile(szWavePath, bImmediateStop ? AUDIO_FLAG_STOP_IMMEDIATE : AUDIO_FLAG_STOP_RELEASE)))
	{
		return false;
	}
	return true;
}

bool ParaAudio::StopWaveFile1(const char* szWavePath)
{
	return StopWaveFile(szWavePath, true);
}

bool ParaAudio::ReleaseWaveFile(const char* szWavePath)
{
	if (FAILED(CAudioEngine2::GetInstance()->ReleaseWaveFile(szWavePath)))
	{
		return false;
	}
	return true;
}

ParaAudioSource ParaAudio::Create(const char* sName, const char* sWavePath, bool bStream)
{
	CAudioSource2_ptr ptr = CAudioEngine2::GetInstance()->Create(sName, sWavePath, bStream);
	return ParaAudioSource(ptr);
}

ParaAudioSource ParaAudio::Get(const char* sName)
{
	CAudioSource2_ptr ptr = CAudioEngine2::GetInstance()->Get(sName);
	return ParaAudioSource(ptr);
}

ParaAudioSource ParaAudio::CreateGet(const char* sName, const char* sWavePath, bool bStream)
{
	CAudioSource2_ptr ptr = CAudioEngine2::GetInstance()->CreateGet(sName, sWavePath, bStream);
	return ParaAudioSource(ptr);
}

void ParaScripting::ParaAudio::SetDistanceModel(int eDistModel)
{
	return CAudioEngine2::GetInstance()->SetDistanceModel((ParaAudioDistanceModelEnum)eDistModel);
}

bool ParaScripting::ParaAudio::StartRecording()
{
	auto pAutoCapture = CAudioEngine2::GetInstance()->CreateGetAudioCapture();
	if (pAutoCapture)
	{
		return pAutoCapture->beginCapture();
	}
	return false;
}

const std::string& ParaScripting::ParaAudio::GetCapturedAudio(int nMaxSize)
{
	static std::string data;
	auto pAutoCapture = CAudioEngine2::GetInstance()->CreateGetAudioCapture();
	if (pAutoCapture)
	{
		auto nSize = pAutoCapture->getCurrentCapturedAudioSize();
		if (nSize > 0)
		{
			nSize = (nMaxSize > 0 && nSize > nMaxSize) ? nMaxSize : nSize;
			data.resize(nSize);
			int nReadSize = pAutoCapture->getCapturedAudio(&(data[0]), nSize);
			return data;
		}
	}
	data.clear();
	return data;
}

void ParaScripting::ParaAudio::StopRecording()
{
	auto pAutoCapture = CAudioEngine2::GetInstance()->CreateGetAudioCapture();
	if (pAutoCapture)
	{
		pAutoCapture->stopCapture();
	}
}

bool ParaScripting::ParaAudio::SaveRecording(const char* szWavePath)
{
	auto pAutoCapture = CAudioEngine2::GetInstance()->CreateGetAudioCapture();
	if (pAutoCapture && szWavePath)
	{
		auto sFileExtension = CParaFile::GetFileExtension(szWavePath);
		if (sFileExtension == "ogg" || sFileExtension == "wav")
		{
			return pAutoCapture->saveToFile(szWavePath, CAudioEngine2::GetInstance()->GetCaptureAudioQuality()) != 0;
		}
		else
		{
			auto nSize = pAutoCapture->getCurrentCapturedAudioSize();
			if (nSize > 0)
			{
				std::vector<char> data;
				data.resize(nSize);
				if (pAutoCapture->getCapturedAudio(&(data[0]), nSize) == nSize)
				{
					bool bSucceed = false;
					CParaFile file;
					if (file.OpenFile(szWavePath, false))
					{
						file.WriteString(&(data[0]), nSize);
						file.close();
						bSucceed = true;
					}
					return bSucceed;
				}
			}
		}
	}
	return false;
}

bool ParaScripting::ParaAudio::SaveRecording2(const char* szWavePath, const char* pBuffer, int nSize)
{
	auto pAutoCapture = CAudioEngine2::GetInstance()->CreateGetAudioCapture();
	if (pAutoCapture && szWavePath)
	{
		auto sFileExtension = CParaFile::GetFileExtension(szWavePath);
		if (sFileExtension == "ogg" || sFileExtension == "wav")
		{
			return pAutoCapture->saveToFile(szWavePath, pBuffer, nSize, CAudioEngine2::GetInstance()->GetCaptureAudioQuality()) != 0;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////
//
// Audio Source Functions
//
//////////////////////////////////////////////////////////////////////////

bool ParaScripting::ParaAudioSource::IsValid() const
{
	return (m_pAudioSource && m_pAudioSource->m_pSource);
}


void ParaScripting::ParaAudioSource::release()
{
	CAudioEngine2::GetInstance()->release(m_pAudioSource);
}


const char* ParaScripting::ParaAudioSource::GetName()
{
	if (m_pAudioSource)
	{
		return m_pAudioSource->GetName().c_str();
	}
	return CGlobals::GetString(0).c_str();
}

bool ParaScripting::ParaAudioSource::play()
{
	if (m_pAudioSource)
	{
		return m_pAudioSource->play();
	}
	return false;
}

bool ParaScripting::ParaAudioSource::play2d(bool toLoop)
{
	if (m_pAudioSource)
	{
		return m_pAudioSource->play2d(toLoop);
	}
	return false;
}

bool ParaScripting::ParaAudioSource::play3d(float x, float y, float z, float soundstr, bool toLoop)
{
	if (m_pAudioSource)
	{
		return m_pAudioSource->play3d(PARAVECTOR3(x, y, z), soundstr, toLoop);
	}
	return false;
}

void ParaScripting::ParaAudioSource::pause()
{
	if (m_pAudioSource)
	{
		m_pAudioSource->pause();
	}
}

void ParaScripting::ParaAudioSource::stop()
{
	if (m_pAudioSource)
	{
		m_pAudioSource->stop();
	}
}

void ParaScripting::ParaAudioSource::loop(bool toLoop)
{
	if (m_pAudioSource)
	{
		m_pAudioSource->loop(toLoop);
	}
}

bool ParaScripting::ParaAudioSource::seek(float seconds, bool relative)
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->seek(seconds, relative);
	}
	return false;
}

float ParaScripting::ParaAudioSource::getTotalAudioTime()
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getTotalAudioTime();
	}
	return 0.f;
}

int ParaScripting::ParaAudioSource::getTotalAudioSize()
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getTotalAudioSize();
	}
	return 0;
}

float ParaScripting::ParaAudioSource::getCurrentAudioTime()
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getCurrentAudioTime();
	}
	return 0.f;
}

int ParaScripting::ParaAudioSource::getCurrentAudioPosition()
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getCurrentAudioPosition();
	}
	return 0;
}

bool ParaScripting::ParaAudioSource::isValid() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->isValid();
	}
	return false;
}

bool ParaScripting::ParaAudioSource::isPlaying() const
{
	if (m_pAudioSource)
	{
		return m_pAudioSource->IsPlaying();
	}
	return false;
}

bool ParaScripting::ParaAudioSource::isPaused() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->isPaused();
	}
	return true;
}

bool ParaScripting::ParaAudioSource::isStopped() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->isStopped();
	}
	return true;
}

bool ParaScripting::ParaAudioSource::isLooping() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->isLooping();
	}
	return false;
}

void ParaScripting::ParaAudioSource::setPosition(float x, float y, float z)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setPosition(PARAVECTOR3(x, y, z));
	}
}

void ParaScripting::ParaAudioSource::setVelocity(float x, float y, float z)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setVelocity(PARAVECTOR3(x, y, z));
	}
}

void ParaScripting::ParaAudioSource::setDirection(float x, float y, float z)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setDirection(PARAVECTOR3(x, y, z));
	}
}

void ParaScripting::ParaAudioSource::setRolloffFactor(float rolloff)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setRolloffFactor(rolloff);
	}
}

void ParaScripting::ParaAudioSource::setStrength(float soundstrength)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setStrength(soundstrength);
	}
}

void ParaScripting::ParaAudioSource::setMinDistance(float minDistance)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setMinDistance(minDistance);
	}
}

void ParaScripting::ParaAudioSource::setMaxDistance(float maxDistance)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setMaxDistance(maxDistance);
	}
}

void ParaScripting::ParaAudioSource::setPitch(float pitch)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setPitch(pitch);
	}
}

void ParaScripting::ParaAudioSource::setVolume(float volume)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setVolume(volume);
	}
}

void ParaScripting::ParaAudioSource::setMinVolume(float minVolume)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setMinVolume(minVolume);
	}
}

void ParaScripting::ParaAudioSource::setMaxVolume(float maxVolume)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setMaxVolume(maxVolume);
	}
}

void ParaScripting::ParaAudioSource::setInnerConeAngle(float innerAngle)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setInnerConeAngle(innerAngle);
	}
}

void ParaScripting::ParaAudioSource::setOuterConeAngle(float outerAngle)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setOuterConeAngle(outerAngle);
	}
}

void ParaScripting::ParaAudioSource::setOuterConeVolume(float outerVolume)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->setOuterConeVolume(outerVolume);
	}
}

void ParaScripting::ParaAudioSource::move(float x, float y, float z)
{
	if (IsValid())
	{
		m_pAudioSource->m_pSource->move(PARAVECTOR3(x, y, z));
	}
}

void ParaScripting::ParaAudioSource::getPosition(float& x, float& y, float& z) const
{
	if (IsValid())
	{
		PARAVECTOR3 v = m_pAudioSource->m_pSource->getPosition();
		x = v.x;
		y = v.y;
		z = v.z;
	}
}

void ParaScripting::ParaAudioSource::getVelocity(float& x, float& y, float& z) const
{
	if (IsValid())
	{
		PARAVECTOR3 v = m_pAudioSource->m_pSource->getVelocity();
		x = v.x;
		y = v.y;
		z = v.z;
	}
}

void ParaScripting::ParaAudioSource::getDirection(float& x, float& y, float& z) const
{
	if (IsValid())
	{
		PARAVECTOR3 v = m_pAudioSource->m_pSource->getDirection();
		x = v.x;
		y = v.y;
		z = v.z;
	}
}

float ParaScripting::ParaAudioSource::getRolloffFactor() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getRolloffFactor();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getStrength() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getStrength();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getMinDistance() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getMinDistance();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getMaxDistance() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getMaxDistance();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getPitch() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getPitch();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getVolume() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getVolume();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getMinVolume() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getMinVolume();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getMaxVolume() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getMaxVolume();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getInnerConeAngle() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getInnerConeAngle();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getOuterConeAngle() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getOuterConeAngle();
	}
	return 0.f;
}

float ParaScripting::ParaAudioSource::getOuterConeVolume() const
{
	if (IsValid())
	{
		return m_pAudioSource->m_pSource->getOuterConeVolume();
	}
	return 0.f;
}
