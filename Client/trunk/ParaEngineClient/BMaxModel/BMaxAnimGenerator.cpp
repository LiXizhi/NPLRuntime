//-----------------------------------------------------------------------------
// Class:Block max animation generator
// Authors:	LiXizhi,LiPeng
// Emails:	lixizhi@yeah.net
// Date:	2015.12.4
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BMaxParser.h"
#include "ParaXModel/AnimTable.h"
#include "ParaXModel/ParaXBone.h"
#include "BMaxAnimGenerator.h"

namespace ParaEngine
{
	bool BMaxAnimGenerator::s_bInitedBoneState = false;
	std::map<pair<BoneName, BoneFlag>, std::map<int, BoneState>> BMaxAnimGenerator::s_boneStates = std::map<pair<BoneName, BoneFlag>, std::map<int, BoneState>>();

	BMaxAnimGenerator::BMaxAnimGenerator() :
		m_pParser(NULL), m_bHasSetMaxMinPosition(false),
		m_vLeftBonePosition(0, 0, 0), m_vRightBonePosition(0, 0, 0)
	{
		InitBoneState();
	}

	BMaxAnimGenerator::BMaxAnimGenerator(BMaxParser* pParser) :
		m_pParser(pParser), m_bHasSetMaxMinPosition(false),
		m_vLeftBonePosition(0, 0, 0), m_vRightBonePosition(0, 0, 0)
	{
		InitBoneState();
	}

	BMaxAnimGenerator::~BMaxAnimGenerator()
	{

	}

	void BMaxAnimGenerator::AddBoneState(const string& boneName, const string& boneFlag, int animID, BoneState boneState)
	{
		auto iter = s_boneStates.find(pair<string, string>(boneName, boneFlag));
		if (iter != s_boneStates.end())
		{
			auto& boneStates = iter->second;
			auto boneStatesIter = boneStates.find(animID);
			if (boneStatesIter != boneStates.end())
			{
				boneStatesIter->second = boneState;
			}
			else
			{
				boneStates.insert(make_pair(animID, boneState));
			}
		}
		else
		{
			std::map<int, BoneState> boneStates;
			boneStates.insert(make_pair(animID, boneState));

			s_boneStates.insert(make_pair(pair<string, string>(boneName, boneFlag), boneStates));
		}
	}

	void BMaxAnimGenerator::AddWheelBoneState()
	{
		string boneName("wheel");
		string boneFlag("default");
		int animID = ANIM_WALK;
		BoneState boneState;
		boneState.boneRot.push_back(BoneRotState(0, 0));
		boneState.boneRot.push_back(BoneRotState(0.5f, 3.14f));
		boneState.boneRot.push_back(BoneRotState(1.0f, 6.28f));

		AddBoneState(boneName, boneFlag, animID, boneState);
	}

	void BMaxAnimGenerator::AddWalkBoneState()
	{
		string boneName("leg");
		string boneFlag("biped");
		int animID = int(ANIM_WALK);

		BoneState legBoneState;
		legBoneState.boneRot.push_back(BoneRotState(0, 0.942f));
		legBoneState.boneRot.push_back(BoneRotState(0.5f, -0.942f));
		legBoneState.boneRot.push_back(BoneRotState(1.0f, 0.942f));

		AddBoneState(boneName, boneFlag, animID, legBoneState);

		AddBoneState(boneName, "multiple", animID, legBoneState);

		BoneState armBoneState;
		armBoneState.boneRot.push_back(BoneRotState(0, -0.942f));
		armBoneState.boneRot.push_back(BoneRotState(0.5f, 0.942f));
		armBoneState.boneRot.push_back(BoneRotState(1.0f, -0.942f));

		AddBoneState("arm", "default", animID, armBoneState);
	}

	void BMaxAnimGenerator::AddFlyBoneState()
	{
		string boneName("wing");
		string boneFlag("default");
		int animID = ANIM_WALK;

		BoneState boneState;
		boneState.boneRot.push_back(BoneRotState(0, 0.942f));
		boneState.boneRot.push_back(BoneRotState(0.5f, -0.942f));
		boneState.boneRot.push_back(BoneRotState(1.0f, 0.942f));

		AddBoneState(boneName, boneFlag, animID, boneState);
	}

	void BMaxAnimGenerator::InitBoneState()
	{
		if (s_bInitedBoneState)
			return;
		s_bInitedBoneState = true;
		AddWheelBoneState();
		AddWalkBoneState();
		AddFlyBoneState();
	}

	void BMaxAnimGenerator::ParseParameters(NPL::NPLObjectProxy& boneInfo, int boneIndex, string& boneName)
	{
		if (boneInfo[1].GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
		{
			NPL::NPLObjectProxy& cmd = boneInfo[1];

			float bx, by, bz;
			if (boneInfo["attr"].GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
			{
				bx = (float)((double)boneInfo["attr"]["bx"]);
				by = (float)((double)boneInfo["attr"]["by"]);
				bz = (float)((double)boneInfo["attr"]["bz"]);

				Vector3 position(bx, by, bz);
				if (!m_bHasSetMaxMinPosition)
				{
					m_bHasSetMaxMinPosition = true;
					m_vRightBonePosition = position;
					m_vLeftBonePosition = position;
				}
				else
				{
					if (m_vRightBonePosition.z > bz)
					{
						m_vRightBonePosition = position;
					}
					if (m_vLeftBonePosition.z < bz)
					{
						m_vLeftBonePosition = position;
					}
				}
			}

			BoneInfo boneInfo;
			if (cmd[1].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
			{
				const std::string& cmdcontent = cmd[1];
				string _boneName = cmdcontent;

				string boneFlag = "default";

				int nFlagPos = 0;
				// skip tag between {} in name
				int nFromPos = cmdcontent.find_first_of('{');
				if (nFromPos != string::npos)
				{
					int nToPos = cmdcontent.find_last_of('}');
					if (nToPos != string::npos)
					{
						nFlagPos = nToPos;
					}
				}

				string::size_type addFlag = cmdcontent.find_first_of("-", nFlagPos);
				if (addFlag != string::npos)
				{
					_boneName = string(cmdcontent, 0, addFlag);
					string _boneFlag(cmdcontent, addFlag + 1);
					boneFlag = _boneFlag;
				}

				addFlag = cmdcontent.find_first_of("|", nFlagPos);
				if (addFlag != string::npos)
				{
					_boneName = string(cmdcontent, 0, addFlag);
					string boneAddInfo(cmdcontent, addFlag + 1);
					NPL::NPLObjectProxy addMsg = NPL::NPLHelper::StringToNPLTable(boneAddInfo.c_str());

					for (NPL::NPLTable::IndexIterator_Type animIter = addMsg.index_begin(); animIter != addMsg.index_end(); ++animIter)
					{
						NPL::NPLObjectProxy& anim = animIter->second;
						int animID = (int)((double)anim[1]);
						NPL::NPLObjectProxy& rotMsg = anim[2];

						BoneState boneState;
						for (NPL::NPLTable::IndexIterator_Type rotIter = rotMsg.index_begin(); rotIter != rotMsg.index_end(); ++rotIter)
						{
							NPL::NPLObjectProxy& rotState = rotIter->second;
							boneState.boneRot.push_back(BoneRotState((float)((double)rotState[1]), (float)((double)rotState[2])));
						}

						//string _boneFlag("default");
						AddBoneState(_boneName, boneFlag, animID, boneState);
					}
				}

				if (_boneName.empty())
				{
					return;
				}
				boneName += _boneName;

				boneInfo.name = _boneName;
				boneInfo.flag = boneFlag;
				boneInfo.index = boneIndex;

				boneInfo.position = "";

				boneInfo.bx = bx;
				boneInfo.by = by;
				boneInfo.bz = bz;

				auto iter = m_boneInfoMap.find(pair<BoneName, BoneFlag>(_boneName, boneFlag));
				if (iter == m_boneInfoMap.end())
				{
					vector<BoneInfo> boneInfos;
					boneInfos.push_back(boneInfo);
					m_boneInfoMap.insert(make_pair(pair<BoneName, BoneFlag>(_boneName, boneFlag), boneInfos));
					/*vector<int> bonePositionInfo;
					bonePositionInfo.push_back(boneIndex);
					m_bonePositionInfoMap.insert(make_pair(pair<string, string>(_boneName, boneInfo.flag), bonePositionInfo));*/
				}
				else
				{
					vector<BoneInfo>& boneInfos = iter->second;
					boneInfos.push_back(boneInfo);
					//CountBonePosition(_boneName,boneFlag,boneInfos);
				}

			}
		}
	}

	void BMaxAnimGenerator::UpdateBonePositionInfo()
	{
		Vector3 centerBonePosition;
		if (m_vRightBonePosition == m_vLeftBonePosition)
		{
			return;
		}
		else
		{
			centerBonePosition.x = (m_vLeftBonePosition.x - m_vRightBonePosition.x) / 2 + m_vRightBonePosition.x;
			centerBonePosition.y = (m_vLeftBonePosition.y - m_vRightBonePosition.y) / 2 + m_vRightBonePosition.y;
			centerBonePosition.z = (m_vLeftBonePosition.z - m_vRightBonePosition.z) / 2 + m_vRightBonePosition.z;
		}
		for (auto& boneInfosIter : m_boneInfoMap)
		{
			auto& boneInfos = boneInfosIter.second;
			for (auto& boneInfo : boneInfos)
			{
				if (boneInfo.bz > centerBonePosition.z)
				{
					boneInfo.position = "left";
				}
				else if (boneInfo.bz == centerBonePosition.z)
				{
					boneInfo.position = "center";
				}
				else if (boneInfo.bz < centerBonePosition.z)
				{
					boneInfo.position = "right";
				}
			}

		}
	}

	void BMaxAnimGenerator::FillAnimations()
	{

		UpdateBonePositionInfo();
		//m_bHasAnimation = true;

		// static animation 0
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = 0;
		anim.timeEnd = 0;
		anim.animID = 0;
		m_pParser->AddAnimation(anim);

		FillAnimation(ANIM_WALK, 4000, 5000, 4.0f, true);
		FillAnimation(ANIM_FLY, 4000, 5000, 4.0f, true);


		// walk animations 
		/*AutoAddWalkAnimation(ANIM_WALK, 4000, 5000, 4.0f, true);
		AutoAddWalkAnimation(ANIM_WALKBACKWARDS, 13000, 14000, 4.0f, false);*/
	}


	bool BMaxAnimGenerator::CompareBonePosition(BoneInfo firstBone, BoneInfo secondBone)
	{
		if (firstBone.by > secondBone.by)
		{
			return true;
		}

		if (firstBone.by == secondBone.by)
		{
			if (firstBone.bx > secondBone.bx)
			{
				return true;
			}

			if (firstBone.bx == secondBone.bx && firstBone.bz > secondBone.bz)
			{
				return true;
			}
		}
		return false;
	}

	void BMaxAnimGenerator::CountBonePosition(string boneName, BoneFlag boneFlag, const std::vector<BoneInfo>& boneInfos)
	{
		BoneInfo newBoneInfo = boneInfos[boneInfos.size() - 1];
		auto bonePositionInfoIter = m_bonePositionInfoMap.find(pair<BoneName, BoneFlag>(boneName, boneFlag));
		if (bonePositionInfoIter != m_bonePositionInfoMap.end())
		{
			std::vector<int>& bonePositionInfo = bonePositionInfoIter->second;
			for (auto boneInfoIter : boneInfos)
			{
				BoneInfo& boneInfo = boneInfoIter;
				if (CompareBonePosition(newBoneInfo, boneInfo))
				{
					for (auto bonePositionIter = bonePositionInfo.begin(); bonePositionIter != bonePositionInfo.end(); bonePositionIter++)
					{
						int& BoneIndex = *bonePositionIter;
						if (BoneIndex == boneInfo.index)
						{
							bonePositionInfo.insert(bonePositionIter, newBoneInfo.index);
							break;
						}
					}
				}
			}
		}
		else
			return;
	}

	float BMaxAnimGenerator::GetBoneRotSign(const string& boneName, const string& boneFlag, const string& bonePosition, int animID, float boneAxisZ)
	{
		if (boneName == "wheel")
		{
			return boneAxisZ > 0 ? 1.f : -1.f;
		}
		if (boneName == "wing")
		{
			if (bonePosition == "left")
			{
				return -1.f;
			}
		}
		return 1.f;
	}

	void BMaxAnimGenerator::FillAnimation(int nAnimID, int nStartTime, int nEndTime, float fMoveSpeed, bool bMoveForward)
	{
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = nStartTime;
		anim.timeEnd = nEndTime;
		anim.animID = nAnimID;
		anim.moveSpeed = fMoveSpeed;
		int animIndex = m_pParser->GetAnimationsCount();
		int nAnimLength = anim.timeEnd - anim.timeStart;

		for (auto boneInfos : m_boneInfoMap)
		{
			string boneName = boneInfos.first.first;
			string boneFlag = boneInfos.first.second;
			auto boneStatesIter = s_boneStates.find(pair<BoneName, BoneFlag>(boneName, boneFlag));
			if (boneStatesIter != s_boneStates.end())
			{
				auto boneStates = boneStatesIter->second;
				auto boneState = boneStates.find(nAnimID);
				if (boneState != boneStates.end())
				{
					for (auto boneInfo : boneInfos.second)
					{
						auto& bone = m_pParser->m_bones[boneInfo.index];
						Bone* pBone = bone->GetBone();

						Vector3 vAxis = bone->GetAxis();
						Quaternion q;
						pBone->rot.used = true;
						int nFirstRotSize = (int)pBone->rot.times.size();

						float fRotSign = GetBoneRotSign(boneName, boneFlag, boneInfo.position, nAnimID, vAxis.z);
						/*float fRotSign = (vAxis.z > 0) ? 1.f : -1.f;
						if (!bMoveForward)
						fRotSign = -fRotSign;*/

						//Vector3 rotAxis = GetRotAxis(boneName, boneFlag, nAnimID, vAxis);

						BoneState& _boneState = boneState->second;

						for (auto rotState : _boneState.boneRot)
						{
							q.FromAngleAxis(Radian(rotState.angle * fRotSign), vAxis);
							pBone->rot.times.push_back(anim.timeStart + (int)(nAnimLength * rotState.time));
							pBone->rot.data.push_back(q);
						}


						pBone->rot.ranges.resize(animIndex + 1, AnimRange(0, 0));
						pBone->rot.ranges[animIndex] = AnimRange(nFirstRotSize, max(nFirstRotSize, (int)pBone->rot.times.size() - 1));
					}
				}
			}
		}
		m_pParser->m_anims.push_back(anim);
	}
}