//-----------------------------------------------------------------------------
// Class:	MovieCtrlers
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.11.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "FileManager.h"
#include "BipedObject.h"
#include "AIBase.h"

#include "BipedController.h"
#include "AISimulator.h"
#include "BipedStateManager.h"
#include "ParaScriptingCharacter.h"
#include "memdebug.h"

using namespace ParaEngine;

/** @def new head on  display for character text */
#define NEW_HEAD_ON_DISPLAY

/** @def max number of characters per line in a movie file.*/
#define MAX_MOVIE_FILE_LINE		300

/** @def the minimum distance between two successive way points */
#define MIN_WAYPOINT_DISTANCE	0.3f

/** @def we will not record an additional way point, if the new turning angle W satisfies sin(W)*sin(W) < MOVIE_WAYPOINT_TURNING_ANGLE_FACTOR */
#define MOVIE_WAYPOINT_TURNING_ANGLE_FACTOR 0.01f

/** default movie file name. */
const string g_sDefaultMovieFileName("script/movie/movie_recorded.txt");

namespace ParaEngine
{
	/** return true if equal. T is a 3D vector, such as Vector3 */
	template <class T>
		inline bool CompareXZ(const T& a, const T& b)
	{
		return (a.x==b.x && a.z==b.z);
	}
}

#include "MovieCtrlers.h"



/************************************************************************/
/* CMovieCtrler                                                        */
/************************************************************************/
CMovieCtrler::CMovieCtrler(CAIBase* pAI)
:CBipedController(pAI),m_vPosOffset(0,0,0),m_nLastAction(-1),m_nLastEffect(-1), m_currentMode(MOVIE_SUSPENDED), m_nLastAnimState(0),m_nLastDialog(-1),m_bHasOffsetPosition(false)
{
}

CMovieCtrler::CMovieCtrler(void)
:CBipedController(),m_vPosOffset(0,0,0),m_nLastAction(-1), m_nLastEffect(-1),m_currentMode(MOVIE_SUSPENDED), m_nLastAnimState(0),m_nLastDialog(-1),m_bHasOffsetPosition(false)
{
}
CMovieCtrler::~CMovieCtrler(void)
{
	if(m_currentMode ==  MOVIE_RECORDING)
	{
		// if it is still recording without saving the file when exits, we will save the movie to a default file.
		SaveToFile(g_sDefaultMovieFileName);
	}
}

bool CMovieCtrler::HasOffsetPosition()
{
	return m_bHasOffsetPosition;
}
void CMovieCtrler::SetPosOffset(const Vector3& vPos)
{
	m_vPosOffset = vPos;
	m_bHasOffsetPosition = true;
}
const Vector3& CMovieCtrler::GetPosOffset()
{
	return m_vPosOffset;
}

void CMovieCtrler::Resume()
{
	// call base class
	CBipedController::Resume();

	// set recording state for the biped's state manager.
	CBipedObject* pBiped = GetBiped();
	if(pBiped)
	{
		CBipedStateManager* pState =  pBiped->GetBipedStateManager();
		if(pState)
		{
			pState->SetRecording(m_currentMode ==  MOVIE_RECORDING);
		}
	}
}

void CMovieCtrler::Suspend()
{
	// call base class
	CBipedController::Suspend();

	// set recording state to false for the biped's state manager.
	CBipedObject* pBiped = GetBiped();
	if(pBiped)
	{
		CBipedStateManager* pState =  pBiped->GetBipedStateManager();
		if(pState)
		{
			pState->SetRecording(false);
		}
	}
}

void CMovieCtrler::SetMode(MovieMode mode)
{
	m_currentMode = mode;
	if(m_currentMode == MOVIE_SUSPENDED)
		Suspend();// suspend
	else
	{
		if(m_currentMode ==  MOVIE_RECORDING)
		{
			Resume(); // resume 
			CBipedObject* pBiped = GetBiped();
			if(pBiped!=NULL && m_fTimeElapsed == 0.f)
				SetPosOffset(pBiped->GetPosition());
		}
	}
}
void CMovieCtrler::AddLoopingFrame(float fLoopToTime)
{

}
void CMovieCtrler::SaveToFile(const std::string& filename)
{
	CParaFile file;
	if(file.CreateNewFile(filename.c_str()))
	{
		char line[MAX_MOVIE_FILE_LINE+1];
		line[MAX_MOVIE_FILE_LINE] = '\0';

		// write position keys.
		int nLen = m_keyframesPos.GetSize();
		snprintf(line, MAX_MOVIE_FILE_LINE, "-- position keys #%d offset(%f,%f,%f)\r\n", nLen, m_vPosOffset.x, m_vPosOffset.y, m_vPosOffset.z);
		file.WriteString(line);

		for (int i=0; i<nLen;i++) {
			const PosKey& key = m_keyframesPos[i];
			snprintf(line, MAX_MOVIE_FILE_LINE, "%f pos(%f,%f,%f,%f)\r\n", m_keyframesPos.time(i), 
				key.vPos.x, key.vPos.y, key.vPos.z, key.fFacing);
			file.WriteString(string(line));
		}

		// write action keys.
		nLen = m_keyframesAction.GetSize();
		snprintf(line, MAX_MOVIE_FILE_LINE, "-- action keys #%d\r\n", nLen);
		file.WriteString(line);
		for (int i=0; i<nLen;i++) {
			snprintf(line, MAX_MOVIE_FILE_LINE, "%f %s\r\n", m_keyframesAction.time(i), m_keyframesAction[i].ToString());
			file.WriteString(string(line));
		}
		// write effect keys.
		nLen = m_keyframesEffect.GetSize();
		snprintf(line, MAX_MOVIE_FILE_LINE, "-- effect keys #%d\r\n", nLen);
		file.WriteString(line);
		for (int i=0; i<nLen;i++) {
			snprintf(line, MAX_MOVIE_FILE_LINE, "%f e(%d) %s\r\n", m_keyframesEffect.time(i), m_keyframesEffect[i].nEffectID, m_keyframesEffect[i].sTarget.c_str());
			file.WriteString(string(line));
		}

		// write dialog keys.
		nLen = m_keyframesDialog.GetSize();
		snprintf(line, MAX_MOVIE_FILE_LINE, "-- dialog keys #%d\r\n", nLen);
		file.WriteString(line);
		for (int i=0; i<nLen;i++) {
			snprintf(line, MAX_MOVIE_FILE_LINE, "%f\r\n%s\r\r", m_keyframesDialog.time(i), m_keyframesDialog[i].sDialog.c_str());
			file.WriteString(string(line));
		}
	}
	SetMode(MOVIE_SUSPENDED);
}

void CMovieCtrler::LoadFromFile(const std::string& filename)
{
	char line[MAX_MOVIE_FILE_LINE+1];
	char name[MAX_MOVIE_FILE_LINE];

	/// use the biped's current position as offset
	CBipedObject* pBiped = GetBiped();
	if(pBiped!=NULL)
		SetPosOffset(pBiped->GetPosition());

	// plus the offset if it is specified.
	int nLength = 0;
	Vector3 vPosOffset(0,0,0);
	if((nLength =sscanf(filename.c_str(), "%s pos(%f,%f,%f)", name, &vPosOffset.x, &vPosOffset.y, &vPosOffset.z))==4)
		m_vPosOffset += vPosOffset;

	if(nLength<1)
		return;

	CParaFile file(name);
	if(file.isEof())
		return;

	/// clear the key frames.
	m_keyframesPos.Clear();
	m_keyframesAction.Clear();
	m_keyframesDialog.Clear();

	/// load new ones from file
	while( (nLength = file.GetNextLine(line, MAX_MOVIE_FILE_LINE)) > 0)
	{
		float t,x,y,z,facing; 
		int nID;
		int nCount = 0;
		if((nCount=sscanf(line, "%f pos(%f,%f,%f,%f) %s", &t, &x, &y, &z, &facing, name))>=5)
		{
			m_keyframesPos.AppendNewKey(t, PosKey(x,y,z,facing));
			if(nCount>=6)
				m_keyframesAction.AppendNewKey(t, ActionKey(name));
		}
		else if ((nCount = sscanf(line, "%f e(%d) %s", &t, &nID, name)) >= 3)
		{
			m_keyframesEffect.AppendNewKey(t, EffectKey(nID, name));
		}
		else if ((nCount = sscanf(line, "%f %s", &t, name)) >= 2)
		{
			m_keyframesAction.AppendNewKey(t, ActionKey(name));
		}
		else if(nCount==1)
		{
			string sDialog;
			// dialog line
			while(!file.isEof())
			{
				char c = *file.getPointer();

				if(c == '\r' && (*(file.getPointer()+1)) == '\r')
				{
					file.seekRelative(2);
					break;
				}
				else 
					sDialog+=c;
				file.seekRelative(1);
			}
			m_keyframesDialog.AppendNewKey(t, DialogKey(sDialog));
		}
	}
}

void CMovieCtrler::SetTime(float fTime)
{
	CBipedObject* pBiped = GetBiped();
	if(pBiped==NULL)
		return;

	m_fTimeElapsed = fTime;

	/** also update the time in the key frames.*/
	m_keyframesPos.UpdateTime(m_fTimeElapsed);
	m_keyframesAction.UpdateTime(m_fTimeElapsed);
	m_keyframesDialog.UpdateTime(m_fTimeElapsed);
	
	if(m_keyframesPos.GetSize() > 0 && HasOffsetPosition())
	{
		/// move the character to the position.
		DVector3 vDest = m_keyframesPos.GetRelative(0).vPos+m_vPosOffset;
		pBiped->SetPosition(vDest);
		// remove way point.
		pBiped->RemoveWayPoint();
	}
	
	CBipedStateManager* pState =  pBiped->GetBipedStateManager();
	if(pState)
	{
		/// use running as movement anyway. Because we can not easily find out the current walking style,
		/// so just assume the maximum speed.
		pState->SetWalkOrRun(false);
		// assume unmounted
		pState->SetMounted(false);
	}
	// we do not know what action is the last one, so we will just set it to unknown.
	m_nLastAction = -1;
	
}

void CMovieCtrler::RecordDialogKey(const string& sDialog, float fTime)
{
	if(m_currentMode != MOVIE_PLAYING)
	{
		if(fTime<0)
		{
			m_keyframesDialog.TrimToTime(m_fTimeElapsed);
			m_keyframesDialog.AppendNewKey(m_fTimeElapsed, DialogKey(sDialog));
		}
		else
			m_keyframesDialog.InsertNewKey(fTime, DialogKey(sDialog));
	}
}

bool CMovieCtrler::RecordActionKey(const string& sActionName, float fTime)
{
	if(m_currentMode != MOVIE_PLAYING)
	{
		if(fTime<0)
		{
			// append to the last
			m_keyframesAction.TrimToTime(m_fTimeElapsed);
			m_keyframesAction.AppendNewKey(m_fTimeElapsed, ActionKey(sActionName));
		}
		else if(fTime == 0.f)
		{
			if(m_fTimeElapsed > 0.f)
			{
				// looping to beginning by appending a 0 key at the current position.
				m_keyframesAction.TrimToTime(m_fTimeElapsed);
				m_keyframesAction.AppendNewKey(m_fTimeElapsed, ActionKey(sActionName));
				m_keyframesAction.AppendNewKey(0.f, ActionKey(sActionName));
			}
			else
				return false;
		}
		else
			m_keyframesAction.InsertNewKey(fTime, ActionKey(sActionName));
		return true;
	}
	return false;
}

bool CMovieCtrler::RecordActionKey(const ActionKey* actionKey, float fTime)
{
	return RecordActionKey(actionKey->ToString(), fTime);
}

void CMovieCtrler::RecordEffectKey(int effectID, const string& sTarget, float fTime)
{
	if(m_currentMode != MOVIE_PLAYING)
	{
		if(fTime<0)
		{
			m_keyframesEffect.TrimToTime(m_fTimeElapsed);
			m_keyframesEffect.AppendNewKey(m_fTimeElapsed, EffectKey(effectID, sTarget));
		}
		else
			m_keyframesEffect.InsertNewKey(fTime, EffectKey(effectID, sTarget));
	}
}

void CMovieCtrler::DisplayDialog(const string& sDialog)
{
#ifdef NEW_HEAD_ON_DISPLAY
	CBipedObject* pBiped = GetBiped();
	if(pBiped)
	{
		static char tmp[256];
		int nDuration = (int)sDialog.size()/4;
		if(nDuration<3)
			nDuration = 3;
		snprintf(tmp, 256, "headon_speech.Speek(\"%s\", [[%s]], %d);", pBiped->GetIdentifier().c_str(),sDialog.c_str(), nDuration);
		CGlobals::GetAISim()->NPLDoString(tmp);
	}
#else
	string sScript = "_movie.AddMovieDialog(\"";
	sScript+=sDialog;
	sScript+="\");";
	CGlobals::GetAISim()->NPLDoString(sScript.c_str());
#endif
}

void CMovieCtrler::FrameMove(float fDeltaTime)
{
	if(m_currentMode == MOVIE_PLAYING)
	{
		m_fTimeElapsed +=fDeltaTime;
		CBipedObject* pBiped = GetBiped();
		if(pBiped==NULL)
			return;
		if(m_keyframesPos.GetSize() > 0)
		{
			int nPos = m_keyframesPos.UpdateTime(m_fTimeElapsed);
			const PosKey& poskey  = m_keyframesPos.GetRelative(0);
			DVector3 vDest = poskey.vPos + m_vPosOffset;
			if(pBiped->GetLastWayPoint().GetPointType()==BipedWayPoint::COMMAND_POINT)
			{
				if(!(CompareXZ<Vector3>(vDest ,pBiped->GetLastWayPoint().vPos)))
				{
					CBipedStateManager* pState =  pBiped->GetBipedStateManager();
					if(pState)
					{
						pState->SetPos(vDest);
						pState->SetAngleDelta(poskey.fFacing);
						pState->AddAction(CBipedStateManager::S_WALK_POINT, (const void*)1);
					}
				}
			}
			else 
			{
				DVector3 vPos =  pBiped->GetPosition();
				if( vPos.x != vDest.x || vPos.z != vDest.z)
				{
					CBipedStateManager* pState =  pBiped->GetBipedStateManager();
					if(pState)
					{
						pState->SetPos(vDest);
						pState->SetAngleDelta(poskey.fFacing);
						pState->AddAction(CBipedStateManager::S_WALK_POINT,(const void*)1);
					}
				}
				else if(pBiped->GetFacing()!=poskey.fFacing)
				{
					BipedWayPoint& lastPT = pBiped->GetLastWayPoint();
					
					pBiped->FacingTarget(poskey.fFacing);
					
					//else if (lastPT.GetPointType() == BipedWayPoint::COMMAND_POINT) {}
					
				}
			}
		}
		if(m_keyframesAction.GetSize()>0)
		{
			int nAction = m_keyframesAction.UpdateTime(m_fTimeElapsed);
			if(m_nLastAction!=nAction && m_keyframesAction.time(nAction-1)<=m_fTimeElapsed)
			{
				m_nLastAction = nAction;

				// play the animation.
				CBipedStateManager* pState =  pBiped->GetBipedStateManager();
				if(pState)
				{
					const ActionKey& actionKey = m_keyframesAction.GetRelative(-1);
					if(actionKey.IsMount())
						pState->AddAction(CBipedStateManager::S_MOUNT, "");
					else
						pState->AddAction(CBipedStateManager::S_ACTIONKEY, &actionKey);
				}
			}
		}
		if(m_keyframesEffect.GetSize()>0)
		{
			int nEffect = m_keyframesEffect.UpdateTime(m_fTimeElapsed);
			if(m_nLastEffect!=nEffect && m_keyframesEffect.time(nEffect-1)<=m_fTimeElapsed)
			{
				m_nLastEffect = nEffect;
				// play the effect.
				ParaScripting::ParaCharacter character_(pBiped);
				const EffectKey& effectKey = m_keyframesEffect.GetRelative(-1);
				character_.CastEffect2(effectKey.nEffectID, effectKey.sTarget.c_str());
			}
		}
		if(m_keyframesDialog.GetSize()>0)
		{
			int nDialog = m_keyframesDialog.UpdateTime(m_fTimeElapsed);
			if(m_nLastDialog!=nDialog && m_keyframesDialog.time(nDialog-1)<=m_fTimeElapsed)
			{
				m_nLastDialog = nDialog;
				// show the dialog
				DisplayDialog(m_keyframesDialog.GetRelative(-1).sDialog);
			}
		}
	}
	else if(m_currentMode == MOVIE_RECORDING)
	{
		CBipedObject* pBiped = GetBiped();
		if(pBiped==NULL)
			return;
		// trim movie to its current position.
		m_keyframesPos.TrimToTime(m_fTimeElapsed);
		m_keyframesAction.TrimToTime(m_fTimeElapsed);

		/* record position keys */
		Vector3 vCurrentPos = pBiped->GetPosition() - m_vPosOffset;
		float fBipedFacing = pBiped->GetFacing();
		int nSize = m_keyframesPos.GetSize();
		if(nSize <=1)
			m_keyframesPos.AppendNewKey(m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
		else
		{
			CBipedStateManager* pState =  pBiped->GetBipedStateManager();
			if(pState )
			{
				if(pState->IsMounted())
				{
				}
				else if(!(CompareXZ<Vector3>(m_keyframesPos[nSize-1].vPos, vCurrentPos)))
				{
					Vector3 tmp = (m_keyframesPos[nSize-1].vPos- m_keyframesPos[nSize-2].vPos);
					float r1 = (tmp.x*tmp.x+tmp.z*tmp.z);
					if(r1 < FLT_TOLERANCE)
					{ // from standing to walking
						m_keyframesPos.AppendNewKey(m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
					}
					else
					{ // walking to walking
						r1 = sqrt(r1);

						if(r1<MIN_WAYPOINT_DISTANCE)
						{
							m_keyframesPos.UpdateKeyFrame(nSize-1, m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
						}
						else
						{
							float x1 = tmp.x/r1;
							float y1 = tmp.z/r1;

							tmp = (vCurrentPos- m_keyframesPos[nSize-1].vPos);
							float r1_ = sqrt(tmp.x*tmp.x+tmp.z*tmp.z);
							float x1_ = tmp.x/r1_;
							float y1_ = tmp.z/r1_;

							float dx = (x1_-x1);
							float dy = (y1_-y1);
							if((dx*dx+dy*dy)<MOVIE_WAYPOINT_TURNING_ANGLE_FACTOR)
							{
								m_keyframesPos.UpdateKeyFrame(nSize-1, m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
							}
							else
							{
								m_keyframesPos.AppendNewKey(m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
							}
						}
					}
				}
				else
				{
					if(!(CompareXZ<Vector3>(m_keyframesPos[nSize-1].vPos, m_keyframesPos[nSize-2].vPos)))
					{ // from walking to standing
						m_keyframesPos.AppendNewKey(m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
					}
					else
					{ // from standing to standing
						m_keyframesPos.UpdateKeyFrame(nSize-1, m_fTimeElapsed, PosKey(vCurrentPos.x,vCurrentPos.y,vCurrentPos.z, fBipedFacing));
					}
				}
			}//if(pState && ! pState->IsMounted())
		}
		/* record action keys */
		// action keys are recorded from the biped state manager itself, rather than polling during each frame move.
		m_fTimeElapsed +=fDeltaTime;
	}

}
