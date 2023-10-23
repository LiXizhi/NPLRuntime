//-----------------------------------------------------------------------------
// Class:	CFrameRateController
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.3.21
// Desc: The class is used to generate smooth framerate

#include "ParaEngine.h"
#include "FrameRateController.h"

using namespace ParaEngine;

namespace ParaEngine
{
	/**
	define the frame controller for the rendering, IO and simulation modules.
	The best combination after experimentation is below
	rendering : constant
	IO : constant
	simulation : first_order
	Another acceptable configuration is:
	rendering : constant
	IO : first_order
	simulation : first_order
	*/
	CFrameRateController g_renderFRC(CFrameRateController::FRC_CONSTANT, "renderFRC");
	CFrameRateController g_ioFRC(CFrameRateController::FRC_CONSTANT, "ioFRC");
	CFrameRateController g_simFRC(CFrameRateController::FRC_FIRSTORDER, "simFRC");
	CFrameRateController g_gameTime(CFrameRateController::FRC_NONE, "gameFRC");
}

/// load the normal frame rate control configuration
void CFrameRateController::LoadFRCNormal(float fIdealInterval)
{
#ifdef NO_FRAME_RATE_CONTROL
	g_renderFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_ABOVE);
	g_ioFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_ABOVE);
	g_simFRC.SetType(CFrameRateController::FRC_FIRSTORDER);
#else
#ifdef REALTIME_FRAME_RATE_CONTROL
	g_ioFRC.SetType(CFrameRateController::FRC_NONE);
	g_renderFRC.SetType(CFrameRateController::FRC_NONE);
	g_simFRC.SetType(CFrameRateController::FRC_NONE);
#else
	g_renderFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_BELOW); // specify FRC_CONSTANT will play slow motion under low frame rate if one want
	g_ioFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_BELOW); // specify FRC_CONSTANT will play slow motion under low frame rate if one want
	g_simFRC.SetType(CFrameRateController::FRC_BELOW); // smooth the simulation one can also use FRC_FIRSTORDER
#endif
#endif
	fIdealInterval = fIdealInterval <= 0 ? IDEAL_FRAME_RATE : fIdealInterval;
	g_renderFRC.m_fConstDeltaTime = fIdealInterval;
	g_ioFRC.m_fConstDeltaTime = fIdealInterval;
	g_simFRC.m_fConstDeltaTime = fIdealInterval;

	g_renderFRC.m_fMinDeltaTime = 1 / 60.f;
	g_ioFRC.m_fMinDeltaTime = 1 / 60.f;
	g_simFRC.m_fMinDeltaTime = 1 / 60.f;
}

/// Real time frame rate control
void CFrameRateController::LoadFRCRealtime(float fIdealInterval)
{
	//g_renderFRC.SetType(CFrameRateController::FRC_CONSTANT_OR_BELOW);
	//g_renderFRC.SetType(CFrameRateController::FRC_CONSTANT);
	g_renderFRC.SetType(CFrameRateController::FRC_NONE);

	g_ioFRC.SetType(CFrameRateController::FRC_NONE);
	g_simFRC.SetType(CFrameRateController::FRC_NONE);
	// g_simFRC.SetType(CFrameRateController::FRC_BELOW);

	fIdealInterval = fIdealInterval <= 0 ? IDEAL_FRAME_RATE : fIdealInterval;
	g_renderFRC.m_fConstDeltaTime = fIdealInterval;
	g_ioFRC.m_fConstDeltaTime = fIdealInterval;
	g_simFRC.m_fConstDeltaTime = fIdealInterval;

	g_renderFRC.m_fMinDeltaTime = 1 / 60.f;
	g_ioFRC.m_fMinDeltaTime = 1 / 60.f;
	g_simFRC.m_fMinDeltaTime = 1 / 60.f;
}

void CFrameRateController::LoadFRCServerMode()
{
	LoadFRCNormal();

	g_simFRC.m_fMinDeltaTime = 1 / 30.f;
	g_simFRC.m_fMaxDeltaTime = 0.99f;
	g_simFRC.SetType(CFrameRateController::FRC_NONE);
}

void CFrameRateController::LoadFRCCapture(int nFPS)
{
	g_renderFRC.SetType(CFrameRateController::FRC_CONSTANT);
	g_ioFRC.SetType(CFrameRateController::FRC_CONSTANT);
	g_simFRC.SetType(CFrameRateController::FRC_CONSTANT);

	g_renderFRC.m_fConstDeltaTime = 1.f / nFPS;
	g_renderFRC.m_fMinDeltaTime = IDEAL_FRAME_RATE;
	g_ioFRC.m_fConstDeltaTime = 1.f / nFPS;
	g_ioFRC.m_fMinDeltaTime = IDEAL_FRAME_RATE;
	g_simFRC.m_fConstDeltaTime = 1.f / nFPS;
	g_simFRC.m_fMinDeltaTime = IDEAL_FRAME_RATE;
}

/************************************************************************/
/* CFrameRateController class                                           */
/************************************************************************/
CFrameRateController::CFrameRateController(ControllerType type, const char* sName)
	:m_bPaused(false)
{
	SetType(type);
	if (sName)
		SetIdentifier(sName);

	m_fConstDeltaTime = IDEAL_FRAME_RATE;		// 30 FPS is the ideal frame rate
	m_fMaxDeltaTime = 0.99f;				// over 1 second lag is truncated to 1.
	m_fMinDeltaTime = 1 / 60.0f;			// 60 FPS is the highest frame rate we allow
	m_fLinearSlope = 0.2f / 30.0f;		// sloop = m_fConstDeltaTime/5, use 5 frames to catch up
	m_fMaxLinearDeltaTime = 1 / 10.0f;	// use linear interpolation only if frame rate is over 10 FPS

	m_fTime = 0;
	m_fLastTime = 0;
	m_fElapsedTime = 0;
	m_fLastElapsedTime = 0;
	m_fNextTime = 0;
}

CFrameRateController::CFrameRateController(void)
	:m_bPaused(false)
{
	SetType(FRC_CONSTANT);
	m_fConstDeltaTime = IDEAL_FRAME_RATE;		// 30 FPS is the ideal frame rate
}

CFrameRateController::~CFrameRateController(void)
{
}

const std::string& CFrameRateController::GetIdentifier()
{
	return m_sIdentifier;
}

void CFrameRateController::SetIdentifier(const std::string& sID)
{
	m_sIdentifier = sID;
}

void CFrameRateController::SetType(ControllerType nType)
{
	m_nType = nType;
}

int CFrameRateController::GetTime()
{
	return (int)(m_fTime * 1000);
}

void CFrameRateController::SetTime(int nTime)
{
	m_fTime = nTime / 1000.0;
}

/** call this function to advance using delta time
return the desired elapsed time
*/
double CFrameRateController::FrameMoveDelta(double deltatime)
{
	return FrameMove(m_fLastTime + deltatime);
}

/** call this function to advance using absolute time,
return the desired elapsed time
*/
double CFrameRateController::FrameMove(double time)
{
	if (!IsPaused())
	{
		if (m_fTime >= time)
			return 0.f;
		m_fTime = time;
	}

	switch (m_nType)
	{
	case FRC_NONE:
	{
		m_fElapsedTime = m_fTime - m_fLastTime;

		m_fLastTime = m_fTime;
		m_fLastElapsedTime = m_fElapsedTime;

		if (m_fElapsedTime > m_fMaxDeltaTime)
			m_fElapsedTime = m_fMaxDeltaTime;
		break;
	}
	case FRC_CONSTANT_OR_ABOVE:
	{
		m_fElapsedTime = m_fTime - m_fLastTime;
		if (m_fElapsedTime > m_fConstDeltaTime)
		{
			m_fElapsedTime = m_fConstDeltaTime;
		}

		m_fLastTime = m_fTime;
		m_fLastElapsedTime = m_fElapsedTime;
		break;
	}
	case FRC_CONSTANT_OR_BELOW:
	{
		m_fElapsedTime = m_fTime - m_fLastTime;
		if (m_fElapsedTime <= 0)
		{
			m_fElapsedTime = 0.f;
		}
		else if (m_fElapsedTime >= m_fConstDeltaTime)
		{
			m_fNextTime = m_fNextTime + m_fConstDeltaTime;
			if (m_fElapsedTime < (m_fNextTime - m_fLastTime))
			{
				m_fElapsedTime = m_fConstDeltaTime;
				m_fLastTime = m_fNextTime;
			}
			else
			{
				m_fNextTime = m_fTime + m_fConstDeltaTime;
				m_fLastTime = m_fNextTime;
				if (m_fElapsedTime > m_fMaxDeltaTime)
					m_fElapsedTime = m_fMaxDeltaTime;
			}
		}
		else
		{
			m_fElapsedTime = m_fConstDeltaTime;
			m_fNextTime = m_fNextTime + m_fConstDeltaTime;
			m_fLastTime = m_fNextTime;
		}
		break;
	}
	case FRC_BELOW:
	{
		// using real time but below a given frame rate. 
		m_fElapsedTime = m_fTime - m_fLastTime;
		if (m_fLastTime <= m_fNextTime)
		{
			if (m_fTime > m_fNextTime)
			{
				m_fNextTime = m_fNextTime + m_fConstDeltaTime;
				if (m_fTime >= m_fNextTime)
					m_fNextTime = m_fTime;
				m_fLastTime = m_fTime;
				if (m_fElapsedTime > m_fMaxDeltaTime)
					m_fElapsedTime = m_fMaxDeltaTime;

			}
			else
			{
				m_fElapsedTime = 0.f;
			}
		}
		else
		{
			m_fNextTime = m_fTime + m_fConstDeltaTime;
			m_fLastTime = m_fTime;
			if (m_fElapsedTime > m_fMaxDeltaTime)
				m_fElapsedTime = m_fMaxDeltaTime;
		}
		break;
	}
	case FRC_CONSTANT:
	{
		if (m_fMinDeltaTime >= (m_fTime - m_fLastTime))
			return 0.0f;
		m_fElapsedTime = m_fConstDeltaTime;
		m_fLastTime = m_fTime;
		m_fLastElapsedTime = m_fElapsedTime;
		break;
	}
	case FRC_FIRSTORDER:
	{
		m_fElapsedTime = m_fTime - m_fLastTime;
		if (fabs(m_fLastElapsedTime - m_fElapsedTime) <= m_fLinearSlope)
		{

		}
		else if (m_fLastElapsedTime<m_fElapsedTime)
		{
			m_fElapsedTime = m_fLastElapsedTime + m_fLinearSlope;
		}
		else if (m_fLastElapsedTime>m_fElapsedTime)
		{
			m_fElapsedTime = m_fLastElapsedTime - m_fLinearSlope;
		}
		if (m_fElapsedTime > m_fMaxLinearDeltaTime)
		{ // restore to normal 
			m_fElapsedTime = m_fTime - m_fLastTime;
			m_fLastElapsedTime = m_fConstDeltaTime;
			m_fLastTime = m_fTime;
		}
		else
		{
			m_fLastTime += m_fElapsedTime;
			m_fLastElapsedTime = m_fElapsedTime;
		}
		break;
	}
	default:
	{
		return m_fConstDeltaTime;
	}
	}

	return m_fElapsedTime;
}

void CFrameRateController::SetConstDeltaTime(double fConstDeltaTime)
{
	m_fConstDeltaTime = (float)fConstDeltaTime;
	if (m_fMaxDeltaTime < m_fConstDeltaTime) {
		m_fMaxDeltaTime = m_fConstDeltaTime;
	}
	if (m_fMinDeltaTime > m_fConstDeltaTime) {
		m_fMinDeltaTime = m_fConstDeltaTime;
	}
}

double CFrameRateController::GetTimeSec()
{
	return m_fTime;
}

int CFrameRateController::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);
	pClass->AddField("Paused", FieldType_Bool, (void*)SetPaused_s, (void*)IsPaused_s, NULL, NULL, bOverride);
	pClass->AddField("ElapsedTime", FieldType_Double, (void*)0, (void*)GetElapsedTime_s, NULL, NULL, bOverride);
	return S_OK;
}
