//-----------------------------------------------------------------------------
// Class:Block max frame node
// Authors:	LiXizhi
// Emails:	lixizhi@yeah.net
// Date:	2015.9.26
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockDirection.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BMaxParser.h"
#include "BMaxFrameNode.h"
using namespace ParaEngine;


BMaxFrameNode::BMaxFrameNode(BMaxParser* pParser, int16 x_, int16 y_, int16 z_, int32 template_id_, int32 block_data_, int32 boneIndex)
	:BMaxNode(pParser, x_, y_, z_, template_id_, block_data_), m_nParentIndex(-1)
{
	m_pBone.reset(new ParaEngine::Bone());
	m_pBone->nIndex = boneIndex;
}

void ParaEngine::BMaxFrameNode::UpdatePivot()
{
	Vector3 pivot(x - m_pParser->m_centerPos.x + BlockConfig::g_blockSize * 0.5f, y + BlockConfig::g_blockSize * 0.5f, z - m_pParser->m_centerPos.z + BlockConfig::g_blockSize * 0.5f);
	m_pBone->bUsePivot = true;
	m_pBone->pivot = pivot * m_pParser->m_fScale;
	m_pBone->flags = ParaEngine::Bone::BONE_USE_PIVOT;
}

Vector3 ParaEngine::BMaxFrameNode::GetAxis()
{
	BlockDirection::Side mySide = BlockDirection::GetBlockSide(block_data);
	Int32x3 offset = BlockDirection::GetOffsetBySide(BlockDirection::GetOpSide(mySide));
	return Vector3((float)offset.x, (float)offset.y, (float)offset.z);
}

int BMaxFrameNode::GetBoneIndex()
{
	return GetBone()->GetBoneIndex();
}

int BMaxFrameNode::GetParentBoneIndex()
{
	auto pParent = GetParent();
	return (pParent) ? pParent->GetBoneIndex() : -1;
}

BMaxFrameNode* BMaxFrameNode::GetParent()
{
	if (m_nParentIndex >= 0)
	{
		auto pParent = m_pParser->m_nodes[m_nParentIndex];
		if (pParent)
			return pParent->ToBoneNode();
	}
	return NULL;
}

ParaEngine::BMaxFrameNode::~BMaxFrameNode()
{

}

int32 ParaEngine::BMaxFrameNode::GetParentIndex() const
{
	return m_nParentIndex;
}

void ParaEngine::BMaxFrameNode::SetParentIndex(int32 val)
{
	m_nParentIndex = val;
	auto pParent = GetParent();
	if (pParent)
	{
		pParent->AddChild(this);
		m_pBone->parent = pParent->GetBoneIndex();
	}
	else
		m_pBone->parent = -1;
}

bool ParaEngine::BMaxFrameNode::HasParent()
{
	return m_nParentIndex >= 0;
}

void ParaEngine::BMaxFrameNode::AutoSetBoneName()
{
	Bone* pBone = GetBone();
	if (pBone->GetName().empty())
	{
		std::ostringstream stream;
		stream << "bone";
		BMaxFrameNode* pChild = this;
		int nSide = -1; // left or right bone
		int nMultiChildParentCount = 0;
		int nParentCount = 0;
		while (pChild)
		{
			BMaxFrameNode* pParent = pChild->GetParent();
			if (pParent && pParent->GetChildCount()>1)
			{
				nMultiChildParentCount++;
				if (pParent->z > pChild->z)
					nSide = 1;
				else if (pParent->z < pChild->z)
					nSide = 0;
			}
			pChild = pParent;
			++nParentCount;
		}
		if (nParentCount > 0)
			nParentCount --;
		// left or right side
		if (nSide == 0)
			stream << "_left";
		else if (nSide == 1)
			stream << "_right";
		if (nSide >= 0 && nMultiChildParentCount>1)
		{
			// how many multi child parent to distinguish hand and feet, etc. 
			stream << "_mp" << nMultiChildParentCount;
		}
		// how many parent levels
		stream << "_p" << nParentCount;
		
		// check for hand and feet and apply inverse IK
		auto parent = GetParent();
		if (parent && parent->GetChildCount() == 1)
		{
			parent = parent->GetParent();
			if (parent && parent->GetChildCount() == 1)
			{
				parent = parent->GetParent();
				if (parent && parent->GetChildCount() > 1)
				{
					stream << "_IK";
				}
			}
		}
		std::string sName = stream.str();
		int nCount = m_pParser->GetNameAppearanceCount(sName, true);
		if (nCount > 0){
			stream << "_" << nCount;
			sName = stream.str();
		}
		pBone->SetName(sName);
	}
}

ParaEngine::Bone* ParaEngine::BMaxFrameNode::GetBone()
{
	return m_pBone.get();
}

void ParaEngine::BMaxFrameNode::SetBoneIndex(int nIndex)
{
	BMaxNode::SetBoneIndex(nIndex);
}

BMaxFrameNode* ParaEngine::BMaxFrameNode::ToBoneNode()
{
	return this;
}

bool ParaEngine::BMaxFrameNode::IsAncestorOf(BMaxFrameNode* pChild)
{
	while (pChild)
	{
		if (pChild == this)
			return true;
		else if (!pChild->HasParent())
			return false;
		else
			pChild = pChild->GetParent();
	}
    return false;
}

bool ParaEngine::BMaxFrameNode::BreakCycles()
{
	if (HasParent())
	{
		if (IsAncestorOf(GetParent()))
		{
			SetParentIndex(-1);
		}
	}
	return true;
}

DWORD ParaEngine::BMaxFrameNode::CalculateBoneColor()
{
	if (m_color != 0)
		return m_color;
	DWORD color = Color::White;
	auto pBlockWorld = BlockWorldClient::GetInstance();
	BlockDirection::Side mySide = BlockDirection::GetBlockSide(block_data);

	int index = GetIndex();
	BMaxFrameNode* pParentNode = GetParent();
	
	// always start from the opposite side
	int myOppositeSide = (int)BlockDirection::GetOpSide(mySide);
	
	for (int i = 0; i < 6; i++)
	{
		BlockDirection::Side side = BlockDirection::GetBlockSide((myOppositeSide + i) % 6);

		if (side != mySide || pParentNode == NULL)
		{
			BMaxNode* neighbourNode = GetNeighbour(side);
			if (neighbourNode && !neighbourNode->HasBoneWeight())
			{
				auto next_node_template = pBlockWorld->GetBlockTemplate((uint16)(neighbourNode->template_id));
				if (next_node_template && next_node_template->isSolidBlock() && neighbourNode->template_id != BMaxParser::BoneBlockId)
				{
					color = next_node_template->GetBlockColor(neighbourNode->block_data);
					SetColor(color);
					return color;
				}
			}
		}
	}

	if (pParentNode!=NULL)
	{
		color = pParentNode->GetColor();
	}
	SetColor(color);
	return color;
}

DWORD ParaEngine::BMaxFrameNode::GetColor()
{
	if (m_color == 0)
	{
		CalculateBoneColor();
	}
	return m_color;
}


void ParaEngine::BMaxFrameNode::SetColor(DWORD val)
{
	BMaxNode::SetColor(val);
}

int ParaEngine::BMaxFrameNode::GetChildCount()
{
	return (int)m_children.size();
}

BMaxFrameNode* ParaEngine::BMaxFrameNode::GetChild(int nIndex)
{
	if ((int)(m_children.size()) > nIndex)
	{
		return m_pParser->m_bones[m_children[nIndex]].get();
	}
	return NULL;
}

int ParaEngine::BMaxFrameNode::GetChildIndexOf(BMaxFrameNode* pChild)
{
	int nIndex = pChild->GetIndex();
	for (int i = 0; i < (int)m_children.size(); ++i)
	{
		if (m_children[i] == nIndex)
			return i;
	}
	return -1;
}

int ParaEngine::BMaxFrameNode::GetChildIndex()
{
	auto parent = GetParent();
	return (parent) ? parent->GetChildIndexOf(this) : -1;
}

void ParaEngine::BMaxFrameNode::AddChild(BMaxFrameNode* pNode)
{
	int nIndex = pNode->GetIndex();
	for (auto childIndex: m_children)
	{
		if (childIndex == nIndex)
			return;
	}
	m_children.push_back(nIndex);
}

ParaEngine::Bone* BMaxFrameNode::GetParentBone(bool bRefresh)
{
	if (bRefresh)
	{
		SetParentIndex(-1);
		int cx = x;
		int cy = y;
		int cz = z;
		BlockDirection::Side side = BlockDirection::GetBlockSide(block_data);
		Int32x3 offset = BlockDirection::GetOffsetBySide(side);
		int dx = offset.x;
		int dy = offset.y;
		int dz = offset.z;
		int maxBoneLength = BMaxParser::MaxBoneLengthHorizontal;
		if (dy != 0){
			maxBoneLength = BMaxParser::MaxBoneLengthVertical;
		}
		for (int i = 1; i <= maxBoneLength; i++)
		{
			int x = cx + dx*i;
			int y = cy + dy*i;
			int z = cz + dz*i;
			BMaxFrameNode* parent_node = m_pParser->GetFrameNode(x, y, z);
			if (parent_node)
			{
				BlockDirection::Side parentSide = BlockDirection::GetBlockSide(parent_node->block_data);
				BlockDirection::Side opSide = BlockDirection::GetOpSide(parentSide);
				if (opSide != side || (dx + dy + dz) < 0)
				{
					// prevent acyclic links
					if (!IsAncestorOf(parent_node))
					{
						SetParentIndex(parent_node->GetIndex());
					}
				}
				break;
			}
		}
	}
	auto pParent = GetParent();
	return (pParent) ? pParent->GetBone() : NULL;
}


