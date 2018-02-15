//-----------------------------------------------------------------------------
// Class:	SequenceEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.5.14
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "SequenceEntity.h"
#include "memdebug.h"
using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// SequenceEntity
//
//////////////////////////////////////////////////////////////////////////

SequenceEntity::SequenceEntity()
:m_nPlayMode(PLAYMODE_FORWORD_LOOPED), m_vStartPos(0,0,0), m_fStartFacing(0)
{
	
}

SequenceEntity::SequenceEntity(const AssetKey& key)
:AssetEntity(key), m_nPlayMode(PLAYMODE_FORWORD_LOOPED), m_vStartPos(0,0,0), m_fStartFacing(0)
{
}

void SequenceEntity::Reset()
{
	m_nPlayMode = PLAYMODE_FORWORD_LOOPED;
	m_vStartPos = Vector3(0,0,0);
	m_fStartFacing = 0;
	m_lables.clear();
	m_items.clear();
	m_description.clear();
}

SequenceEntity::~SequenceEntity()
{

}

bool SequenceEntity::DeleteKeysRange(int nFrom, int nTo)
{
	vector<SequenceItem>::iterator itFrom, itTo;
	if ( nFrom>=0 && nTo<GetTotalKeys())
		itFrom = m_items.begin()+nFrom;
	else
		return false;
	if(nTo < 0)
		itTo = m_items.end();
	else
		itTo = m_items.begin()+nTo;
	m_items.erase(itFrom, itTo);
	return true;
}

HRESULT SequenceEntity::InitDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;
	m_bIsInitialized =true;


	return S_OK;
}

HRESULT SequenceEntity::DeleteDeviceObjects()
{
	m_bIsInitialized =false;
	m_items.clear();
	return S_OK;
}

void SequenceEntity::Cleanup()
{
	m_lables.clear();
}

void SequenceEntity::CompileKeys()
{
	Vector3 vPosLast(0,0,0);
	Vector3 vNorm(0,1,0);
	float fFacingLast = 0;

	vector<SequenceItem>::iterator itCur, itEnd = m_items.end();
	for (itCur = m_items.begin();itCur!=itEnd;++itCur)
	{
		SequenceItem& item = (*itCur);
		switch(item.m_commandtype)
		{
		case CMD_MoveTo:
		case CMD_WalkTo:
		case CMD_RunTo:
			{
				if(Math::ComputeFacingTarget(item.m_vPos_R, Vector3(0,0,0), item.m_fFacing))
					fFacingLast = item.m_fFacing;
				else
					item.m_fFacing = fFacingLast;
				break;
			}
		case CMD_Turn:
			{
				fFacingLast = item.m_fFacing;
				break;
			}
		case CMD_MoveForward:
			{
				item.m_fFacing = fFacingLast;
				float fMoveFacing = fFacingLast;
				item.m_vPos_R.x += cosf(fMoveFacing)*item.m_fMoveDistance;
				item.m_vPos_R.z += sinf(fMoveFacing)*item.m_fMoveDistance;
				break;
			}
		case CMD_MoveBack:
			{
				item.m_fFacing = fFacingLast;
				float fMoveFacing = Math::ToStandardAngle(fFacingLast+MATH_PI);
				item.m_vPos_R.x += cosf(fMoveFacing)*item.m_fMoveDistance;
				item.m_vPos_R.z += sinf(fMoveFacing)*item.m_fMoveDistance;
				break;
			}
		case CMD_MoveLeft:
			{
				item.m_fFacing = fFacingLast;
				float fMoveFacing = Math::ToStandardAngle(fFacingLast-MATH_PI/2);
				item.m_vPos_R.x += cosf(fMoveFacing)*item.m_fMoveDistance;
				item.m_vPos_R.z += sinf(fMoveFacing)*item.m_fMoveDistance;
				break;
			}
		case CMD_MoveRight:
			{
				item.m_fFacing = fFacingLast;
				float fMoveFacing = Math::ToStandardAngle(fFacingLast+MATH_PI/2);
				item.m_vPos_R.x += cosf(fMoveFacing)*item.m_fMoveDistance;
				item.m_vPos_R.z += sinf(fMoveFacing)*item.m_fMoveDistance;
				break;
			}
		case CMD_Goto:
			{
				item.m_fFacing = fFacingLast;
				if(!item.m_strParam.empty())
				{
					item.m_nGotoOffset = GetLableIndex(item.m_strParam);
				}
				break;
			}
		default:
			{
				item.m_fFacing = fFacingLast;
				break;
			}
		}
		item.m_vPos = vPosLast + item.m_vPos_R;
		vPosLast = item.m_vPos;
	}
}
void SequenceEntity::AddLable(const string& sLable)
{
	m_lables[sLable] = GetTotalKeys();
}

int SequenceEntity::GetLableIndex(const string& sLable)
{
	map<std::string, int>::iterator it =  m_lables.find(sLable);
	if (it != m_lables.end())
	{
		return (*it).second;
	}
	OUTPUT_LOG("warning: label %s not found\r\n", sLable.c_str());
	return 0;
}
