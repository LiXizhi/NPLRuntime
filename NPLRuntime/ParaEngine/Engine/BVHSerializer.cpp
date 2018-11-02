//-----------------------------------------------------------------------------
// Class:	CBVHSerializer
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.7.29
// Note: serialization functions for BVH file.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "ParaXModel/AnimTable.h"
#include <queue>
#include <algorithm>
#include "BVHSerializer.h"

using namespace ParaEngine;

CBVHSerializer::CBVHSerializer()
{
	m_nFPS = 30;
	m_bRotateY = true;
	m_bEscapeUnknownBones = true;
	m_bEscapeUnmarkedBones = true;
	m_bExportBVHPosition = false;
	m_bSwapChestHip = false;
	m_UniversalNodes[UN_Root].m_nodeName = "UN_Root";
	m_UniversalNodes[UN_Root1].m_nodeName = "UN_Root1";
	m_UniversalNodes[UN_Root2].m_nodeName = "UN_Root2";
	m_UniversalNodes[UN_Spine0].m_nodeName = "UN_Spine0";
	m_UniversalNodes[UN_Left_Thigh].m_nodeName = "	UN_Left_Thigh";
	m_UniversalNodes[UN_Left_Calf].m_nodeName = "		UN_Left_Calf";
	m_UniversalNodes[UN_Right_Thigh].m_nodeName = "	UN_Right_Thigh";
	m_UniversalNodes[UN_Right_Calf].m_nodeName = "		UN_Right_Calf";
	m_UniversalNodes[UN_Spine1].m_nodeName = "	UN_Spine1";
	m_UniversalNodes[UN_Spine2].m_nodeName = "	UN_Spine2";
	m_UniversalNodes[UN_Spine3].m_nodeName = "	UN_Spine3";
	m_UniversalNodes[UN_Spine4].m_nodeName = "	UN_Spine4";
	m_UniversalNodes[UN_Neck].m_nodeName = "		UN_Neck";
	m_UniversalNodes[UN_Head].m_nodeName = "			UN_Head";
	m_UniversalNodes[UN_Left_Clavicle].m_nodeName = "			UN_Left_Clavicle";
	m_UniversalNodes[UN_Left_UpperArm].m_nodeName = "				UN_Left_UpperArm";
	m_UniversalNodes[UN_Right_Clavicle].m_nodeName = "			UN_Right_Clavicle";
	m_UniversalNodes[UN_Right_UpperArm].m_nodeName = "UN_Right_UpperArm";
}

CBVHSerializer::~CBVHSerializer(void)
{
}

bool CBVHSerializer::SaveMarkerFile(const string& sFileName)
{
	CParaFile file;
	if (file.CreateNewFile(sFileName.c_str()))
	{
		return DumpParaXBones(0, m_nRootBoneIndex, &file);
	}
	return false;
}

bool CBVHSerializer::LoadMarkerFile(const string& sFileName)
{
	for (int i = 0; i<MAX_BVH_NODE; ++i)
	{
		m_BVHnodes[i] = -1;
	}
	CParaFile file(sFileName.c_str());
	if (file.isEof())
		return false;
	char buf[2048];
	while (file.GetNextLine(buf, 2048) > 0)
	{
		char sName[256];
		int nBoneIndex = 0;
		if (_snscanf(buf, 2048, "%s %d", sName, &nBoneIndex) == 2)
		{
			if ((int)m_bones.size()>nBoneIndex)
			{
				string sMarker = sName;
				m_bones[nBoneIndex].m_sMarkerName = sMarker;


				if (sMarker == "Hips")
				{
					m_BVHnodes[Hips] = nBoneIndex;
					m_nHipBoneIndex = nBoneIndex;
					int nParent = GetMarkedParentIndex(nBoneIndex);
					if (nParent >= 0 && m_bones[nParent].m_sMarkerName == "Chest")
					{
						m_bSwapChestHip = true;
						m_nRootBoneIndex = nParent;
					}
					else
					{
						m_nRootBoneIndex = nBoneIndex;
						m_bSwapChestHip = false;
					}
				}
				else if (sMarker == "Chest")
				{
					m_BVHnodes[Chest] = nBoneIndex;
					m_nChestBoneIndex = nBoneIndex;
				}
				else if (sMarker == "LeftHip")
					m_BVHnodes[LeftHip] = nBoneIndex;
				else if (sMarker == "LeftKnee")
					m_BVHnodes[LeftKnee] = nBoneIndex;
				else if (sMarker == "LeftAnkle")
					m_BVHnodes[LeftAnkle] = nBoneIndex;
				else if (sMarker == "LeftToe")

					m_BVHnodes[LeftToe] = nBoneIndex;
				else if (sMarker == "RightHip")
					m_BVHnodes[RightHip] = nBoneIndex;
				else if (sMarker == "RightKnee")
					m_BVHnodes[RightKnee] = nBoneIndex;
				else if (sMarker == "RightAnkle")
					m_BVHnodes[RightAnkle] = nBoneIndex;
				else if (sMarker == "RightToe")
					m_BVHnodes[RightToe] = nBoneIndex;

				else if (sMarker == "Chest")
					m_BVHnodes[Chest] = nBoneIndex;
				else if (sMarker == "Chest2")
					m_BVHnodes[Chest2] = nBoneIndex;
				else if (sMarker == "Chest3")
					m_BVHnodes[Chest3] = nBoneIndex;
				else if (sMarker == "Chest4")
					m_BVHnodes[Chest4] = nBoneIndex;
				else if (sMarker == "Chest5")
					m_BVHnodes[Chest5] = nBoneIndex;
				else if (sMarker == "Chest6")
					m_BVHnodes[Chest6] = nBoneIndex;
				else if (sMarker == "Chest7")
					m_BVHnodes[Chest7] = nBoneIndex;

				else if (sMarker == "LeftCollar")
					m_BVHnodes[LeftCollar] = nBoneIndex;
				else if (sMarker == "LeftShoulder")
					m_BVHnodes[LeftShoulder] = nBoneIndex;
				else if (sMarker == "LeftElbow")
					m_BVHnodes[LeftElbow] = nBoneIndex;
				else if (sMarker == "LeftWrist")
					m_BVHnodes[LeftWrist] = nBoneIndex;

				else if (sMarker == "LeftFinger0")
					m_BVHnodes[LeftFinger0] = nBoneIndex;
				else if (sMarker == "LeftFinger01")
					m_BVHnodes[LeftFinger01] = nBoneIndex;
				else if (sMarker == "LeftFinger02")
					m_BVHnodes[LeftFinger02] = nBoneIndex;

				else if (sMarker == "RightCollar")
					m_BVHnodes[RightCollar] = nBoneIndex;
				else if (sMarker == "RightShoulder")
					m_BVHnodes[RightShoulder] = nBoneIndex;
				else if (sMarker == "RightElbow")
					m_BVHnodes[RightElbow] = nBoneIndex;
				else if (sMarker == "RightWrist")
					m_BVHnodes[RightWrist] = nBoneIndex;

				else if (sMarker == "RightFinger0")
					m_BVHnodes[RightFinger0] = nBoneIndex;
				else if (sMarker == "RightFinger01")
					m_BVHnodes[RightFinger01] = nBoneIndex;
				else if (sMarker == "RightFinger02")
					m_BVHnodes[RightFinger02] = nBoneIndex;

				else if (sMarker == "Head")
					m_BVHnodes[Head] = nBoneIndex;
				else if (sMarker == "Neck")
					m_BVHnodes[Neck] = nBoneIndex;


			}
			else
			{
				OUTPUT_LOG("Bone index out of range when reading animation marker file. \r\n");
				return false;
			}
		}
	}
	return true;
}

bool CBVHSerializer::DumpParaXBones(int nLevel, int nBoneIndex, CParaFile* pFile)
{
	if (m_pXMesh == NULL)
		return false;
	for (int i = 0; i<nLevel; ++i)
	{
		if (pFile == 0) {
			OUTPUT_LOG("\t");
		}
		else {
			pFile->WriteFormated("\t");
		}
	}

	if (pFile == 0) {
		if (!m_bones[nBoneIndex].m_sMarkerName.empty())
		{
			OUTPUT_LOG("%s ", m_bones[nBoneIndex].m_sMarkerName.c_str());
		}
		OUTPUT_LOG("%d: ", nBoneIndex);
	}
	else {
		if (!m_bones[nBoneIndex].m_sMarkerName.empty())
		{
			pFile->WriteFormated("%s ", m_bones[nBoneIndex].m_sMarkerName.c_str());
		}
		pFile->WriteFormated("%d: ", nBoneIndex);
	}

	int nChildCount = m_bones[nBoneIndex].GetChildCount();
	Vector3 vAbsOffset = m_bones[nBoneIndex].m_vAbsOffset;

	if (pFile == 0) {
		OUTPUT_LOG("ChildCount %d SubBones %d SubLevel %d vAbsPos(%.1f %.1f %.1f) BoneLength:%4.2f\r\n", nChildCount, m_bones[nBoneIndex].m_nSubBones,
			m_bones[nBoneIndex].m_nSubLevels, vAbsOffset.x, vAbsOffset.y, vAbsOffset.z, m_bones[nBoneIndex].GetBoneLength());
	}
	else {
		pFile->WriteFormated("ChildCount %d SubBones %d SubLevel %d vAbsPos(%.1f %.1f %.1f) BoneLength:%4.2f\n", nChildCount, m_bones[nBoneIndex].m_nSubBones,
			m_bones[nBoneIndex].m_nSubLevels, vAbsOffset.x, vAbsOffset.y, vAbsOffset.z, m_bones[nBoneIndex].GetBoneLength());
	}
	for (int i = 0; i<nChildCount; ++i)
	{
		DumpParaXBones(nLevel + 1, m_bones[nBoneIndex].m_childBones[i], pFile);
	}
	return true;
}

int  CBVHSerializer::ComputeSubBonesCount(int nBoneIndex)
{
	if (m_pXMesh == NULL)
		return false;
	int nChildCount = m_bones[nBoneIndex].GetChildCount();
	int nTotalCount = 0;
	int nSubLevels = nChildCount>0 ? 1 : 0;

	for (int i = 0; i<nChildCount; ++i)
	{
		nTotalCount += ComputeSubBonesCount(m_bones[nBoneIndex].m_childBones[i]) + 1;
		int level = m_bones[m_bones[nBoneIndex].m_childBones[i]].m_nSubLevels + 1;
		if (level>nSubLevels)
			nSubLevels = level;
	}
	m_bones[nBoneIndex].m_nSubBones = nTotalCount;
	m_bones[nBoneIndex].m_nSubLevels = nSubLevels;
	return nTotalCount;
}


bool CBVHSerializer::BuildInternalBones()
{
	if (m_pXMesh == NULL)
		return false;

	int nBones = (int)m_pXMesh->GetObjectNum().nBones;
	m_nodes.resize(nBones);
	for (int i = 0; i<nBones; ++i)
	{
		m_nodes[i].m_nBoneIndex = -1;
	}

	for (int i = 0; i<MAX_NODE_NUM; ++i)
	{
		m_UniversalNodes[i].m_nBoneIndex = -1;
	}

	//////////////////////////////////////////////////////////////////////////
	/// build bidirectional bones from the input

	m_bones.resize(nBones);
	for (int i = 0; i<nBones; i++) {
		m_pXMesh->bones[i].calc = false;

		// build child bones
		m_bones[i].m_childBones.clear();
		m_bones[i].m_nParentIndex = m_pXMesh->bones[i].parent;
		m_bones[i].m_nSubBones = 0;
		m_bones[i].m_nSubLevels = 0;
		m_bones[i].m_vAbsOffset = *(Vector3*)&(m_pXMesh->bones[i].pivot);
		m_bones[i].m_vOffsetToParent = m_bones[i].m_vAbsOffset;
		if (m_bones[i].m_nParentIndex>0)
		{
			m_bones[i].m_vOffsetToParent -= *(Vector3*)&(m_pXMesh->bones[m_bones[i].m_nParentIndex].pivot);
		}
		float fLenSq = (m_bones[i].m_vOffsetToParent).squaredLength();
		m_bones[i].m_fBoneLength = (fLenSq>0.00001f) ? sqrt(fLenSq) : fLenSq;

		for (int k = 0; k<nBones; k++) {
			if (m_pXMesh->bones[k].parent == i)
			{
				m_bones[i].m_childBones.push_back(k);
			}
		}
	}

	// find root
	m_nRootBoneIndex = 0;

	int nCount = 0;
	for (int i = 0; i<nBones; i++) {
		if (m_bones[i].m_nParentIndex < 0)
		{
			// compute sub bones for all root bones
			ComputeSubBonesCount(i);

			if (nCount <= m_bones[i].m_nSubBones)
			{
				m_nRootBoneIndex = i;
				nCount = m_bones[i].m_nSubBones;
				m_UniversalNodes[UN_Root].m_nBoneIndex = i;
			}
		}
	}

	// sort child bones
	for (int i = 0; i<nBones; i++) {
		sort(m_bones[i].m_childBones.begin(), m_bones[i].m_childBones.end(), BoneChildCountCompare(m_bones));
	}

#ifdef _DEBUG
	OUTPUT_LOG("\r\n dumping bone hierarchy \r\n");
	DumpParaXBones(0, m_nRootBoneIndex);
#endif
	return true;
}

bool CBVHSerializer::GenHipMarker(int nBoneIndex, int* ChestBoneIndex)
{
	if (m_bones[nBoneIndex].GetChildCount() < 2 || m_bones[nBoneIndex].m_nSubLevels<3 || m_bones[nBoneIndex].m_nSubBones<6)
		return false;
	int i = 0;
	int nFirstHipBone = 0;
	if (m_bones[nBoneIndex].GetChildCount() >= 3 &&
		fabs(m_bones[m_bones[nBoneIndex].m_childBones[i]].m_vAbsOffset.z)<0.01f && m_bones[m_bones[nBoneIndex].m_childBones[i]].m_nSubLevels >= 4)
	{
		// if there are few bones in elsewhere , Hip may contain chest.
		if ((m_bones[m_nRootBoneIndex].m_nSubBones - m_bones[nBoneIndex].m_nSubBones)<6)
		{
			// this looks like a chest bone.
			*ChestBoneIndex = m_bones[nBoneIndex].m_childBones[i];
			nFirstHipBone++;
		}
	}
	if (m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone]].GetBoneLength()>0 &&
		fabs(m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone]].m_vAbsOffset.z)>0.01f &&
		m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone]].m_nSubLevels >= 2 && m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone + 1]].m_nSubLevels >= 2 &&
		(m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone]].m_nSubLevels - m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone + 1]].m_nSubLevels) <= 1 &&
		fabs(m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone]].GetBoneLength() - m_bones[m_bones[nBoneIndex].m_childBones[nFirstHipBone + 1]].GetBoneLength())<0.01f)
	{
		m_bones[nBoneIndex].m_sMarkerName = "Hips";
		// these look like a hip left/right bones
		for (int i = 0; i<2; i++)
		{
			int nBone = m_bones[nBoneIndex].m_childBones[nFirstHipBone + i];
			if (m_bones[nBone].m_vAbsOffset.z > 0.f)
				m_bones[nBone].m_sMarkerName = "LeftHip";
			else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
				m_bones[nBone].m_sMarkerName = "RightHip";
			else
				break;

			nBone = m_bones[nBone].m_childBones[0];
			if (m_bones[nBone].m_vAbsOffset.z > 0.f)
				m_bones[nBone].m_sMarkerName = "LeftKnee";
			else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
				m_bones[nBone].m_sMarkerName = "RightKnee";
			else
				break;

			if (m_bones[nBone].GetChildCount()>0)
			{
				nBone = m_bones[nBone].m_childBones[0];
				if (m_bones[nBone].m_vAbsOffset.z > 0.f)
					m_bones[nBone].m_sMarkerName = "LeftAnkle";
				else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
					m_bones[nBone].m_sMarkerName = "RightAnkle";
				else
					break;

				if (m_bones[nBone].GetChildCount()>0)
				{
					nBone = m_bones[nBone].m_childBones[0];
					if (m_bones[nBone].m_vAbsOffset.z > 0.f)
						m_bones[nBone].m_sMarkerName = "LeftToe";
					else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
						m_bones[nBone].m_sMarkerName = "RightToe";
					else
						break;
				}
			}
		}
	}
	else
		return false;
	return true;
}

bool CBVHSerializer::GenChestMarker(int nBoneIndex)
{
	if (m_bones[nBoneIndex].GetChildCount() <= 2 || m_bones[nBoneIndex].m_nSubLevels<3 || m_bones[nBoneIndex].m_nSubBones<6)
		return false;
	int i = 0;
	int nFirstCollarBone = -1;
	int nNeckBone = -1;

	if (m_bones[nBoneIndex].GetChildCount() >= 3 &&
		fabs(m_bones[m_bones[nBoneIndex].m_childBones[i]].m_vAbsOffset.z)<0.01f)
	{
		// this looks like a neck bone.
		nNeckBone = m_bones[nBoneIndex].m_childBones[i];
		i++;
	}
	if (m_bones[m_bones[nBoneIndex].m_childBones[i]].GetBoneLength()>0 &&
		m_bones[m_bones[nBoneIndex].m_childBones[i]].m_nSubLevels >= 3 && m_bones[m_bones[nBoneIndex].m_childBones[i + 1]].m_nSubLevels >= 3 &&
		(m_bones[m_bones[nBoneIndex].m_childBones[i]].m_nSubLevels - m_bones[m_bones[nBoneIndex].m_childBones[i + 1]].m_nSubLevels) <= 1 &&
		fabs(m_bones[m_bones[nBoneIndex].m_childBones[i]].GetBoneLength() - m_bones[m_bones[nBoneIndex].m_childBones[i + 1]].GetBoneLength())<0.015f)
	{
		// this looks like that we have collar bones. 	
		nFirstCollarBone = i;
		i += 2;
	}

	if (nNeckBone<0 && nFirstCollarBone >= 0)
	{

		if (m_bones[nBoneIndex].GetChildCount() >= 3 &&
			fabs(m_bones[m_bones[nBoneIndex].m_childBones[i]].m_vAbsOffset.z)<0.04f)
		{
			// this looks like a neck bone.
			nNeckBone = m_bones[nBoneIndex].m_childBones[i];
		}
	}

	if (nNeckBone >= 0 && nFirstCollarBone >= 0)
	{
		if (GetMarkedParentIndex(nBoneIndex)>0)
		{
			// if there is a hips bone above chest
			m_bones[nBoneIndex].m_sMarkerName = "Chest";
		}
		else
		{
			// if chest becomes the top bone.
			m_bones[nBoneIndex].m_sMarkerName = "Chest2";
			if (m_bones[nBoneIndex].m_nParentIndex>0)
			{
				int nIndex = m_bones[nBoneIndex].m_nParentIndex;
				if (m_bones[nIndex].GetBoneLength() > 0.1f)
				{
					m_bones[nIndex].m_sMarkerName = "Chest";
				}
				else
				{
					int nPPIndex = m_bones[nIndex].m_nParentIndex;
					if (nPPIndex >= 0 && m_bones[nPPIndex].GetBoneLength() > 0.1f)
						m_bones[nPPIndex].m_sMarkerName = "Chest";
					else
						m_bones[nIndex].m_sMarkerName = "Chest";
				}
			}

		}
		m_bones[nNeckBone].m_sMarkerName = "Neck";
		if (m_bones[nNeckBone].GetChildCount()>0)
			m_bones[m_bones[nNeckBone].m_childBones[0]].m_sMarkerName = "Head";

		for (int i = 0; i<2; i++)
		{
			int nBone = m_bones[nBoneIndex].m_childBones[nFirstCollarBone + i];
			if (m_bones[nBone].m_vAbsOffset.z > 0.f)
				m_bones[nBone].m_sMarkerName = "LeftCollar";
			else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
				m_bones[nBone].m_sMarkerName = "RightCollar";
			else
				break;

			nBone = m_bones[nBone].m_childBones[0];
			if (m_bones[nBone].m_vAbsOffset.z > 0.f)
				m_bones[nBone].m_sMarkerName = "LeftShoulder";
			else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
				m_bones[nBone].m_sMarkerName = "RightShoulder";
			else
				break;

			nBone = m_bones[nBone].m_childBones[0];
			if (m_bones[nBone].m_vAbsOffset.z > 0.f)
				m_bones[nBone].m_sMarkerName = "LeftElbow";
			else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
				m_bones[nBone].m_sMarkerName = "RightElbow";
			else
				break;

			if (m_bones[nBone].GetChildCount()>0)
			{
				nBone = m_bones[nBone].m_childBones[0];
				if (m_bones[nBone].m_vAbsOffset.z > 0.f)
					m_bones[nBone].m_sMarkerName = "LeftWrist";
				else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
					m_bones[nBone].m_sMarkerName = "RightWrist";
				else
					break;

				if (m_bones[nBone].GetChildCount()>0)
				{
					nBone = m_bones[nBone].m_childBones[0];
					if (m_bones[nBone].m_vAbsOffset.z > 0.f)
						m_bones[nBone].m_sMarkerName = "LeftFinger0";
					else if (m_bones[nBone].m_vAbsOffset.z < 0.f)
						m_bones[nBone].m_sMarkerName = "RightFinger0";
					else
						break;
				}
			}
		}
	}
	else
		return false;
	return true;
}

bool CBVHSerializer::AutoGenerateBVHMarker(int nBoneIndex, bool bGenChest, bool bGenHips)
{
	// use breadth first, traversing.
	queue<int> indexQueue;
	indexQueue.push(nBoneIndex);

	while ((bGenChest || bGenHips) && !indexQueue.empty())
	{
		nBoneIndex = indexQueue.front();
		indexQueue.pop();

		int nChestBone = -1;

		if (bGenHips && GenHipMarker(nBoneIndex, &nChestBone))
		{
			bGenHips = false;
			if (nChestBone >= 0)
			{
				if (bGenChest && GenChestMarker(nChestBone))
				{
					bGenChest = false;
				}
			}
		}
		else if (bGenChest && GenChestMarker(nBoneIndex))
		{
			bGenChest = false;
		}


		bool bFinished = !(bGenChest || bGenHips);

		// continue process children if not finished.
		if (!bFinished)
		{
			int nChildCount = m_bones[nBoneIndex].GetChildCount();
			for (int i = 0; i<nChildCount; ++i)
			{
				indexQueue.push(m_bones[nBoneIndex].m_childBones[i]);
			}
		}
	}
	bool bFinished = !(bGenChest || bGenHips);
	return bFinished;
}
bool CBVHSerializer::BuildBipedUniversalBones()
{
	if (m_pXMesh == NULL)
		return false;

	int nBones = GetBoneCount();

	int nIndex = 0, nCount = 0;
	bool bFound = false;

	//////////////////////////////////////////////////////////////////////////
	// find spine0
	int nSpine0 = UN_Root;
	while ((nCount = m_bones[GetBoneIndex(nSpine0)].GetChildCount()) < 3 && nCount >= 1 && nSpine0 <= UN_Spine0)
	{
		m_UniversalNodes[nSpine0 + 1].m_nBoneIndex = m_bones[GetBoneIndex(nSpine0)].m_childBones[0];
		++nSpine0;
	}

	if (nSpine0>UN_Spine0)
	{
		OUTPUT_LOG("error: too many root before a spine0 bone is found\r\n");
		return false;
	}
	if (UN_Spine0 != nSpine0)
		m_UniversalNodes[UN_Spine0] = m_UniversalNodes[nSpine0];

	//////////////////////////////////////////////////////////////////////////
	// find two legs and spineN
	nIndex = m_UniversalNodes[UN_Spine0].m_nBoneIndex;
	if ((nCount = m_bones[nIndex].GetChildCount())<3)
	{
		OUTPUT_LOG("error: the spine0 bone should have more than three child \r\n");
		return false;
	}

	if (m_bones[nIndex].m_childBones[0] > m_bones[nIndex].m_childBones[1])
	{
		m_UniversalNodes[UN_Spine1].m_nBoneIndex = m_bones[nIndex].m_childBones[0];
		bool bFound = false;
		for (int i = 1; i<nCount - 1; i++)
		{
			if (m_bones[nIndex].m_childBones[i] == m_bones[nIndex].m_childBones[i + 1])
			{
				// which is left and which is right is arbitrary.
				m_UniversalNodes[UN_Left_Thigh].m_nBoneIndex = m_bones[nIndex].m_childBones[i];
				m_UniversalNodes[UN_Right_Thigh].m_nBoneIndex = m_bones[nIndex].m_childBones[i];
				bFound = true;
			}
		}
		if (!bFound)
		{
			OUTPUT_LOG("error: the Left_Thigh and Right_Thigh are not found \r\n");
			return false;
		}
	}
	else
	{
		OUTPUT_LOG("error: the spine1 not found \r\n");
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// find neck
	nIndex = m_UniversalNodes[UN_Spine1].m_nBoneIndex;
	if ((nCount = m_bones[nIndex].GetChildCount())<7)
	{
		OUTPUT_LOG("error: the neck bone not found \r\n");
		return false;
	}
	int nNeck = UN_Spine1;
	bFound = false;
	while (bFound && nNeck <= UN_Neck)
	{
		nIndex = GetBoneIndex(nNeck);
		nCount = m_bones[nIndex].GetChildCount();
		bFound = false;
		for (int i = 0; i<nCount; ++i)
		{
			if (m_bones[nIndex].m_childBones[i] == m_bones[nIndex].m_childBones[i + 1])
			{
				bFound = true;
			}
		}
		if (!bFound)
		{
			m_UniversalNodes[nNeck + 1].m_nBoneIndex = m_bones[nIndex].m_childBones[0];
			++nNeck;
		}
	}
	if (nNeck>UN_Neck)
	{
		OUTPUT_LOG("error: the neck bone not found \r\n");
		return false;
	}

	m_UniversalNodes[UN_Neck] = m_UniversalNodes[nNeck];
	nCount = m_bones[GetBoneIndex(UN_Neck)].GetChildCount();
	if (nCount < 5)
	{
		OUTPUT_LOG("error: number of child nodes of neck bone are too small.\r\n");
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	// TODO: process left/right thigh
	return true;
}

bool CBVHSerializer::DumpUniversalBones()
{
	if (m_pXMesh == NULL)
		return false;

	for (int i = 0; i<MAX_NODE_NUM; ++i)
	{
		int nIndex = m_UniversalNodes[i].m_nBoneIndex;
		OUTPUT_LOG("%s: ", m_UniversalNodes[i].m_nodeName.c_str());
		OUTPUT_LOG("index %d,", nIndex);
		if (nIndex>0)
		{
			OUTPUT_LOG("ChildCount %d,", m_bones[nIndex].GetChildCount());
		}
		OUTPUT_LOG("\r\n");
	}
	return true;
}

bool CBVHSerializer::InitHierachy(CParaXModel& xmesh)
{
	m_pXMesh = &xmesh;
	bool bBiped = true;

	// must be called first. 
	if (!BuildInternalBones())
		return false;
	//
	//	if(!BuildBipedUniversalBones())
	//	{
	//		return false;
	//	}
	//	else
	//	{
	//#ifdef _DEBUG
	//		DumpUniversalBones();
	//#endif
	//	}

	// TODO: export from universal bones to CSM bone hierarchy. 

	return bBiped;
}

int CBVHSerializer::GetMarkedParentIndex(int nBoneIndex)
{
	// find offset to a known parent.
	int nIndex = nBoneIndex;
	while (m_bones[nIndex].m_nParentIndex != -1)
	{
		nIndex = m_bones[nIndex].m_nParentIndex;
		if (!m_bones[nIndex].m_sMarkerName.empty())
		{
			return nIndex;
		}
	}
	return -1; // not found
}

bool CBVHSerializer::UpdateCriticalBone(int nBoneIndex)
{
	int nChildCount = m_bones[nBoneIndex].GetChildCount();
	bool bCritical = false;

	for (int i = 0; i<nChildCount; ++i)
	{
		if (UpdateCriticalBone(m_bones[nBoneIndex].m_childBones[i]))
		{
			bCritical = true;
		}
	}
	m_bones[nBoneIndex].m_bCriticalBone = bCritical || !(m_bones[nBoneIndex].m_sMarkerName.empty());
	return m_bones[nBoneIndex].m_bCriticalBone;
}

bool CBVHSerializer::SortBVHMarker()
{
	int nBones = (int)m_bones.size();
	for (int i = 0; i<nBones; i++) {
		sort(m_bones[i].m_childBones.begin(), m_bones[i].m_childBones.end(), BoneMarkerCompare(m_bones));
	}
	return true;
}

bool CBVHSerializer::WriteBVHNode(int nLevel, int nBoneIndex, CParaFile* pFile, bool bEscapeUnknownBones)
{
	int nChildCount = m_bones[nBoneIndex].GetChildCount();
	if (bEscapeUnknownBones && m_bones[nBoneIndex].m_sMarkerName == "End")
		nChildCount = 0;
	bool bIsUnknownBone = m_bones[nBoneIndex].IsUnKnownBone() && (nChildCount != 0);

	if (!bEscapeUnknownBones || !bIsUnknownBone)
	{
		// write root or joint name
		for (int i = 0; i<nLevel; ++i)
			pFile->WriteFormated("\t");

		if (nChildCount == 0)
		{
			// end site
			pFile->WriteFormated("End Site\n");
		}
		else if (nLevel == 0)
		{
			// root 
			if (!m_bones[nBoneIndex].m_sMarkerName.empty())
				pFile->WriteFormated("HIERARCHY\nROOT %s\n", m_bones[nBoneIndex].m_sMarkerName.c_str());
			else
			{
				char sID[MAX_PATH];
				sprintf(sID, "%d", nBoneIndex);
				pFile->WriteFormated("HIERARCHY\nROOT %s\n", sID);
			}
		}
		else
		{
			// joint
			if (!m_bones[nBoneIndex].m_sMarkerName.empty())
				pFile->WriteFormated("JOINT %s\n", m_bones[nBoneIndex].m_sMarkerName.c_str());
			else
			{
				char sID[MAX_PATH];
				sprintf(sID, "%d", nBoneIndex);
				pFile->WriteFormated("JOINT %s\n", sID);
			}
		}
		// left curly brace
		for (int i = 0; i<nLevel; ++i)
			pFile->WriteFormated("\t");
		pFile->WriteFormated("{\n");


		// write offset relative to parent
		for (int i = 0; i<nLevel + 1; ++i)
			pFile->WriteFormated("\t");

		Vector3 vOffset = m_bones[nBoneIndex].m_vOffsetToParent;
		if (bEscapeUnknownBones)
		{
			int nIndex = GetMarkedParentIndex(nBoneIndex);
			if (nIndex >= 0)
				vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;
		}
		pFile->WriteFormated("OFFSET \t\t%f %f %f\n", vOffset.z, vOffset.y, vOffset.x);
		//pFile->WriteFormated("OFFSET \t\t%f %f %f\n", vOffset.x, vOffset.y, vOffset.z);

		// write channels 
		if (nChildCount != 0)
		{
			for (int i = 0; i<nLevel + 1; ++i)
				pFile->WriteFormated("\t");
			if (nLevel == 0 || m_bExportBVHPosition)
				pFile->WriteFormated("CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n");
			else
				pFile->WriteFormated("CHANNELS 3 Zrotation Xrotation Yrotation\n");
		}
	}


	for (int i = 0; i<nChildCount; ++i)
	{
		// this will prevent multiple end site be exported.
		int nCount = m_bones[m_bones[nBoneIndex].m_childBones[i]].GetChildCount();
		if (nCount > 0 || i == 0)
		{
			if (!bEscapeUnknownBones ||
				(m_bones[m_bones[nBoneIndex].m_childBones[i]].m_bCriticalBone))
				WriteBVHNode(nLevel + 1, m_bones[nBoneIndex].m_childBones[i], pFile, bEscapeUnknownBones);
		}
	}

	if (!bEscapeUnknownBones || !bIsUnknownBone)
	{
		// right curly brace
		for (int i = 0; i<nLevel; ++i)
			pFile->WriteFormated("\t");
		pFile->WriteFormated("}\n");
	}

	return true;
}

bool CBVHSerializer::WriteBVHNodeAnimation(int nLevel, int nBoneIndex, CParaFile* pFile, bool bEscapeUnknownBones)
{
	int nChildCount = m_bones[nBoneIndex].GetChildCount();
	if (bEscapeUnknownBones && m_bones[nBoneIndex].m_sMarkerName == "End")
		nChildCount = 0;
	bool bIsUnknownBone = m_bones[nBoneIndex].IsUnKnownBone();

	// write animation on each channel. 
	if (nChildCount != 0)
	{
		if (!bEscapeUnknownBones || !bIsUnknownBone)
		{
			Vector3 vPos1, vPos2, vPos(0, 0, 0);
			Vector3 vRot(0, 0, 0);
			//// define this to compensate for unknown bones using the immediate known bones. Right now, it does not work very well. 
			////#define AUTO_COMPENSATE_UNKNOWN_BONES
			//#ifdef AUTO_COMPENSATE_UNKNOWN_BONES
			//			int nMarkedParent = -1;
			//			if(!bEscapeUnknownBones 
			//				|| m_bones[nBoneIndex].m_nParentIndex == (nMarkedParent=GetMarkedParentIndex(nBoneIndex)) )
			//			{
			//#endif

			// if the parent bone is its immediate upper level bone, then we only need to read data from the key
			if (m_pXMesh->bones[nBoneIndex].rot.used) {
				Quaternion q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
				//swap(q.x, q.z);
				//q.x = -q.x;

				q.ToRadians(&vRot.x, &vRot.y, &vRot.z);
				vRot.x *= 180.f / MATH_PI;
				vRot.y *= 180.f / MATH_PI;
				vRot.z *= 180.f / MATH_PI;
			}
			/*#ifdef AUTO_COMPENSATE_UNKNOWN_BONES
			}
			else
			{
			if(nMarkedParent>=0)
			{
			Vector3 vAbsPos = (m_pXMesh->bones[nBoneIndex].mat*m_pXMesh->bones[nBoneIndex].pivot);
			Vector3 vVector1 = vAbsPos - m_bones[nMarkedParent].m_vAbsOffset;
			Vector3 vVector0 = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nMarkedParent].m_vAbsOffset;
			ParaVec3Normalize(&vVector0, &vVector0);
			ParaVec3Normalize(&vVector1, &vVector1);
			if(vVector0!=vVector1)
			{
			D3DXVec3Cross(&vRot, &vVector0, &vVector1);
			float fCosTheta = D3DXVec3Dot(&vVector0, &vVector1);
			fCosTheta = acosf(fCosTheta);
			Quaternion q(*(Vector3*)&vRot, fCosTheta);

			q.ToRadians(&vRot.x, &vRot.y, &vRot.z);
			vRot.x *= 180.f/MATH_PI;
			vRot.y *= 180.f/MATH_PI;
			vRot.z *= 180.f/MATH_PI;
			}
			}
			}
			#endif		*/

			if (nLevel == 0)
			{
				// since we have different default orientations, rotate the root bone to face positive X.
				if (m_bRotateY)
					vRot.y += 90.f;
			}
			if (nLevel == 0 || m_bExportBVHPosition)
			{
				Vector3 vAbsPos = (m_pXMesh->bones[nBoneIndex].mat*m_pXMesh->bones[nBoneIndex].pivot);
				vPos = vAbsPos - m_bones[nBoneIndex].m_vAbsOffset;
				// if this is the root node.
				pFile->WriteFormated("%f %f %f ", vPos.x, vPos.y, -vPos.z); // the order is affected by m_RotateY settings.
			}
			pFile->WriteFormated("%f %f %f ", vRot.z, vRot.x, vRot.y);
		}
	}

	// write child nodes.
	for (int i = 0; i<nChildCount; ++i)
	{
		// this will prevent multiple end site be exported.
		int nCount = m_bones[m_bones[nBoneIndex].m_childBones[i]].GetChildCount();
		if (nCount > 0 || i == 0)
		{
			if (!bEscapeUnknownBones || m_bones[m_bones[nBoneIndex].m_childBones[i]].m_bCriticalBone)
				WriteBVHNodeAnimation(nLevel + 1, m_bones[nBoneIndex].m_childBones[i], pFile, bEscapeUnknownBones);
		}
	}

	if (!bEscapeUnknownBones || !bIsUnknownBone)
	{
		// end this frame
		if (nLevel == 0)
		{
			pFile->WriteFormated("\n");
		}
	}
	return true;
}

bool CBVHSerializer::WriteQuaternion(CParaFile* pFile, const Quaternion& q, bool bRotY)
{
	Vector3 vRot(0, 0, 0);
	q.ToRadians(&vRot.x, &vRot.y, &vRot.z);
	vRot.x *= 180.f / MATH_PI;
	vRot.y *= 180.f / MATH_PI;
	vRot.z *= 180.f / MATH_PI;

	if (bRotY)
	{
		vRot.y += 90.f;
		pFile->WriteFormated("%f %f %f ", vRot.z, vRot.x, vRot.y);
	}
	else
	{
		pFile->WriteFormated("%f %f %f ", vRot.z, vRot.x, vRot.y);
	}
	return true;
}

bool CBVHSerializer::WriteEndSite(CParaFile* pFile, int nLevel, int nBoneIndex)
{
	Vector3 vOffset(0, 0.1f, 0);

	// TODO: write the end site for nBoneIndex;
	// get any node with "End" mark
	BeginBVHNode(pFile, nLevel, "End", vOffset, false, false);
	EndBVHNode(pFile, nLevel, "End");
	return true;
}

bool CBVHSerializer::BeginBVHNode(CParaFile* pFile, int nLevel, const string& sMarker, const Vector3& vOffset, bool bExportPos, bool bExportRot)
{
	// write root or joint name
	for (int i = 0; i<nLevel; ++i)
		pFile->WriteFormated("\t");

	if (sMarker == "End")
	{
		// end site
		pFile->WriteFormated("End Site\n");
	}
	else if (sMarker == "Hips")
	{
		pFile->WriteFormated("HIERARCHY\nROOT %s\n", sMarker.c_str());
	}
	else
	{
		pFile->WriteFormated("JOINT %s\n", sMarker.c_str());
	}

	// left curly brace
	for (int i = 0; i<nLevel; ++i)
		pFile->WriteFormated("\t");
	pFile->WriteFormated("{\n");


	// write offset relative to parent
	for (int i = 0; i<nLevel + 1; ++i)
		pFile->WriteFormated("\t");

	pFile->WriteFormated("OFFSET \t\t%f %f %f\n", vOffset.z, vOffset.y, vOffset.x);

	if (sMarker != "End")
	{
		for (int i = 0; i<nLevel + 1; ++i)
			pFile->WriteFormated("\t");

		if (bExportPos && bExportRot)
			pFile->WriteFormated("CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation\n");
		else if (bExportRot)
			pFile->WriteFormated("CHANNELS 3 Zrotation Xrotation Yrotation\n");
	}
	return true;
}

bool CBVHSerializer::EndBVHNode(CParaFile* pFile, int nLevel, const string& sMarker)
{
	// right curly brace
	for (int i = 0; i<nLevel; ++i)
		pFile->WriteFormated("\t");
	pFile->WriteFormated("}\n");
	return true;
}

bool CBVHSerializer::WriteHips(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[Hips];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "Hips", vOffset, true, true);
		WriteChest(nLevel + 1, bHierachy, pFile);
		WriteLeftHip(nLevel + 1, bHierachy, pFile);
		WriteRightHip(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "Hips");
	}
	else
	{
		Quaternion q;

		Vector3 vAbsPos = (m_pXMesh->bones[nBoneIndex].mat*m_pXMesh->bones[nBoneIndex].pivot);
		Vector3 vPos = vAbsPos - m_bones[nBoneIndex].m_vAbsOffset;
		q = Quaternion(Vector3(0, 1, 0), -MATH_PI / 2);
		Matrix4 mRot = Matrix4(q);

		int nIndex = m_BVHnodes[Hips];
		if (nIndex >= 0)
		{
			if (m_pXMesh->bones[nIndex].rot.used) {
				//q = m_pXMesh->bones[nIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0,0,0.f);
				Matrix4 m = mRot * m_pXMesh->bones[nIndex].mrot;
				q.FromRotationMatrix(m);
			}
		}
		vPos = *(Vector3*)&(mRot * (*(Vector3*)&vPos));

		// if this is the root node.
		//pFile->WriteFormated("%f %f %f ", vPos.x, vPos.y, -vPos.z); // the order is affected by m_RotateY settings.
		pFile->WriteFormated("%f %f %f ", vPos.z, vPos.y, vPos.x); // the order is affected by m_RotateY settings.

		WriteQuaternion(pFile, q, false); // TODO: should be true

		WriteChest(nLevel + 1, bHierachy, pFile);
		WriteLeftHip(nLevel + 1, bHierachy, pFile);
		WriteRightHip(nLevel + 1, bHierachy, pFile);


		pFile->WriteFormated("\n");
	}
	return true;
}

bool CBVHSerializer::WriteChest(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[Chest];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = m_BVHnodes[Hips];
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "Chest", vOffset, false, true);
		WriteChest2(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "Chest");
	}
	else
	{
		Quaternion q;
		if (!m_bSwapChestHip)
		{
			if (m_pXMesh->bones[nBoneIndex].rot.used) {
				q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
			}
		}
		else
		{
			// swapped
			int nIndex = m_BVHnodes[Hips];
			if (nIndex >= 0)
			{
				if (m_pXMesh->bones[nIndex].rot.used) {
					q = m_pXMesh->bones[nIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
				}
			}
			q.x = -q.x;
			q.y = -q.y;
			q.z = -q.z;
		}
		WriteQuaternion(pFile, q, false);

		WriteChest2(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteChest2(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[Chest2];
	if (nBoneIndex == -1)
	{
		// TODO:return  WriteChest3(nLevel, bHierachy, pFile);
		WriteNeck(nLevel, bHierachy, pFile);
		WriteLeftCollar(nLevel, bHierachy, pFile);
		WriteRightCollar(nLevel, bHierachy, pFile);
		return true;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = m_BVHnodes[Chest];
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "Chest2", vOffset, false, true);
		WriteNeck(nLevel + 1, bHierachy, pFile);
		WriteLeftCollar(nLevel + 1, bHierachy, pFile);
		WriteRightCollar(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "Chest2");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteNeck(nLevel + 1, bHierachy, pFile);
		WriteLeftCollar(nLevel + 1, bHierachy, pFile);
		WriteRightCollar(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteLeftHip(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftHip];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = m_BVHnodes[Hips];
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftHip", vOffset, false, true);
		WriteLeftKnee(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftHip");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftKnee(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteRightHip(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightHip];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = m_BVHnodes[Hips];
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightHip", vOffset, false, true);
		WriteRightKnee(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightHip");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightKnee(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteNeck(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[Neck];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "Neck", vOffset, false, true);
		WriteHead(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "Neck");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteHead(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteLeftCollar(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftCollar];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftCollar", vOffset, false, true);
		WriteLeftShoulder(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftCollar");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftShoulder(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteRightCollar(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightCollar];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightCollar", vOffset, false, true);
		WriteRightShoulder(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightCollar");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightShoulder(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteHead(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[Head];
	if (nBoneIndex == -1)
	{
		if (bHierachy)
		{
			// create default head in the up direction 0.2 meters 
			BeginBVHNode(pFile, nLevel, "Head", Vector3(0, 0.2f, 0), false, true);
			WriteEndSite(pFile, nLevel + 1, nBoneIndex);
			EndBVHNode(pFile, nLevel, "Head");
		}
		else
		{
			Quaternion q;
			WriteQuaternion(pFile, q, false);
		}
		return false;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "Head", vOffset, false, true);
		WriteEndSite(pFile, nLevel + 1, nBoneIndex);
		EndBVHNode(pFile, nLevel, "Head");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);
	}
	return true;
}
bool CBVHSerializer::WriteLeftKnee(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftKnee];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftKnee", vOffset, false, true);
		WriteLeftAnkle(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftKnee");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftAnkle(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteLeftAnkle(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftAnkle];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftAnkle", vOffset, false, true);
		WriteLeftToe(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftAnkle");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftToe(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteLeftToe(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftToe];
	if (nBoneIndex == -1)
	{
		if (bHierachy)
			WriteEndSite(pFile, nLevel, nBoneIndex);
		return true;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftToe", vOffset, false, true);
		WriteEndSite(pFile, nLevel + 1, nBoneIndex);
		EndBVHNode(pFile, nLevel, "LeftToe");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		//WriteXXX(nLevel+1, bHierachy, pFile);
	}
	return true;
}


bool CBVHSerializer::WriteRightKnee(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightKnee];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightKnee", vOffset, false, true);
		WriteRightAnkle(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightKnee");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightAnkle(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteRightAnkle(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightAnkle];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightAnkle", vOffset, false, true);
		WriteRightToe(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightAnkle");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightToe(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteRightToe(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightToe];
	if (nBoneIndex == -1)
	{
		if (bHierachy)
			WriteEndSite(pFile, nLevel, nBoneIndex);
		return true;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightToe", vOffset, false, true);
		WriteEndSite(pFile, nLevel + 1, nBoneIndex);
		EndBVHNode(pFile, nLevel, "RightToe");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		//WriteXXX(nLevel+1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteLeftShoulder(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftShoulder];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftShoulder", vOffset, false, true);
		WriteLeftElbow(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftShoulder");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftElbow(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteLeftElbow(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftElbow];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftElbow", vOffset, false, true);
		WriteLeftWrist(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftElbow");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftWrist(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteLeftWrist(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftWrist];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftWrist", vOffset, false, true);
		WriteLeftFinger0(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "LeftWrist");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteLeftFinger0(nLevel + 1, bHierachy, pFile);
	}
	return true;
}


bool CBVHSerializer::WriteLeftFinger0(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[LeftFinger0];
	if (nBoneIndex == -1)
	{
		if (bHierachy)
			WriteEndSite(pFile, nLevel, nBoneIndex);
		return true;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "LeftFinger0", vOffset, false, true);
		WriteEndSite(pFile, nLevel + 1, nBoneIndex);
		EndBVHNode(pFile, nLevel, "LeftFinger0");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		//WriteXXX(nLevel+1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteRightShoulder(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightShoulder];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightShoulder", vOffset, false, true);
		WriteRightElbow(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightShoulder");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightElbow(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteRightElbow(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightElbow];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightElbow", vOffset, false, true);
		WriteRightWrist(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightElbow");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightWrist(nLevel + 1, bHierachy, pFile);
	}
	return true;
}
bool CBVHSerializer::WriteRightWrist(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightWrist];
	if (nBoneIndex == -1)
		return false;
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightWrist", vOffset, false, true);
		WriteRightFinger0(nLevel + 1, bHierachy, pFile);
		EndBVHNode(pFile, nLevel, "RightWrist");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		WriteRightFinger0(nLevel + 1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::WriteRightFinger0(int nLevel, bool bHierachy, CParaFile* pFile)
{
	int nBoneIndex = m_BVHnodes[RightFinger0];
	if (nBoneIndex == -1)
	{
		if (bHierachy)
			WriteEndSite(pFile, nLevel, nBoneIndex);
		return true;
	}
	if (bHierachy)
	{
		Vector3 vOffset = m_bones[nBoneIndex].m_vAbsOffset;
		int nIndex = GetMarkedParentIndex(nBoneIndex);
		if (nIndex >= 0)
			vOffset = m_bones[nBoneIndex].m_vAbsOffset - m_bones[nIndex].m_vAbsOffset;

		BeginBVHNode(pFile, nLevel, "RightFinger0", vOffset, false, true);
		WriteEndSite(pFile, nLevel + 1, nBoneIndex);
		EndBVHNode(pFile, nLevel, "RightFinger0");
	}
	else
	{
		Quaternion q;
		if (m_pXMesh->bones[nBoneIndex].rot.used) {
			q = m_pXMesh->bones[nBoneIndex].rot.getValue(m_pXMesh->m_CurrentAnim.nIndex, m_pXMesh->m_CurrentAnim.nCurrentFrame, 0, 0, 0.f);
		}
		WriteQuaternion(pFile, q, false);

		//WriteXXX(nLevel+1, bHierachy, pFile);
	}
	return true;
}

bool CBVHSerializer::SaveBVH(const string& sFileName, const string& sMarkerNameFile)
{
	m_bSwapChestHip = false;
	bool bUseMarkerFile = !sMarkerNameFile.empty();
	if (!bUseMarkerFile)
	{
		string sMarkerPath = sFileName + ".marker";
		if (CParaFile::DoesFileExist(sMarkerPath.c_str()))
		{
			LoadMarkerFile(sMarkerPath);
		}
		else
		{
			// generate marker
			AutoGenerateBVHMarker(m_nRootBoneIndex, true, true);

			// dump automatically generated marker file
			SaveMarkerFile(sMarkerPath);
			return true;
		}
	}
	else
	{
		// read marker into m_nodes.
		LoadMarkerFile(sMarkerNameFile);
	}

	// find which bones are critical. 
	UpdateCriticalBone(m_nRootBoneIndex);

	// sort by marker name
	//SortBVHMarker();

	m_bEscapeUnknownBones = m_BVHnodes[Hips] >= 0;
	//m_bEscapeUnmarkedBones = true;

	// for each animation.
	int nAnimCount = m_pXMesh->GetObjectNum().nAnimations;
	for (int nIndexAnim = 0; nIndexAnim<nAnimCount; ++nIndexAnim)
	{
		int nIndexAnimID = m_pXMesh->GetAnimIDByIndex(nIndexAnim);

		m_pXMesh->m_CurrentAnim.nIndex = nIndexAnim;
		ModelAnimation &animInfo = m_pXMesh->anims[nIndexAnim];

		string sPerAnimationBVHName = sFileName.substr(0, sFileName.size() - 4);
		{
			char sParam[255];
			memset(sParam, 0, sizeof(sParam));
			if (animInfo.loopType == 1)
			{
				// does not loop
				snprintf(sParam, 255, "(%d, %f, nonloop)", nIndexAnimID, animInfo.moveSpeed);
			}
			else
			{
				if (animInfo.moveSpeed != 0.f)
					snprintf(sParam, 255, "(%d, %f)", nIndexAnimID, animInfo.moveSpeed);
				else
					snprintf(sParam, 255, "(%d)", nIndexAnimID);

			}
			sPerAnimationBVHName += sParam;
			sPerAnimationBVHName += ".bvh";
		}

		CParaFile file;
		if (file.CreateNewFile(sPerAnimationBVHName.c_str()))
		{

			m_pXMesh->m_CurrentAnim.nCurrentFrame = animInfo.timeStart;
			m_pXMesh->m_NextAnim.MakeInvalid();
			m_pXMesh->m_BlendingAnim.MakeInvalid();
			m_pXMesh->blendingFactor = 0;

			bool bResult = false;
			if (!m_bEscapeUnknownBones)
				bResult = WriteBVHNode(0, m_nRootBoneIndex, &file, m_bEscapeUnmarkedBones);
			else
				bResult = WriteHips(0, true, &file);

			if (bResult)
			{
				file.WriteString("MOTION\n");
				int nTotalFrames = (int)((animInfo.timeEnd - animInfo.timeStart)*m_nFPS / 1000.f);
				float fFrameDuration = 1.0f / m_nFPS;
				file.WriteFormated("Frames: %d\n", nTotalFrames);
				file.WriteFormated("Frame Time: %f\n", fFrameDuration);

				for (int i = 0; i< nTotalFrames; ++i)
				{
					if (i == nTotalFrames - 1)
						m_pXMesh->m_CurrentAnim.nCurrentFrame = animInfo.timeEnd;
					else
						m_pXMesh->m_CurrentAnim.nCurrentFrame = (int)(animInfo.timeStart + i * fFrameDuration*1000.f);

					m_pXMesh->animate(CGlobals::GetSceneState(), NULL);

					if (!m_bEscapeUnknownBones)
						WriteBVHNodeAnimation(0, m_nRootBoneIndex, &file, m_bEscapeUnmarkedBones);
					else
						WriteHips(0, false, &file);
				}
			}
		}
	}

	return false;
}