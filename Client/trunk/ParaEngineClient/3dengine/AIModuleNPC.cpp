#include "ParaEngine.h"
#include "BipedObject.h"
#include "BipedController.h"
#include "MovieCtrlers.h"
#include "SceneObject.h"
#include "SequenceCtler.h"
#include "FaceTrackingCtrler.h"

#include "AIModuleNPC.h"

using namespace ParaEngine;


CAIModuleNPC::CAIModuleNPC(CBipedObject* pBiped)
	:m_pFaceTrackingCtrler(NULL), 
	m_pFollowCtrler(NULL), 
	m_pMovieCtrler(NULL),m_pSequenceCtrler(NULL),
	CAIBase(pBiped)
{
}

CAIModuleNPC::CAIModuleNPC(void)
	:m_pFaceTrackingCtrler(NULL), 
	m_pFollowCtrler(NULL), 
	m_pMovieCtrler(NULL),m_pSequenceCtrler(NULL)
{
}

CAIModuleNPC::~CAIModuleNPC(void)
{
	SAFE_DELETE(m_pFaceTrackingCtrler);
	SAFE_DELETE(m_pFollowCtrler);
	SAFE_DELETE(m_pMovieCtrler);
	SAFE_DELETE(m_pSequenceCtrler);
}

void CAIModuleNPC::EnableFaceTracking(bool bFaceTracking)
{
	if(!m_pFaceTrackingCtrler && bFaceTracking)
	{
		m_pFaceTrackingCtrler = new CFaceTrackingCtrler(this);
	}
	if(m_pFaceTrackingCtrler)
	{
		if(bFaceTracking)
			m_pFaceTrackingCtrler->Resume();
		else
			m_pFaceTrackingCtrler->Suspend();
	}
}
bool CAIModuleNPC::IsFaceTrackingEnabled()
{
	return (m_pFaceTrackingCtrler!=0 && m_pFaceTrackingCtrler->IsActive());
}

void CAIModuleNPC::FollowBiped(const std::string& sName)
{
	if(!m_pFollowCtrler && sName != "")
	{
		m_pFollowCtrler = new CFollowCtrler(this);
	}
	if(m_pFollowCtrler)
	{
		if(sName == "")
			m_pFollowCtrler->Suspend();
		else
		{
			m_pFollowCtrler->SetFollowTarget(sName);
			m_pFollowCtrler->Resume();
		}
	}
}

bool CAIModuleNPC::IsFollowEnabled()
{
	return (m_pFollowCtrler!=0 && m_pFollowCtrler->IsActive());
}

void CAIModuleNPC::EnableMovieControler(bool bEnable)
{
	if(!m_pMovieCtrler && bEnable)
	{
		m_pMovieCtrler = new CMovieCtrler(this);
	}
	if(m_pMovieCtrler)
	{
		if(!bEnable)
			m_pMovieCtrler->Suspend();
	}
}

bool CAIModuleNPC::IsMovieControlerEnabled()
{
	return (m_pMovieCtrler!=0 && m_pMovieCtrler->IsActive());
}

CMovieCtrler* CAIModuleNPC::GetMovieController()
{
	return m_pMovieCtrler;
}

void CAIModuleNPC::EnableSequenceControler(bool bEnable)
{
	if(!m_pSequenceCtrler && bEnable)
	{
		m_pSequenceCtrler = new CSequenceCtler(this);
	}
	if(m_pSequenceCtrler)
	{
		if(!bEnable)
			m_pSequenceCtrler->Suspend();
	}
}
CSequenceCtler* CAIModuleNPC::GetSequenceController()
{
	return m_pSequenceCtrler;
}

bool CAIModuleNPC::IsSequenceControlerEnabled()
{
	return (m_pSequenceCtrler!=0 && m_pSequenceCtrler->IsActive());
}

void CAIModuleNPC::FrameMove(float fDeltaTime)
{
	if(m_pMovieCtrler && m_pMovieCtrler->IsActive())
	{
		m_pMovieCtrler->FrameMove(fDeltaTime);
		return; // ignore all other controller
	}
	if(m_pFaceTrackingCtrler && m_pFaceTrackingCtrler->IsActive())
	{
		m_pFaceTrackingCtrler->FrameMove(fDeltaTime);
	}

	if(m_pFollowCtrler && m_pFollowCtrler->IsActive())
		m_pFollowCtrler->FrameMove(fDeltaTime);
	else if(m_pSequenceCtrler && m_pSequenceCtrler->IsActive())
	{
		m_pSequenceCtrler->FrameMove(fDeltaTime);
	}
}

CFaceTrackingCtrler* CAIModuleNPC::GetFaceController()
{
	return m_pFaceTrackingCtrler;
}

CFollowCtrler* CAIModuleNPC::GetFollowController()
{
	return m_pFollowCtrler;
}