#pragma once
#include <vector>
#include <string>

namespace ParaEngine
{
	using namespace std;

	struct AnimInfo
	{
		AnimInfo() :id(0), startTick(0), endTick(0), loopType(0), fSpeed(0.f) {};
		//string name;
		int id;
		int startTick;
		int endTick;
		int loopType;
		float fSpeed;

		bool operator<(const AnimInfo &rhs) const { return startTick < rhs.startTick || endTick < rhs.endTick; }
	};

	/** for reading anim xml file, which is meta file when export FBX model.
	<anims>
		<anim startTick="0" startTick="3" loopType="0" />
		<anim startTick="4" startTick="7" loopType="0" />
	</anims>
	*/
	class FBXModelInfo
	{
	public:
		FBXModelInfo(void);
		~FBXModelInfo(void);

	public:
		/**
		* load data from file to class data structure. 
		*/
		bool LoadFromFile(const string& filename);

		/**
		* load data from file to class data structure. 
		*/
		bool LoadFromBuffer(const char* pData, int nSize);

		/** get the number of anims*/
		int GetAnimCount(){ return m_Anims.size(); }

	public:
		string m_sFilename;

		// store the anims info
		vector<AnimInfo> m_Anims;

		Vector3 m_vNeckYawAxis;
		Vector3 m_vNeckPitchAxis;
	};

}
