#pragma once
#include "BMaxNode.h"

namespace ParaEngine
{
	class Bone;

	/** bone node in bmax */
	struct BMaxFrameNode : public BMaxNode
	{
	public:
		BMaxFrameNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_, int32 boneIndex);
		virtual ~BMaxFrameNode();
	public:
		BMaxFrameNode* GetParent();
		int32 GetParentIndex() const;
		void SetParentIndex(int32 val);
		bool HasParent();
		/** we will automatically set the bone name according to topology, if it is not set before. */
		void AutoSetBoneName();
		ParaEngine::Bone* GetBone();
		/** @param bRefresh: if true, we will refresh */
		ParaEngine::Bone* GetParentBone(bool bRefresh = true);
		virtual int GetBoneIndex();
		virtual void SetBoneIndex(int nIndex);
		/** get the bone node interface if it is*/
		virtual BMaxFrameNode* ToBoneNode();

		int GetParentBoneIndex();
		
		 /** Returns true if this object is a parent, (or grandparent and so on to any level), of the given child. */
		bool IsAncestorOf(BMaxFrameNode* pChild);

		/** if bone chain is recursive, we will break it. return true if cycles is found broken*/
		bool BreakCycles();

		/** get the bone color */
		virtual DWORD GetColor();
		virtual void SetColor(DWORD val);

		/** get child bone count */
		int GetChildCount();
		BMaxFrameNode* GetChild(int nIndex);
		/** -1 if not found. */
		int GetChildIndexOf(BMaxFrameNode* pChild);
		int GetChildIndex();
		/** add child node */
		void AddChild(BMaxFrameNode* pNode);

		void UpdatePivot();

		/** get the bone axis. 6 possible directions */
		Vector3 GetAxis();
	protected:
		/** the bone's color if not calculated before. */
		DWORD CalculateBoneColor();

	public:
		int m_nParentIndex;
		ref_ptr<ParaEngine::Bone> m_pBone;
		/** child node position index*/
		vector<int> m_children;
	};
	typedef ref_ptr<BMaxFrameNode> BMaxFrameNodePtr;
}



