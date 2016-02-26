#pragma once

#include <vector>

#define MAX_NODE_NUM	100
#define MAX_CHILDBONE	6
namespace ParaEngine
{
	using namespace std;

	class Quaternion;
	class CParaXModel;

	/**
	* BioVision Hierarchy (BVH) motion capture file serializer for ParaX file. 
	* it can also export as CSM (character studio model format), which is a much simpler one. 
	* Usage: call InitHierachy() and then call SaveBVH().
	* in order to use the exported BVH file. First create a biped in 3dsmax 8 or above, then load the BVH file, save the animation as bip file.
	* open or create your character, load the .bip file to the biped of your character. And it is done, 3dsmax will automatically 
	* fit the animation in .bip file with your character bone structure.
	* please see SaveBVH() for more information.
	*/
	class CBVHSerializer
	{
	public:
		CBVHSerializer();
		~CBVHSerializer(void);
		
		enum BVH_NODES
		{
			Hips = 0,//

			LeftHip,//lfemur		
			LeftKnee,//	ltibia
			LeftAnkle,//	lfoot
			LeftToe,//     ltoe

			RightHip,//	rfemur
			RightKnee,//	rtibia
			RightAnkle,//	rfoot
			RightToe,//    rtoe

			Chest,//		upperback
			Chest2,//	thorax
			Chest3,//      spine_bone3
			Chest4,//      spine_bone4
			Chest5,//      spine_bone5
			Chest6,//      spine_bone6
			Chest7,//      spine_bone7

			LeftCollar,//	lshoulderjoint
			LeftShoulder,//	lhumerus
			LeftElbow,//	lradius
			LeftWrist,//	lwrist


			LeftFinger0,//    lThumb_0Bone
			LeftFinger01,//   lThumb_1Bone
			LeftFinger02,//   lThumb_2Bone

			LeftFinger1,//     lfinger1_0Bone
			LeftFinger11,//    lfinger1_1Bone
			LeftFinger12,//    lfinger1_2Bone
			//.
			//.
			//.
			LeftFinger4,//     lfinger4_0Bone
			LeftFinger41,//    lfinger4_1Bone
			LeftFinger42,//    lfinger4_2Bone


			RightCollar,//	rshoulderjoint
			RightShoulder,//	rhumerus
			RightElbow,//	rradius
			RightWrist,//	rwrist

			RightFinger0,//    rThumb_0Bone
			RightFinger01,//   rThumb_1Bone
			RightFinger02,//   rThumb_2Bone

			RightFinger1,//     rfinger1_0Bone
			RightFinger11,//    rfinger1_1Bone
			RightFinger12,//    rfinger1_2Bone
			//.
			//.
			//.
			RightFinger4,//     rfinger4_0Bone
			RightFinger41,//    rfinger4_1Bone
			RightFinger42,//    rfinger4_2Bone

			Head,//		head
			Neck,//		neck

			MAX_BVH_NODE,
		};

		struct MotionCaptureNode
		{
			/// name of the motion capture node, it is mostly predefined. 
			string m_nodeName;
			/// to which bone this motion capture node is mapped to.By default this is -1, which means that it maps to nothing. 
			int m_nBoneIndex;
		};

		struct UniBone{
			/// index of the parent bone
			int m_nParentIndex;
			/// array of index of the child bones.
			vector<int> m_childBones;
			/// number of bones.
			int m_nSubBones;
			/// how many levels are the child objects
			int m_nSubLevels;
			/// relative offset to the parent bone
			Vector3 m_vOffsetToParent;
			/// this is a pre-calculated value for |m_vOffsetToParent|
			float m_fBoneLength;
			/// absolute offset in object space. 
			Vector3 m_vAbsOffset;
			/// maker name for BVH or CSM, default is bone ID.
			string m_sMarkerName;
			/// a bone is a critical bone, if this bone or any of its child has a marker name. 
			bool m_bCriticalBone;
		public:
			inline int GetChildCount(){
				return (int)m_childBones.size();
			};
			inline bool IsUnKnownBone(){return m_sMarkerName.empty();};
			inline float GetBoneLength(){ return m_fBoneLength;}
			/// get relative offset to the parent bone
			inline const Vector3& GetOffset(){ return m_vOffsetToParent;}
		};

		struct BoneChildCountCompare
		{	
			BoneChildCountCompare(vector<UniBone>& bones)
			{
				m_pBones = &bones;
			}

			bool operator()(const int& _Left, const int& _Right) const
			{	// apply operator> to operands
				return (((*m_pBones)[_Left].m_nSubLevels > (*m_pBones)[_Right].m_nSubLevels) ||
					((((*m_pBones)[_Left].m_nSubLevels == (*m_pBones)[_Right].m_nSubLevels)) && (*m_pBones)[_Left].m_fBoneLength > (*m_pBones)[_Right].m_fBoneLength) );
			}
		private:
			vector<UniBone>* m_pBones;
		};

		struct BoneMarkerCompare
		{	
			BoneMarkerCompare(vector<UniBone>& bones)
			{
				m_pBones = &bones;
			}

			bool operator()(const int& _Left, const int& _Right) const
			{	// apply operator> to operands

				if((*m_pBones)[_Right].m_sMarkerName.empty())
					return true;
				else if((*m_pBones)[_Left].m_sMarkerName.empty())
					return true;
				else if((*m_pBones)[_Left].m_sMarkerName[0] == 'L' && (*m_pBones)[_Right].m_sMarkerName[0] == 'R')
					return true;
				else if((*m_pBones)[_Right].m_sMarkerName[0] == 'C' || (*m_pBones)[_Right].m_sMarkerName[0] == 'N')
					return true;
				else
					return false;
			}
		private:
			vector<UniBone>* m_pBones;
		};

		/** predefined CSM file node. */
		enum CharacterStudioNodeEnum{
			C7=0,CLAV,LANK,LBHD, LBWT, LELB, LFHD, LFIN, LFWT, LKNE, LMT5, LSHO, LTOE, LWRE, LWRI, RANK, RBHD, RBWT, RELB, RFHD, RFIN, RFWT, RKNE, RMT5, RSHO, RTOE, RWRE, RWRI, STRN, T10, 
		};
		enum UniversalNodeEnum{
			UN_Root,
			UN_Root1,
			UN_Root2,
			UN_Spine0,
				UN_Left_Thigh,
					UN_Left_Calf,
				UN_Right_Thigh,
					UN_Right_Calf,
				UN_Spine1,UN_Spine2,UN_Spine3,UN_Spine4,
					UN_Neck,
						UN_Head,
						UN_Left_Clavicle,
							UN_Left_UpperArm,
						UN_Right_Clavicle,
							UN_Right_UpperArm,
		};
		
	public:
		/** not implemented. because i have no idea where the marker point should be. */
		void SaveCSM(const string& sFileName);

		/**
		* must call this function before save to any of the supported file format
		* it will read the bone hierarchy of the animated mesh in to some intermediary structure.
		* @param xmesh 
		* @return 
		*/
		bool InitHierachy(CParaXModel& xmesh);

		/**
		* save to BioVision Hierarchy file. InitHierachy() must be called before this function is called.
		* if the marker file is not generated, it will generate and return without exporting animations, this gives the user 
		* the chance to manually examine the maker file. When this function is called a second time, the serializer will be 
		* able to locate an existing marker file, and it will generate all animations with the marked bones.
		* @param sFileName output BVH file name
		* @param sMarkerNameFile if this is blank, a default marker name file will be saved in "sFileName.marker" if it does not exist.
		* however if the default marker exists, it will be used. When generating the default marker file, the serializer automatically 
		* generate BVH marker position. But it does not guarantee that the data is correct, the user should manually check the marker file.
		* if sMarkerNameFile is a valid marker file, BVH file will use this marker file and will not generate new ones or use default ones.
		* @remark: at least the "Hips" marker should be present in the marker file, in order for animation to be generated. 
		* however, all known marker position must be present in order to be used by 3dsmax. Please see the BVH.rtf document in the 3dsmax CD.
		* @remark: marker file format: it is really simple format. Each line denote a bone that begins with its ID. 
		* When a maker file is automatically generated, just append any of the known marker name in front of each line. such as \n
		* Hips 16: ChildCount 2 SubBones 8 SubLevel 4 vAbsPos(0.1 0.6 0.0) BoneLength:0.61 \n
		*	RightHip 19: ChildCount 1 SubBones 3 SubLevel 3 vAbsPos(0.1 0.6 -0.1) BoneLength:0.12 \n
		*		RightKnee 34: ChildCount 1 SubBones 2 SubLevel 2 vAbsPos(0.1 0.3 -0.1) BoneLength:0.25 \n
		* @remark: currently we support all traditional BVH nodes, as well as those supported by 3dsmax 8, such as Chest2, LeftToe, LeftFinger0, etc.
		* we also allow the position of Hips and Chest node to be arbitrary in mesh bone hirarchy. i.e. the chest bone can be the parent or sibling of the Hips bones. 
		*/
		bool SaveBVH(const string& sFileName, const string& sMarkerNameFile);

	private:
		/** all nodes. */
		vector <MotionCaptureNode> m_nodes;

		/** universal nodes */
		MotionCaptureNode m_UniversalNodes[MAX_NODE_NUM];

		/** temporary bidirectional bones from the input*/
		vector<UniBone> m_bones;
		int m_BVHnodes[MAX_BVH_NODE];
		/** current mesh object. */
		CParaXModel* m_pXMesh;
		
		/** top level bone index in the parax file.*/
		int m_nRootBoneIndex;

		/** at which FPS to export the animation.*/
		int m_nFPS;

		/** since we have different default orientations, rotate the root bone to face positive X. */
		bool m_bRotateY;

		/** whether to escape unknown BVH bones.*/
		bool m_bEscapeUnknownBones;

		/** whether to escape unmarked bones.*/
		bool m_bEscapeUnmarkedBones;

		/** whether export offset position besides the root bone in the motion section of BVH file*/
		bool m_bExportBVHPosition;

		/** Swapped chest bone with Hip bone. In some existing models, the hip is not the root bone, but a spine(Chest) is. 
		* in such cases, we need to swap the Hip with Chest when exporting BVH files. Since in BVH, the Hip bone is always the root bone.*/
		bool m_bSwapChestHip;

		int m_nHipBoneIndex;
		int m_nChestBoneIndex;

		/** build the internal m_bones presentation for efficient parsing. 
		* always call this function, before any of the following functions are called.
		*/
		bool BuildInternalBones();

		/** build a universal biped bone hierarchy from unknown bone topology. */
		bool BuildBipedUniversalBones();

		/** get the parax bone index, that a universal bone is mapped to.*/
		inline int GetBoneIndex(int nIndex){return m_UniversalNodes[(UniversalNodeEnum)nIndex].m_nBoneIndex;};

		/** number of bones in the ParaX file*/
		int GetBoneCount(){return (int)m_nodes.size();};

		/** dump to log file the universal bone hierarchy for debugging purposes. */
		bool DumpUniversalBones();
		/** dump to log file the original parax mesh bones. This function is called recursively. */
		bool DumpParaXBones(int nLevel, int nBoneIndex, CParaFile* pFile = NULL);
		/** write the BVH node.*/
		bool WriteBVHNode(int nLevel, int nBoneIndex, CParaFile* pFile, bool bEscapeUnknownBones = true);
		bool WriteBVHNodeAnimation(int nLevel, int nBoneIndex, CParaFile* pFile, bool bEscapeUnknownBones = true);

		bool BeginBVHNode(CParaFile* pFile, int nLevel, const string& sMarker, const Vector3& vOffset, bool bExportPos, bool bExportRot);
		bool EndBVHNode(CParaFile* pFile, int nLevel,const string& sMarker);
		bool WriteQuaternion(CParaFile* pFile, const Quaternion& q, bool bRotY = false);
		bool WriteEndSite(CParaFile* pFile, int nLevel, int nBoneIndex);


		bool WriteHips(int nLevel, bool bHierachy, CParaFile* pFile);
		bool WriteLeftHip(int nLevel, bool bHierachy, CParaFile* pFile);
			bool WriteLeftKnee(int nLevel, bool bHierachy, CParaFile* pFile);
				bool WriteLeftAnkle(int nLevel, bool bHierachy, CParaFile* pFile);
					bool WriteLeftToe(int nLevel, bool bHierachy, CParaFile* pFile);
				
		bool WriteRightHip(int nLevel, bool bHierachy, CParaFile* pFile);
			bool WriteRightKnee(int nLevel, bool bHierachy, CParaFile* pFile);
				bool WriteRightAnkle(int nLevel, bool bHierachy, CParaFile* pFile);
					bool WriteRightToe(int nLevel, bool bHierachy, CParaFile* pFile);
			

		bool WriteChest(int nLevel, bool bHierachy, CParaFile* pFile);
		bool WriteChest2(int nLevel, bool bHierachy, CParaFile* pFile);
			bool WriteNeck(int nLevel, bool bHierachy, CParaFile* pFile);
				bool WriteHead(int nLevel, bool bHierachy, CParaFile* pFile);
			bool WriteLeftCollar(int nLevel, bool bHierachy, CParaFile* pFile);
				bool WriteLeftShoulder(int nLevel, bool bHierachy, CParaFile* pFile);
					bool WriteLeftElbow(int nLevel, bool bHierachy, CParaFile* pFile);
						bool WriteLeftWrist(int nLevel, bool bHierachy, CParaFile* pFile);
							bool WriteLeftFinger0(int nLevel, bool bHierachy, CParaFile* pFile);
			bool WriteRightCollar(int nLevel, bool bHierachy, CParaFile* pFile);
				bool WriteRightShoulder(int nLevel, bool bHierachy, CParaFile* pFile);
					bool WriteRightElbow(int nLevel, bool bHierachy, CParaFile* pFile);
						bool WriteRightWrist(int nLevel, bool bHierachy, CParaFile* pFile);
							bool WriteRightFinger0(int nLevel, bool bHierachy, CParaFile* pFile);
		


		/** This function is called recursively. */
		int ComputeSubBonesCount(int nBoneIndex);

		/** save the marker file */
		bool SaveMarkerFile(const string& sFileName);
		/** load marker file. Each line in the marker file represent a bone.
		One can append before each line a BVH or CSM marker name, hence when exported, the bone will have this name.*/
		bool LoadMarkerFile(const string& sFileName);

		/**
		* get the closest marked parent bone index
		* @param nIndex index of the current bone. 
		* @return the closest marked parent bone index. this may be -1, if not found. 
		*/
		int GetMarkedParentIndex(int nIndex);

		/** update all critical bones. This function is called recursively. */
		bool UpdateCriticalBone(int nBoneIndex);

		/** for unknown reasons, 3dsmax only accept bones in the given order(first left bones and then right bones). */
		bool SortBVHMarker();
		/** call, bones must be build and sorted before this function is called. automatically generate BVH marker */
		bool AutoGenerateBVHMarker(int nBoneIndex, bool bGenChest, bool bGenHips);

		/** generate marks for hip and left/right leg bones if nBoneIndex looks like a hip*/
		bool GenHipMarker(int nBoneIndex, int* ChestBoneIndex);
		/** generate marks for chest if nBoneIndex looks like a chest*/
		bool GenChestMarker(int nBoneIndex);

	};

}
