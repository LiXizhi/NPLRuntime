#pragma once
#include "NPLHelper.h"
#include "NPLTable.h"
#include "ParaXModel/ParaXModel.h"

namespace ParaEngine
{
	class Bone;
	class BMaxParser;

	//typedef int BoneIndex;
	typedef string BoneName;
	typedef string BoneFlag;

	struct BoneRotState
	{
		BoneRotState(float time,float angle){
			this->time = time;
			this->angle = angle;
		}
		//rot state
		float angle;
		float time;
	};

	//struct BoneTransState
	//{
	//	BoneTransState(float time, float angle){
	//		this->time = time;
	//		this->angle = angle;
	//	}
	//	//trans state
	//	float angle;
	//	float time;
	//};

	//struct BoneScaleState
	//{
	//	BoneScaleState(float time, float angle){
	//		this->time = time;
	//		this->angle = angle;
	//	}
	//	//scale state
	//	float angle;
	//	float time;
	//};

	struct BoneState
	{
	public:
		BoneState(){}
		//rot state
		vector<BoneRotState> boneRot;
		//vector<BoneTransState> boneTrans;
		//vector<BoneScaleState> boneScale;
	};

	struct BoneInfo
	{
		BoneInfo(){}
		// the bone is "leg","arm","wing","wheel"
		string name;
		// the flag of bone.
		string flag;
		//the bone position,"right","left","center"
		string position;
		int index;
		float bx;
		float by;
		float bz;
	};

	/** BMax Animation generator
	1. x+ is the front, z+ right, z- left
	2. z axis's bone blocks should face inward
	3. x axis's bone block should face x+ direction.
	*/
	class BMaxAnimGenerator
	{
	public:
		BMaxAnimGenerator();
		BMaxAnimGenerator(BMaxParser *pParser);
		~BMaxAnimGenerator();
	public:
		/**
		* @param boneName: output bone name
		*/
		void ParseParameters(NPL::NPLObjectProxy& boneInfo, int boneIndex, string& boneName);
		void FillAnimations();
		ModelAnimation* FillWalkAnimation();
		void FillAnimation(int nAnimID, int nStartTime, int nEndTime, float fMoveSpeed, bool bMoveForward);
		void CountBonePosition(string boneName, BoneFlag boneFlag, const std::vector<BoneInfo>& boneInfos);
		void UpdateBonePositionInfo();
		
	private:

		void InitBoneState();
		bool CompareBonePosition(BoneInfo firstBone,BoneInfo secondBone);
		bool GetBonePosition(int boneIndex);

		void AddBoneState(const string& boneName,const string& boneFlag,int animID,BoneState boneState);
		void AddWheelBoneState();
		void AddWalkBoneState();
		void AddFlyBoneState();

		//Vector3 GetRotAxis(string boneName, string boneFlag, int animID, Vector3 boneAxis);
		float GetBoneRotSign(const string& boneName, const string& boneFlag, const string& bonePosition, int animID, float boneAxisZ);

	private:
		static std::map<pair<BoneName, BoneFlag>, std::map<int, BoneState>> s_boneStates;
		static bool s_bInitedBoneState;

		BMaxParser *m_pParser;
		
		std::map<pair<BoneName, BoneFlag>, std::vector<BoneInfo>> m_boneInfoMap;
		std::map<pair<BoneName, BoneFlag>, std::vector<int>> m_bonePositionInfoMap;

		bool m_bHasSetMaxMinPosition;
		Vector3 m_vRightBonePosition;
		Vector3 m_vLeftBonePosition;
	};
}
