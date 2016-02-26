//-----------------------------------------------------------------------------
// Class:	CSequenceCtler
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.5.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SequenceEntity.h"
#include "BipedObject.h"
#include "BipedStateManager.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"

#include "util/StringHelper.h"
#include "SequenceCtler.h"

using namespace ParaEngine;

/** @def if defined, when the biped is told to walk to a new destination, it will turn to that direction immediately with using an animation. */
#define IMMEDIATE_FACING
//////////////////////////////////////////////////////////////////////////
//
// SequenceCtler
//
//////////////////////////////////////////////////////////////////////////
CSequenceCtler::CSequenceCtler(CAIBase* pAI)
:CBipedController(pAI),
m_bIsAddingKey(false),m_bForward(true),m_nKeyPos(0), m_nSequenceState(SEQ_EMPTY), m_fMinInterval(0.1f), 
m_fUnprocessedTime(0), m_fMovingTimeOut(30.f), m_fItemDuration(0),m_fTurningTimeOut(2.f)
{
}

CSequenceCtler::CSequenceCtler(void)
:m_bIsAddingKey(false),m_bForward(true),m_nKeyPos(0), m_nSequenceState(SEQ_EMPTY), m_fMinInterval(0.1f), 
m_fUnprocessedTime(0), m_fMovingTimeOut(30.f), m_fItemDuration(0),m_fTurningTimeOut(2.f)
{
}

CSequenceCtler::~CSequenceCtler(void)
{
	if(m_nSequenceState == SEQ_MANAGED)
	{
	}
	else if(m_nSequenceState == SEQ_CREATED)
	{
	}
}

bool CSequenceCtler::Save(bool bOverride)
{
	//TODO:
	return true;
}
bool CSequenceCtler::Load(int nSequenceID)
{
	//TODO:
	return true;
}
bool CSequenceCtler::Load(const string& fileName)
{
	//TODO:
	return true;
}

int  CSequenceCtler::Create(const string& name, const string& description, const char* pData,bool bInMemory)
{
	//TODO:
	return 0;
}

string CSequenceCtler::ToString()
{
	//TODO:
	return "";
}
int CSequenceCtler::GetSequenceID()
{
	//TODO:
	return 0;
}

bool CSequenceCtler::DeleteKeysRange(int nFrom, int nTo)
{
	if(m_pSequenceEntity)
		return m_pSequenceEntity->DeleteKeysRange(nFrom, nTo);
	else
		return false;
}


void CSequenceCtler::Stop(CBipedObject* pBiped)
{
	CBipedStateManager* pState = NULL;
	if(pBiped!=0)
	{
		pBiped->ForceStop();
		pBiped->SetStandingState();
		if((pState=pBiped->GetBipedStateManager())!= NULL)
		{
			pState->AddAction(CBipedStateManager::S_STANDING);
		}
	}
}

void CSequenceCtler::MoveCommand(CBipedObject* pBiped, const DVector3& vDest_r, bool bForceRun, bool bForceWalk, float * pFacing)
{
	CBipedStateManager* pState = NULL;
	if(pBiped==NULL || (pState=pBiped->GetBipedStateManager())== NULL)
		return;
	
	DVector3 vDest = pBiped->GetPosition();
	vDest += vDest_r;

	if(!pBiped->IsSentient())
	{
		pBiped->SetPosition(vDest);
		pBiped->UpdateTileContainer();
		if(pFacing)
		{
			pBiped->SetFacing(*pFacing);
		}
		return;
	}
	
	pState->SetPos(vDest);

	float fFacing = 0.f;
	if( !Math::ComputeFacingTarget(vDest_r, Vector3(0,0,0), fFacing) )
		fFacing = pBiped->GetFacing();

#ifdef IMMEDIATE_FACING
	pBiped->SetFacing(fFacing);
#endif
	if(pFacing!=NULL)
		pState->SetAngleDelta(*pFacing);
	else
		pState->SetAngleDelta(fFacing);

	if(bForceWalk)
		pState->SetWalkOrRun(true);
	else if(bForceRun)
		pState->SetWalkOrRun(false);

	pState->AddAction(CBipedStateManager::S_WALK_POINT, (const void*)1);
}

void CSequenceCtler::RunTo(double x, double y, double z)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_RunTo);
			int nKeyCount = m_pSequenceEntity->GetTotalKeys();
			item.m_vPos_R = DVector3(x,y,z);
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
		DVector3 v(x, y, z);
		MoveCommand(GetBiped(), v, false, false);
	}
}
void CSequenceCtler::WalkTo(double x, double y, double z)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_WalkTo);
			int nKeyCount = m_pSequenceEntity->GetTotalKeys();
			item.m_vPos_R = DVector3(x,y,z);
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
		DVector3 v(x, y, z);
		MoveCommand(GetBiped(), v, false, true);
	}
}

void CSequenceCtler::MoveTo(double x, double y, double z)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_MoveTo);
			int nKeyCount = m_pSequenceEntity->GetTotalKeys();
			item.m_vPos_R = DVector3(x,y,z);
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
		DVector3 v(x, y, z);
		MoveCommand(GetBiped(), v, false, false);
	}
}

void ParaEngine::CSequenceCtler::MoveAndTurn(double x, double y, double z, float facing)
{
	if(m_bIsAddingKey)
	{
		OUTPUT_LOG("warning: MoveAndTurn() is only supported outside sequence adding\n");
	}
	else
	{
		// execute the command
		DVector3 v(x, y, z);
		MoveCommand(GetBiped(), v, false, false, &facing);
	}
}

void CSequenceCtler::PlayAnim(const string& sAnim)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_PlayAnim);
			item.m_strParam = sAnim;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::PlayAnim(int nAnimID)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_PlayAnim);
			item.m_dwValue = nAnimID;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Wait(float fSeconds)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Wait);
			item.m_fWaitSeconds = fSeconds;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Exec(const string& sCmd)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Exec);
			item.m_strParam = sCmd;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Pause()
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Pause);
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Turn(float fAngleAbsolute)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Turn);
			item.m_fFacing = fAngleAbsolute;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
		CBipedObject* pBiped = GetBiped();
		if(pBiped != 0)
			pBiped->FacingTarget(fAngleAbsolute);
	}
}
void CSequenceCtler::MoveForward(float fDistance)
{
	if(m_bIsAddingKey)
	{
		if(m_pSequenceEntity)
		{
			// add command to sequence
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_MoveForward);
			item.m_fMoveDistance = fDistance;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::MoveBack(float fDistance)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_MoveBack);
			item.m_fMoveDistance = fDistance;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::MoveLeft(float fDistance)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_MoveLeft);
			item.m_fMoveDistance = fDistance;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::MoveRight(float fDistance)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_MoveRight);
			item.m_fMoveDistance = fDistance;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Jump()
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Jump);
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Goto(int nOffset)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Goto);
			item.m_nGotoOffset = nOffset;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}
void CSequenceCtler::Goto(const string& sLable)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			SequenceEntity::SequenceItem item(SequenceEntity::CMD_Goto);
			item.m_strParam = sLable;
			item.m_nGotoOffset = 0;
			m_pSequenceEntity->PushItem(item);
		}
	}
	else
	{
		// execute the command
	}
}

void CSequenceCtler::Lable(const string& sLable)
{
	if(m_bIsAddingKey)
	{
		// add command to sequence
		if(m_pSequenceEntity)
		{
			m_pSequenceEntity->AddLable(sLable);
		}
	}
	else
	{
		// execute the command
	}
}

void CSequenceCtler::SetEntity(SequenceEntity* e)
{
	m_pSequenceEntity = e;
	if(m_pSequenceEntity)
	{
		CopyEntityParamter(*e);
		if(m_nSequenceState == SEQ_EMPTY)
		{
			// if old sequence is empty, we will use the current biped's position as the initial position.
			CBipedObject* pBiped = GetBiped();
			if(pBiped != 0)
			{
				m_vStartPos = pBiped->GetPosition();
				m_fStartFacing = pBiped->GetFacing();
			}
		}
	}
}

void CSequenceCtler::BeginAddKeys()
{
	m_bIsAddingKey = true;
	if(m_nSequenceState == SEQ_EMPTY)
	{
		CBipedObject* pBiped = GetBiped();
		string sName;
		if(pBiped)
			SetEntity(CGlobals::GetAssetManager()->LoadSequence(pBiped->GetName().c_str()));
		
		if(m_pSequenceEntity){
			m_pSequenceEntity->Reset();
			m_nSequenceState = SEQ_CREATED;
		}
		else
		{
			OUTPUT_LOG("failed creating sequence\r\n");
		}
	}
};
int CSequenceCtler::GetTotalKeys()
{

	return (m_pSequenceEntity) ? m_pSequenceEntity->GetTotalKeys() : 0;
}
void CSequenceCtler::SetKeyPos(int nPos)
{
	AdvanceKey(nPos - m_nKeyPos);
}

void CSequenceCtler::EndAddKeys()
{
	if(m_bIsAddingKey == false)
	{
		OUTPUT_LOG("warning: CSequenceCtler::BeginAddKeys not called. ");
	}
	m_bIsAddingKey = false;
	if(m_pSequenceEntity)
		m_pSequenceEntity->CompileKeys();
};

void CSequenceCtler::CopyEntityParamter(const SequenceEntity& e)
{
	m_description = e.m_description;
	m_vStartPos = e.m_vStartPos;
	m_fStartFacing = e.m_fStartFacing;
	SetPlayMode(e.m_nPlayMode);
}

void CSequenceCtler::SetPlayMode(int mode)
{
	m_nPlayMode=mode;
	m_bForward = (m_nPlayMode != SequenceEntity::PLAYMODE_BACKWORD) && (m_nPlayMode != SequenceEntity::PLAYMODE_BACKWORD_LOOPED);
}

int CSequenceCtler::AdvanceKey(int nOffset)
{
	int nCount = GetTotalKeys();
	if(nCount == 0)
		return 0;
	int nPos = m_nKeyPos+nOffset;
	if(nPos<nCount && nPos>=0)
		m_nKeyPos = nPos;
	else
	{
		switch(m_nPlayMode)
		{
		case SequenceEntity::PLAYMODE_FORWORD:
		case SequenceEntity::PLAYMODE_BACKWORD:
			if(nPos >= nCount)
			{
				nOffset = nCount-m_nKeyPos-1;
				m_nKeyPos = nCount-1;
			}
			else // if(nPos<0)
			{
				nOffset = m_nKeyPos;
				m_nKeyPos = 0;
			}
			break;
		case SequenceEntity::PLAYMODE_FORWORD_LOOPED:
		case SequenceEntity::PLAYMODE_BACKWORD_LOOPED:
			if(nPos >= nCount)
			{
				nPos = nPos%nCount;
				m_nKeyPos = nPos;
			}
			else // if(nPos<0)
			{
				nPos = nCount - (-nPos)%nCount;
				nOffset = nPos;
			}
			break;
		case SequenceEntity::PLAYMODE_ZIGZAG:
			if(nPos >= nCount)
			{
				m_bForward = false;
				nPos = nPos%nCount;
				m_nKeyPos = nCount - nPos;
			}
			else // if(nPos<0)
			{
				m_bForward = true;
				nPos = (-nPos)%nCount;
				nOffset = nPos;
			}
			break;
		default:
			break;
		}
	}
	PE_ASSERT(m_nKeyPos>=0 && m_nKeyPos<nCount);
	return nOffset;
}
void CSequenceCtler::FrameMove(float fDeltaTime)
{
	if(IsActive() && GetTotalKeys()>0)
	{
		CBipedObject* pBiped = GetBiped();
		CBipedStateManager* pState = NULL;
		if(pBiped==NULL || (pState=pBiped->GetBipedStateManager())== NULL)
			return;
		// process FPS. 
		m_fUnprocessedTime += fDeltaTime;
		fDeltaTime = m_fUnprocessedTime;
		m_fItemDuration += fDeltaTime;
		if(m_fUnprocessedTime> m_fMinInterval)
			m_fUnprocessedTime = 0;
		else
			return;
		bool bFinished = false;
		
		//////////////////////////////////////////////////////////////////////////
		//
		// check if the current sequence command is finished
		//
		//////////////////////////////////////////////////////////////////////////
		SequenceEntity::SequenceItem& item =  m_pSequenceEntity->GetItem(m_nKeyPos);
		int nOffset = m_bForward?1:-1;
		switch(item.m_commandtype)
		{
		case SequenceEntity::CMD_MoveTo:
		case SequenceEntity::CMD_WalkTo:
		case SequenceEntity::CMD_RunTo:
		case SequenceEntity::CMD_MoveForward:
		case SequenceEntity::CMD_MoveBack:
		case SequenceEntity::CMD_MoveLeft:
		case SequenceEntity::CMD_MoveRight:
			{
				if(m_fItemDuration > m_fMovingTimeOut)
				{
					bFinished = true;
				}
				else
				{
					Vector3 vCurPos =  pBiped->GetPosition();
					Vector3 vDest = item.m_vPos+m_vStartPos;
					if (Math::CompareXZ(vCurPos, vDest, 0.01f))
					{
						bFinished = true;
					}
					else if(pBiped->IsStanding())
					{
						nOffset = 0;
						bFinished = true;
					}
				}
				break;
			}
		case SequenceEntity::CMD_Turn:
			{
				if(m_fItemDuration > m_fTurningTimeOut)
				{
					bFinished = true;
				}
				else
				{
					float fCurFacing	=  pBiped->GetFacing();
					float fFacing = item.m_fFacing + m_fStartFacing; 
					if (abs(fFacing-fCurFacing)<0.01f)
					{
						bFinished = true;
					}
				}
				
				break;
			}
		case SequenceEntity::CMD_Wait:
			{
				bFinished = (m_fItemDuration >= item.m_fWaitSeconds);
				break;
			}
		default:
			bFinished = true;
			break;
		}

		if(bFinished)
		{
			//////////////////////////////////////////////////////////////////////////
			//
			// process the next sequence command
			//
			//////////////////////////////////////////////////////////////////////////
			m_fItemDuration = 0;
			bool bEndOfSequence = AdvanceKey(nOffset) != nOffset;

			if(bEndOfSequence)
			{
				Suspend();
				return;
			}
			
			SequenceEntity::SequenceItem& item =  m_pSequenceEntity->GetItem(m_nKeyPos);
			DVector3 vDest = item.m_vPos+m_vStartPos;
			float fFacing = item.m_fFacing;
			switch(item.m_commandtype)
			{
			case SequenceEntity::CMD_MoveTo:
			case SequenceEntity::CMD_WalkTo:
			case SequenceEntity::CMD_RunTo:
			case SequenceEntity::CMD_MoveForward:
			case SequenceEntity::CMD_MoveBack:
			case SequenceEntity::CMD_MoveLeft:
			case SequenceEntity::CMD_MoveRight:
				pState->SetPos(vDest);
				pState->SetAngleDelta(fFacing);
				if(item.m_commandtype == SequenceEntity::CMD_WalkTo)
					pState->SetWalkOrRun(true);
				else if(item.m_commandtype == SequenceEntity::CMD_RunTo)
					pState->SetWalkOrRun(false);

				pState->AddAction(CBipedStateManager::S_WALK_POINT, (const void*)1);
				break;
			case SequenceEntity::CMD_Turn:
				{
					pBiped->FacingTarget(fFacing);
					break;
				}
			case SequenceEntity::CMD_Exec:
				{
					string sFile, sCode;
					StringHelper::DevideString(item.m_strParam, sFile, sCode, ';');
					CGlobals::GetScene()->GetScripts().AddScript(sFile, 0, sCode, pBiped);	
					break;
				}
			case SequenceEntity::CMD_Pause:
				{
					Suspend();
					break;
				}
			case SequenceEntity::CMD_PlayAnim:
				{
					if(!item.m_strParam.empty()){
						ActionKey a(item.m_strParam);
						pState->AddAction(CBipedStateManager::S_ACTIONKEY, &a);
					}
					else{
						ActionKey a(item.m_dwValue);
						pState->AddAction(CBipedStateManager::S_ACTIONKEY, &a);
					}
						

					break;
				}
			case SequenceEntity::CMD_Jump:
				{
					pState->AddAction(CBipedStateManager::S_JUMP_START);
					break;
				}
			case SequenceEntity::CMD_Goto:
				{
					if(!item.m_strParam.empty())
					{ 
						// m_nGotoOffset is absolute position for goto command with Labels, so ... 
						AdvanceKey(item.m_nGotoOffset - m_nKeyPos);
					}
					else if(AdvanceKey(item.m_nGotoOffset) != item.m_nGotoOffset)
						Suspend();
					break;
				}
			default:
				break;
			}
		}
	}
}
