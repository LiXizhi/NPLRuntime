//-----------------------------------------------------------------------------
// Class:Block max model parser	
// Authors:	leio, LiXizhi, wujiajun(winless)
// Emails:	lixizhi@yeah.net
// Date:	2015.6.29
// Desc: at most 65535*65535*65535 blocks can be loaded
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXSerializer.h"
#include "BlockEngine/BlockModel.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockTemplate.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "StringHelper.h"
#include "NPLHelper.h"
#include "NPLTable.h"
#include "ParaXModel/AnimTable.h"
#include "ParaXModel/XFileHelper.h"
#include "BMaxBlockModelNode.h"
#include <tinyxml.h>
#include "BlockEngine/BlockDirection.h"
#include "BMaxAnimGenerator.h"
#include "BMaxParser.h"

#include "ZipArchive.h"

/** @def define this to use algorithm that is not recursive for merging coplanar faces. */
#define USE_COPLANER_ALGORITHM_NON_RECURSIVE

namespace ParaEngine
{
	const int BMaxParser::MaxBoneLengthHorizontal = 50;
	const int BMaxParser::MaxBoneLengthVertical = 100;

	BMaxParser::BMaxParser(const char* filename, BMaxParser* pParent)
		: m_bAutoScale(true)
		, m_nHelperBlockId(90)
		, m_pAnimGenerator(NULL)
		, m_pParent(pParent)
		, m_bHasAnimation(false)
		, m_centerPos(0, 0, 0)
		, m_fScale(1.f)
		, m_nLodLevel(0)
		, m_bMergeCoplanerBlockFace(true)
	{
		m_bHasBoneBlock = false;
		if (filename)
			SetFilename(filename);
		//Load(pBuffer, nSize);
	}

	BMaxParser::~BMaxParser(void)
	{
		for (uint32 i = 0; i < m_blockModels.size(); i++)
		{
			SAFE_DELETE(m_blockModels[i]);
		}
		m_blockModels.clear();
		m_nodes.clear();
		SAFE_DELETE(m_pAnimGenerator);
	}

	void BMaxParser::Load(const char* pBuffer, int32 nSize)
	{
		m_nLodLevel = 0;

		BMaxXMLDocument doc;

		std::string uncompressedData;

		if (IsZipData(pBuffer, nSize))
		{
			if (GetFirstFileData(pBuffer, nSize, uncompressedData))
			{
				pBuffer = uncompressedData.c_str();
			}
		}


		doc.Parse(pBuffer);
		ParseHead(doc);
		ParseBlocks(doc);
	}

	void BMaxParser::ParseBlocks(BMaxXMLDocument& doc)
	{
		BMaxXMLElement* element = doc.FirstChildElement("pe:blocktemplate");
		if (element != NULL)
		{
			BMaxXMLElement* blocks_element = element->FirstChildElement("pe:blocks");
			if (blocks_element != NULL)
			{

				const char* value = blocks_element->GetText();
				ParseBlocks_Internal(value);
				ParseBlockFrames();
				CalculateBoneWeights();
				if (m_bMergeCoplanerBlockFace) {
					MergeCoplanerBlockFace();
				}
				else {
					CreatRectanglesFromBlocks();
				}
			}
		}
	}

	void BMaxParser::ParseBlocks_Internal(const char* value)
	{
		auto pBlockWorld = BlockWorldClient::GetInstance();
		vector<BMaxNodePtr> nodes;
		CShapeBox aabb;
		NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(value);
		for (NPL::NPLTable::IndexIterator_Type itCur = msg.index_begin(); itCur != msg.index_end(); ++itCur)
		{
			NPL::NPLObjectProxy& block = itCur->second;
			int x = (int)((double)block[1]);
			int y = (int)((double)block[2]);
			int z = (int)((double)block[3]);
			int template_id = (int)((double)block[4]);
			int block_data = (int)((double)block[5]);

			aabb.Extend(Vector3((float)x, (float)y, (float)z));
			auto pBlockTemplate = pBlockWorld->GetBlockTemplate(template_id);

			if (pBlockTemplate && m_nHelperBlockId != template_id)
			{
				if (template_id == BlockModelBlockId)
				{
					if (block[6].GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
					{
						NPL::NPLObjectProxy& entityData = block[6];
						NPL::NPLObjectProxy& attr = entityData["attr"];
						if (attr.GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
						{
							std::string sFilename = attr["filename"];
							if (!sFilename.empty())
							{
								if (IsFileNameRecursiveLoaded(sFilename))
								{
									OUTPUT_LOG("warning: bmax models reference the file %s recursively\n", sFilename.c_str());
								}
								else
								{
									BMaxBlockModelNodePtr node(new BMaxBlockModelNode(this, x, y, z, template_id, block_data));
									node->SetFilename(sFilename);
									Matrix4 matLocalTrans;
									matLocalTrans.identity();
									bool bHasTransform = false;

									float fScaling = (float)((double)attr["scale"]);
									if (fScaling != 0.f && fScaling != 1.f)
									{
										bHasTransform = true;
										matLocalTrans.makeScale(fScaling, fScaling, fScaling);
									}

									float fFacing = (float)((double)attr["facing"]);
									if (fFacing != 0.f)
									{
										bHasTransform = true;
										if (fScaling != 0.f && fScaling != 1.f)
										{
											Matrix4 rot;
											rot.makeRot(Quaternion(Vector3::UNIT_Y, fFacing), Vector3::ZERO);
											matLocalTrans = matLocalTrans * rot;
										}
										else
										{
											matLocalTrans.makeRot(Quaternion(Vector3::UNIT_Y, fFacing), Vector3::ZERO);
										}
									}

									float fOffsetX = (float)((double)attr["offsetX"]);
									float fOffsetY = (float)((double)attr["offsetY"]);
									float fOffsetZ = (float)((double)attr["offsetZ"]);
									if (fOffsetX != 0.f || fOffsetY != 0.f || fOffsetZ != 0.f) 
									{
										const float fScaleCorrection = 512.f / 533.3333f;
										fOffsetX *= fScaleCorrection;
										fOffsetY *= fScaleCorrection;
										fOffsetZ *= fScaleCorrection;
										bHasTransform = true;
										matLocalTrans.offsetTrans(Vector3(fOffsetX, fOffsetY, fOffsetZ));
									}

									if(bHasTransform)
										node->SetTransform(matLocalTrans);
									
									nodes.push_back(BMaxNodePtr(node.get()));
								}
							}
						}
					}
				}
				else if (template_id == BoneBlockId)
				{

					m_bHasBoneBlock = true;
					int nBoneIndex = (int)m_bones.size();
					BMaxFrameNodePtr pFrameNode(new BMaxFrameNode(this, x, y, z, template_id, block_data, nBoneIndex));
					m_bones.push_back(pFrameNode);
					nodes.push_back(BMaxNodePtr(pFrameNode.get()));

					if (block[6].GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
					{
						NPL::NPLObjectProxy& entityData = block[6];

						std::string sBoneName = "";
						GetAnimGenerator()->ParseParameters(entityData, nBoneIndex, sBoneName);

						if (entityData[1].GetType() == NPL::NPLObjectBase::NPLObjectType_Table)
						{
							NPL::NPLObjectProxy& cmd = entityData[1];
							if (cmd[1].GetType() == NPL::NPLObjectBase::NPLObjectType_String)
							{
								if (!sBoneName.empty())
								{
									int nCount = GetNameAppearanceCount(sBoneName, true);
									if (nCount > 0) {
										std::ostringstream stream;
										stream << sBoneName << nCount;
										pFrameNode->GetBone()->SetName(stream.str());
									}
									else
										pFrameNode->GetBone()->SetName(sBoneName);

									pFrameNode->GetBone()->AutoSetBoneInfoFromName();
								}

								/*const std::string& sBoneName = cmd[1];
								if (!sBoneName.empty())
								{
									int nCount = GetNameAppearanceCount(sBoneName, true);
									if (nCount > 0){
										std::ostringstream stream;
										stream << sBoneName << nCount;
										pFrameNode->GetBone()->SetName(stream.str());
									}
									else
										pFrameNode->GetBone()->SetName(sBoneName);
								}*/

							}
						}
					}
				}
				else if (pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_obstruction) && pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_cubeModel) && !pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid))
				{
					// non-solid, but obstruction blocks such as slope, stairs, and other custom models
					BMaxNodePtr node(new BMaxNode(this, x, y, z, template_id, block_data));
					node->setSolid(false);
					nodes.push_back(node);
				}
				else if(!pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid))
				{
					// non-solid non-obstruction blocks,like web, grass, will not be rendered. but can be used to connect bones
					template_id = TransparentBlockId;
					BMaxBlockModelNodePtr node(new BMaxBlockModelNode(this, x, y, z, template_id, block_data));
					node->setSolid(false);
					nodes.push_back(BMaxNodePtr(node.get()));
				}
				else
				{
					// treat as standard cube
					BMaxNodePtr node(new BMaxNode(this, x, y, z, template_id, block_data));
					nodes.push_back(node);
				}
			}
		}

		CalculateAABB(nodes);
		// set AABB and center
		/*m_blockAABB = aabb;
		m_centerPos = m_blockAABB.GetCenter();
		m_centerPos.y = 0;
		m_centerPos.x = (m_blockAABB.GetWidth() + 1.f) * 0.5f;
		m_centerPos.z = (m_blockAABB.GetDepth() + 1.f) * 0.5f;

		int offset_x = (int)m_blockAABB.GetMin().x;
		int offset_y = (int)m_blockAABB.GetMin().y;
		int offset_z = (int)m_blockAABB.GetMin().z;

		for (auto node : nodes)
		{
			node->x -= offset_x;
			node->y -= offset_y;
			node->z -= offset_z;
			InsertNode(node);
		}*/

		// set scaling;

	}

	void BMaxParser::ParseVisibleBlocks()
	{
		for (uint32 i = 0; i < m_blockModels.size(); i++)
		{
			SAFE_DELETE(m_blockModels[i]);
		}
		m_blockModels.clear();
		for (auto& item : m_nodes)
		{
			BMaxNode* node = item.second.get();
			if (node != NULL)
			{
				if (node->template_id == TransparentBlockId)
				{
					// total skipped for cobweb
				}
				else
				{
					BlockModel* tessellatedModel = new BlockModel();
					if (node->TessellateBlock(tessellatedModel) > 0)
					{
						node->SetBlockModel(tessellatedModel);
						m_blockModels.push_back(tessellatedModel);
					}
					else
					{
						node->SetBlockModel(NULL);
						delete tessellatedModel;
					}
				}
			}
		}
	}
	void BMaxParser::ScaleModels()
	{
		if (m_blockModels.size() == 0)
			return;
		float scale = m_fScale;

		int nSize = (int)m_blockModels.size();
		for (int i = 0; i < nSize; i++)
		{
			BlockModel* model = m_blockModels.at(i);
			int nVertices = model->GetVerticesCount();
			BlockVertexCompressed* pVertices = model->GetVertices();
			for (int k = 0; k < nVertices; k++)
			{
				pVertices[k].position[0] *= scale;
				pVertices[k].position[1] *= scale;
				pVertices[k].position[2] *= scale;
			}
		}
	}

	float BMaxParser::CalculateScale(float length)
	{
		int nPowerOf2Length = Math::NextPowerOf2((int)(length + 0.1f));
		return (float)(BlockConfig::g_blockSize / nPowerOf2Length);
	}

	int64 BMaxParser::GetIndexFromXmlElement(BMaxXMLElement* node, const char* name, int& x, int& y, int& z)
	{
		if (node == NULL)
		{
			return -1;
		}
		const char* id = node->Attribute(name);
		std::string id_str;
		if (id)
		{
			id_str = id;
			std::vector<std::string> ids;
			StringHelper::split(id_str, ",", ids);
			if (ids.size() > 2)
			{
				x = atoi(ids[0].c_str());
				y = atoi(ids[1].c_str());
				z = atoi(ids[2].c_str());
				x -= (int)m_blockAABB.GetMin().x;
				y -= (int)m_blockAABB.GetMin().y;
				z -= (int)m_blockAABB.GetMin().z;

				int64 index = (int64)GetNodeIndex(x, y, z);
				return index;
			}
		}
		return -1;
	}

	BMaxNode* BMaxParser::GetBMaxNode(int x, int y, int z)
	{
		if (x >= 0 && y >= 0 && z >= 0) // && x < 256 && y < 256 && z < 256
			return GetNode((uint16)x, (uint16)y, (uint16)z);
		else
			return NULL;
	}

	int64 BMaxParser::InsertNode(BMaxNodePtr& nodePtr)
	{
		auto index = nodePtr->GetIndex();
		if (GetNode(nodePtr->x, nodePtr->y, nodePtr->z) == NULL)
		{
			m_nodes[index] = nodePtr;
		}
		return index;
	}
	void BMaxParser::ParseHead(BMaxXMLDocument& doc)
	{
		BMaxXMLElement* element = doc.FirstChildElement("pe:blocktemplate");
		if (element != NULL)
		{
			std::string auto_scale;
			ReadValue(element, "auto_scale", auto_scale);
			if (auto_scale == "false" || auto_scale == "False")
			{
				m_bAutoScale = false;
			}
		}
	}

	CParaXModel* BMaxParser::ParseParaXModel()
	{
		ClearModel();
		CParaXModel* pMesh = NULL;
		ParaXHeaderDef m_xheader;
		pMesh = new CParaXModel(m_xheader);
		FillParaXModelData(pMesh);
		pMesh->SetBmaxModel();
		return pMesh;
	}

	CParaXModel * BMaxParser::ParseParaXModel(uint32 nMaxTriangleCount)
	{
		ClearModel();
		CalculateLod(nMaxTriangleCount);

		CParaXModel* pMesh = NULL;
		ParaXHeaderDef m_xheader;
		pMesh = new CParaXModel(m_xheader);
		pMesh->SetBmaxModel();
		FillParaXModelData(pMesh, nMaxTriangleCount);
		if (m_rectangles.size() == 0)
			return pMesh;
		return pMesh;
	}

	const std::string& BMaxParser::GetFilename() const
	{
		return m_filename;
	}

	void BMaxParser::SetFilename(const std::string& val)
	{
		m_filename = val;
	}

	void BMaxParser::SetAutoScale(bool value)
	{
		m_bAutoScale = value;
	}

	bool BMaxParser::IsAutoScale()
	{
		return m_bAutoScale;
	}

	const Vector3& BMaxParser::GetCenterPos() const
	{
		return m_centerPos;
	}



	void BMaxParser::MergeCoplanerBlockFace()
	{
		ParseVisibleBlocks();

		m_rectangles.clear();
		for (auto& item : m_nodes)
		{
			BMaxNode *node = item.second.get();
			if (node->GetBlockModel() && node->isSolid() && !(node->GetBlockModel()->IsUniformLighting()))
			{
				for (int i = 0; i < 6; i++)
				{
					if (node->IsFaceNotUse(i))
					{
						FindCoplanerFace(node, i);
					}
				}
			}
		}

		float fScale = m_fScale;
		if (m_nLodLevel > 0) {
			fScale *= (float)pow(2, m_nLodLevel);
		}

		for (RectanglePtr& rectangle : m_rectangles)
		{
			rectangle->ScaleVertices(fScale);
		}
		// OUTPUT_LOG("rect count %d \n", m_rectangles.size());
	}

	void BMaxParser::CreatRectanglesFromBlocks()
	{
		ParseVisibleBlocks();

		m_rectangles.clear();
		for (auto& item : m_nodes)
		{
			BMaxNode *node = item.second.get();
			if (node->GetBlockModel())
			{
				for (int i = 0; i < 6; i++)
				{
					RectanglePtr rectangle(new Rectangle(node, i));
					rectangle->CloneNodes();
					m_rectangles.push_back(rectangle);
				}
			}
		}

		float fScale = m_fScale;
		if (m_nLodLevel > 0) {
			fScale *= (float)pow(2, m_nLodLevel);
		}

		for (RectanglePtr& rectangle : m_rectangles)
		{
			rectangle->ScaleVertices(fScale);
		}
	}


	bool BMaxParser::IsCoplaneNode(BMaxNode* node1, BMaxNode* node2, int nFaceIndex)
	{
		return (node1 && node2 && node1->isSolid() && node2->isSolid() &&
			node2->IsFaceNotUse(nFaceIndex) &&
			node1->GetColor() == node2->GetColor() &&
			node1->GetBoneIndex() == node2->GetBoneIndex() &&
			node2->GetBlockModel() && node2->GetBlockModel()->GetVerticesCount() > 0);
	}

	void BMaxParser::FindCoplanerFace(BMaxNode* node, uint32 nFaceIndex)
	{
#ifdef USE_COPLANER_ALGORITHM_NON_RECURSIVE
		const Vector3 *directionOffsetTable = Rectangle::DirectionOffsetTable;
		int nIndex = nFaceIndex * 4;
		BMaxNode* coplanar_node = node;

		const Vector3& offset = directionOffsetTable[nIndex];
		int nLength = 0;
		for (nLength = 0; IsCoplaneNode(node, node->GetNeighbourByOffset(offset*(nLength + 1.f)), nFaceIndex); ++nLength)
		{
		}
		int nLength2 = 0;
		for (nLength2 = 0; IsCoplaneNode(node, node->GetNeighbourByOffset(-offset * (nLength2 + 1.f)), nFaceIndex); ++nLength2)
		{
		}

		const Vector3& offset2 = directionOffsetTable[nIndex + 1];
		int nLength1 = 0;
		bool bBreak = false;
		for (nLength1 = 0; !bBreak; ++nLength1)
		{
			for (int i = -nLength2; i <= nLength; i++)
			{
				if (!IsCoplaneNode(node, node->GetNeighbourByOffset(offset2*(nLength1 + 1.f) + offset * (float)i), nFaceIndex))
				{
					nLength1 = nLength1 - 1;
					bBreak = true;
					break;
				}
			}
		}
		int nLength3 = 0;
		bBreak = false;
		for (nLength3 = 0; !bBreak; ++nLength3)
		{
			for (int i = -nLength2; i <= nLength; i++)
			{
				if (!IsCoplaneNode(node, node->GetNeighbourByOffset(-offset2 * (nLength3 + 1.f) + offset * (float)i), nFaceIndex))
				{
					nLength3 = nLength3 - 1;
					bBreak = true;
					break;
				}
			}
		}
		// mark all faces as used
		node->SetFaceUsed(nFaceIndex);
		for (int i = -nLength2; i <= nLength; i++)
		{
			for (int j = -nLength3; j <= nLength1; j++)
			{
				auto node_ = node->GetNeighbourByOffset(offset * (float)i + offset2 * (float)j);
				if (node_)
					node_->SetFaceUsed(nFaceIndex);
			}
		}

		RectanglePtr rectangle(new Rectangle(node, nFaceIndex));
		m_rectangles.push_back(rectangle);
		rectangle->SetCornerNode(node->GetNeighbourByOffset(offset * (float)nLength - offset2 * (float)nLength3), 1);
		rectangle->SetCornerNode(node->GetNeighbourByOffset(offset2 * (float)nLength1 + offset * (float)nLength), 2);
		rectangle->SetCornerNode(node->GetNeighbourByOffset(-offset * (float)nLength2 + offset2 * (float)nLength1), 3);
		rectangle->SetCornerNode(node->GetNeighbourByOffset(-offset2 * (float)nLength3 - offset * (float)nLength2), 0);

#else
		const uint16 nVertexCount = 4;

		BMaxNodePtr nodes[nVertexCount] =
		{
			BMaxNodePtr(node), BMaxNodePtr(node), BMaxNodePtr(node), BMaxNodePtr(node)
		};

		RectanglePtr rectangle(new Rectangle(nodes, nFaceIndex));
		for (uint32 i = 0; i < nVertexCount; i++)
		{
			FindNeighbourFace(rectangle.get(), i, nFaceIndex);
			node->SetFaceUsed(nFaceIndex);
		}

		rectangle->CloneNodes();
		m_rectangles.push_back(rectangle);
#endif
	}

	void BMaxParser::FindNeighbourFace(Rectangle *rectangle, uint32 i, uint32 nFaceIndex)
	{
		const Vector3 *directionOffsetTable = Rectangle::DirectionOffsetTable;
		int nIndex = nFaceIndex * 4 + i;
		PE_ASSERT(nIndex < 24);
		const Vector3& offset = directionOffsetTable[nIndex];

		int nextI = nIndex + ((i == 3) ? -3 : 1);

		PE_ASSERT(nextI < 24);
		BMaxNode *fromNode = rectangle->GetFromNode(nextI);
		BMaxNode *toNode = rectangle->GetToNode(nextI);

		const Vector3& nextOffset = directionOffsetTable[nextI];
		BMaxNode *currentNode = fromNode;

		vector<BMaxNodePtr>nodes;

		if (fromNode)
		{
			do
			{
				BMaxNode *neighbourNode = currentNode->GetNeighbourByOffset(offset);
				if (!currentNode->isSolid() || neighbourNode == NULL || !neighbourNode->isSolid() || currentNode->GetColor() != neighbourNode->GetColor() || currentNode->GetBoneIndex() != neighbourNode->GetBoneIndex())
					return;
				BlockModel* neighbourCube = neighbourNode->GetBlockModel();

				if (neighbourCube && neighbourCube->GetVerticesCount() > 0 && neighbourNode->IsFaceNotUse(nFaceIndex))
					nodes.push_back(BMaxNodePtr(neighbourNode));
				else
					return;

				if (currentNode == toNode)
					break;
				currentNode = currentNode->GetNeighbourByOffset(nextOffset);
			} while (currentNode);
		}

		BMaxNode *newFromNode = fromNode->GetNeighbourByOffset(offset);
		BMaxNode *newToNode = toNode->GetNeighbourByOffset(offset);

		for (BMaxNodePtr nodePtr : nodes)
		{
			nodePtr->SetFaceUsed(nFaceIndex);
		}
		rectangle->UpdateNode(newFromNode, newToNode, nextI);
		FindNeighbourFace(rectangle, i, nFaceIndex);
	}

	void BMaxParser::CalculateLod(uint32 nMaxTriangleCount)
	{
		/*
		MergeCoplanerBlockFace(m_rectangles);
		//m_originRectangles = rectangles;

		if (fabs(m_fScale - 1.0f) > FLT_EPSILON)
		{
			for (RectanglePtr& rectangle : rectangles)
			{
				rectangle->ScaleVertices(m_fScale);
			}
		}

		vector<uint32> lodTable;
		GetLodTable(rectangles.size(), lodTable);
		for (uint16 i = 0;i < lodTable.size();i++)
		{
			uint32 nextFaceCount = lodTable[i];
			while (rectangles.size() > nextFaceCount)
			{
				PerformLod();
				rectangles.clear();
				MergeCoplanerBlockFace(rectangles);
			}
			if (fabs(m_fScale - 1.0f) > FLT_EPSILON)
			{
				for (RectanglePtr& rectangle : rectangles)
				{
					rectangle->ScaleVertices(m_fScale);
				}
			}
			m_lodRectangles[i] = rectangles;
		}
		*/

		//a rectangle generate two triangles
		while (m_rectangles.size() * 2 > nMaxTriangleCount)
		{
			PerformLod();
			MergeCoplanerBlockFace();
		}

	}

	void BMaxParser::GetLodTable(uint32 faceCount, vector<uint32>&lodTable)
	{
		if (faceCount >= 4000)
			lodTable.push_back(4000);
		if (faceCount >= 2000)
			lodTable.push_back(2000);
		if (faceCount >= 500)
			lodTable.push_back(500);
	}

	void BMaxParser::PerformLod()
	{
		m_nLodLevel++;

		CShapeAABB aabb;
		map<int64, BMaxNodePtr> nodesMap;

		int width = (int)m_blockAABB.GetWidth();
		int height = (int)m_blockAABB.GetHeight();
		int depth = (int)m_blockAABB.GetDepth();

		for (int direction = 0; direction <= 3; direction++)
		{
			int x = (int)m_centerPos[0];
			while (x >= -1 && x <= width)
			{
				for (int y = 0; y <= height; y += 2)
				{
					int z = (int)m_centerPos[2];
					while (z >= -1 && z <= depth)
					{
						CalculateLodNode(nodesMap, x, y, z);
						(direction & 1) == 0 ? z += 2 : z -= 2;
					}
				}
				if (direction >= 2)
					x += 2;
				else
					x -= 2;
			}
		}
		vector<BMaxNodePtr>nodes;

		for (auto iter = nodesMap.begin(); iter != nodesMap.end(); iter++)
		{
			nodes.push_back(iter->second);
		}
		CalculateAABB(nodes);
	}

	void BMaxParser::CalculateAABB(vector<BMaxNodePtr>&nodes)
	{
		m_blockAABB.SetEmpty();
		for (auto item : nodes)
		{
			BMaxNode *node = item.get();
			m_blockAABB.Extend(Vector3((float)node->x, (float)node->y, (float)node->z));
		}

		m_centerPos = m_blockAABB.GetCenter();
		m_centerPos.y = 0;
		m_centerPos.x = (m_blockAABB.GetWidth() + 1.f) * 0.5f;
		m_centerPos.z = (m_blockAABB.GetDepth() + 1.f) * 0.5f;

		// OUTPUT_LOG("center %f %f %f\n", m_centerPos[0], m_centerPos[1], m_centerPos[2]);
		const auto& vMin = m_blockAABB.GetMin();
		int offset_x = (int)vMin.x;
		int offset_y = (int)vMin.y;
		int offset_z = (int)vMin.z;

		m_nodes.clear();
		for (auto node : nodes)
		{
			node->x -= offset_x;
			node->y -= offset_y;
			node->z -= offset_z;
			InsertNode(node);
		}

		// OUTPUT_LOG("nodes count %d\n", m_nodes.size());
		if (m_bAutoScale && m_nLodLevel == 0)
		{
			float fMaxLength = Math::Max(Math::Max(m_blockAABB.GetHeight(), m_blockAABB.GetWidth()), m_blockAABB.GetDepth()) + 1.f;
			m_fScale = CalculateScale(fMaxLength);
			if (m_bHasBoneBlock)
			{
				// for animated models, it is by default 1-2 blocks high, for static models, it is 0-1 block high. 
				m_fScale *= 2.f;
			}
		}
	}

	void BMaxParser::CalculateLodNode(map<int64, BMaxNodePtr> &nodeMap, int x, int y, int z)
	{
		int32 cnt = 0;

		map<int32, int32> colorMap;
		map<int32, int32> boneMap;

		for (int16 dx = 0; dx <= 1; dx++)
		{
			for (int16 dy = 0; dy <= 1; dy++)
			{
				for (int16 dz = 0; dz <= 1; dz++)
				{
					int16 cx = x + dx;
					int16 cy = y + dy;
					int16 cz = z + dz;

					if (cx >= 0 && cy >= 0 && cz >= 0)
					{
						BMaxNode *node = GetNode(cx, cy, cz);
						if (node)
						{
							cnt++;
							bool hasFind = false;
							int boneIndex = node->GetBoneIndex();
							if (boneIndex >= 0)
							{
								BMaxFrameNode *myBone = m_bones[boneIndex].get();
								for (auto iter = boneMap.begin(); iter != boneMap.end(); iter++)
								{
									BMaxFrameNode *bone = m_bones[iter->first].get();
									if (boneIndex == iter->first || bone->IsAncestorOf(myBone))
									{
										iter->second++;
										hasFind = true;
										break;
									}
									else if (myBone->IsAncestorOf(bone))
									{
										boneMap.insert(make_pair(boneIndex, iter->second + 1));
										boneMap.erase(iter);
										hasFind = true;
										break;
									}
								}
								if (!hasFind)
								{
									boneMap.insert(make_pair(boneIndex, 1));
								}
							}

							int32 myColor = node->GetColor();
							auto iter = colorMap.find(myColor);
							if (iter != colorMap.end())
							{
								iter->second++;
							}
							else
							{
								colorMap.insert(make_pair(myColor, 1));
							}
						}
					}
				}
			}
		}

		if (cnt >= 4)
		{
			uint16 newX = (x + 1) / 2;
			uint16 newY = y / 2;
			uint16 newZ = (z + 1) / 2;
			uint64 index = GetNodeIndex(newX, newY, newZ);

			BMaxNodePtr node(new BMaxNode(this, newX, newY, newZ, 0, 0));

			int32 color = 0;
			auto itColor = std::max_element(colorMap.begin(), colorMap.end(), [](const std::pair<int32, int32>& a, const std::pair<int32, int32>& b)
			{
				return a.second < b.second;
			});

			if (itColor != colorMap.end())
				color = itColor->first;
			node->SetColor(color);

			int32 boneIndex = -1;
			auto itBone = std::max_element(boneMap.begin(), boneMap.end(), [](const std::pair<int32, int32>& a, const std::pair<int32, int32>& b)
			{
				return a.second < b.second;
			});
			if (itBone != boneMap.end())
				boneIndex = itBone->first;
			node->SetBoneIndex(boneIndex);

			if (nodeMap.find(index) == nodeMap.end())
			{
				nodeMap.insert(make_pair(index, node));
			}
		}
	}

	ModelGeoset* BMaxParser::AddGeoset()
	{
		ModelGeoset geoset;
		memset(&geoset, 0, sizeof(geoset));
		geoset.id = (uint16_t)m_geosets.size();
		m_geosets.push_back(geoset);
		return &(m_geosets.back());
	}

	ModelRenderPass* BMaxParser::AddRenderPass()
	{
		ModelRenderPass pass;
		pass.cull = true;
		pass.texanim = -1;
		pass.color = -1;
		pass.opacity = -1;
		m_renderPasses.push_back(pass);
		return &(m_renderPasses.back());
	}

	BMaxAnimGenerator* BMaxParser::GetAnimGenerator()
	{
		if (!m_pAnimGenerator)
			m_pAnimGenerator = new BMaxAnimGenerator(this);
		return m_pAnimGenerator;
	}

	void BMaxParser::FillVerticesAndIndices(int32 nMaxTriangleCount)
	{
		ModelGeoset* geoset = AddGeoset();
		ModelRenderPass* pass = AddRenderPass();
		pass->geoset = geoset->id;

		int32 nStartIndex = 0;

		CShapeAABB aabb;
		int total_count = 0;
		int nStartVertex = 0;

		int nRootBoneIndex = 0;
		for (uint32 i = 0; i < m_rectangles.size(); i++)
		{
			Rectangle *rectangle = m_rectangles[i].get();
			BlockVertexCompressed* pVertices = rectangle->GetVertices();

			int nVertices = 4;
			int nIndexCount = 6;

			if ((nIndexCount + geoset->icount) >= 0xffff)
			{
				// break geoset, if it is too big
				nStartIndex = (int32)m_indices.size();
				geoset = AddGeoset();
				pass = AddRenderPass();
				pass->geoset = geoset->id;
				pass->SetStartIndex(nStartIndex);
				geoset->SetVertexStart(total_count);
				nStartVertex = 0;
			}

			geoset->icount += nIndexCount;
			pass->indexCount += nIndexCount;

			uint8 vertex_weight = 0xff;

			for (int k = 0; k < nVertices; k++, pVertices++)
			{
				ModelVertex modelVertex;
				memset(&modelVertex, 0, sizeof(ModelVertex));
				pVertices->GetPosition(modelVertex.pos);
				pVertices->GetNormal(modelVertex.normal);

				modelVertex.color0 = pVertices->color2;
				//set bone and weight, only a single bone
				int nBoneIndex = rectangle->GetBoneIndexAt(k);
				// if no bone is found, use the default root bone
				modelVertex.bones[0] = (nBoneIndex != -1) ? nBoneIndex : nRootBoneIndex;
				modelVertex.weights[0] = vertex_weight;

				m_vertices.push_back(modelVertex);
				aabb.Extend(modelVertex.pos);
			}

			int start_index = nStartVertex;
			m_indices.push_back(start_index + 0);
			m_indices.push_back(start_index + 1);
			m_indices.push_back(start_index + 2);
			m_indices.push_back(start_index + 0);
			m_indices.push_back(start_index + 2);
			m_indices.push_back(start_index + 3);
			total_count += nVertices;
			nStartVertex += nVertices;
		}

		if (nMaxTriangleCount < 0)
		{
			for (auto& item : m_nodes)
			{
				BMaxNode* node = item.second.get();
				if (!node->isSolid() && node->GetParaXModel() == 0)
				{
					// for stairs, slabs, buttons, etc
					BlockModel* model = node->GetBlockModel();
					if (model != 0 && model->IsUniformLighting())
					{
						int nFromVertex = (int)m_vertices.size();

						int nVertices = model->GetVerticesCount();
						BlockVertexCompressed* pVertices = model->GetVertices();
						int nFace = model->GetFaceCount();

						int nIndexCount = nFace * 6;

						if ((nIndexCount + geoset->icount) >= 0xffff)
						{
							// break geoset, if it is too big
							nStartIndex = (int32)m_indices.size();
							geoset = AddGeoset();
							pass = AddRenderPass();
							pass->geoset = geoset->id;
							pass->SetStartIndex(nStartIndex);
							geoset->SetVertexStart((int32)m_vertices.size());
						}

						geoset->icount += nIndexCount;
						pass->indexCount += nIndexCount;

						int nBoneIndex = node->GetBoneIndex();
						// if no bone is found, use the default root bone
						if (nBoneIndex == -1)
							nBoneIndex = nRootBoneIndex;
						uint8 vertex_weight = 0xff;

						for (int k = 0; k < nVertices; k++, pVertices++)
						{
							ModelVertex modelVertex;
							memset(&modelVertex, 0, sizeof(ModelVertex));
							pVertices->GetPosition(modelVertex.pos);
							modelVertex.pos *= m_fScale;
							pVertices->GetNormal(modelVertex.normal);
							modelVertex.color0 = pVertices->color2;
							//set bone and weight, only a single bone
							modelVertex.bones[0] = nBoneIndex;
							modelVertex.weights[0] = vertex_weight;

							m_vertices.push_back(modelVertex);
							aabb.Extend(modelVertex.pos);
						}

						int nVertexOffset = nFromVertex - geoset->GetVertexStart();
						for (int k = 0; k < nFace; k++)
						{
							int start_index = k * 4 + nVertexOffset;
							m_indices.push_back(start_index + 0);
							m_indices.push_back(start_index + 1);
							m_indices.push_back(start_index + 2);
							m_indices.push_back(start_index + 0);
							m_indices.push_back(start_index + 2);
							m_indices.push_back(start_index + 3);
						}
					}
				}
				else if (node->GetParaXModel())
				{
					// for BlockModel like bmax, x, etc
					int nBoneIndex = node->GetBoneIndex();
					// if no bone is found, use the default root bone
					if (nBoneIndex == -1)
						nBoneIndex = nRootBoneIndex;
					uint8 vertex_weight = 0xff;

					int nFromIndices = (int)m_indices.size();
					int total_count = (int)m_vertices.size();
					int nFromVertex = total_count;
					CParaXModel* pModel = node->GetParaXModel();

					Matrix4 matLocalTrans;
					bool m_bHasTransform = node->HasTransform();
					if (m_bHasTransform)
					{
						matLocalTrans = node->GetTransform();
					}
					int nPasses = (int)pModel->passes.size();
					int nTotalFaceCount = 0;

					const Vector3& vCenter = GetCenterPos();
					Vector3 vOffset((float)node->x - vCenter.x + BlockConfig::g_half_blockSize, (float)node->y, (float)node->z - vCenter.z + BlockConfig::g_half_blockSize);

					ModelVertex *ov = pModel->m_origVertices;
					int nModelVerticesCount = pModel->GetObjectNum().nVertices;
					for (int i = 0; i < nModelVerticesCount; i++)
					{
						auto ov = pModel->m_origVertices + i;
						ModelVertex modelVertex;
						memset(&modelVertex, 0, sizeof(ModelVertex));

						if (m_bHasTransform)
						{
							modelVertex.pos = (ov->pos*matLocalTrans + vOffset);
							modelVertex.normal = (ov->normal*matLocalTrans);
						}
						else
						{
							modelVertex.pos = (ov->pos + vOffset);
							modelVertex.normal = (ov->normal);
						}

						modelVertex.pos *= m_fScale;
						modelVertex.color0 = ov->color0;
						//set bone and weight, only a single bone
						modelVertex.bones[0] = nBoneIndex;
						modelVertex.weights[0] = vertex_weight;

						m_vertices.push_back(modelVertex);
						aabb.Extend(modelVertex.pos);
					}

					for (int nPass = 0; nPass < nPasses; nPass++)
					{
						ModelRenderPass &p = pModel->passes[nPass];
						if (pModel->showGeosets[p.geoset])
						{
							int nIndexCount = p.indexCount;
							int nIndexOffset = p.GetStartIndex();

							if ((nIndexCount + geoset->icount) >= 0xffff)
							{
								// break geoset, if it is too big
								nStartIndex = (int32)m_indices.size();
								geoset = AddGeoset();
								pass = AddRenderPass();
								pass->geoset = geoset->id;
								pass->SetStartIndex(nStartIndex);
								geoset->SetVertexStart(total_count);
							}

							geoset->icount += nIndexCount;
							pass->indexCount += nIndexCount;

							int nVertexOffset = p.GetVertexStart(pModel) + nFromVertex - geoset->GetVertexStart();

							for (int i = 0; i < p.indexCount; ++i)
							{
								int a = pModel->m_indices[nIndexOffset + i] + nVertexOffset;
								m_indices.push_back((uint16)a);
							}
						}
					}
				}
			}
		}

		aabb.GetMin(m_minExtent);
		aabb.GetMax(m_maxExtent);
	}

	void BMaxParser::ClearModel()
	{
		m_geosets.clear();
		m_renderPasses.clear();
		m_indices.clear();
		m_vertices.clear();
	}

	void BMaxParser::FillParaXModelData(CParaXModel *pMesh, int32 nMaxTriangleCount)
	{
		if (pMesh == NULL)
		{
			return;
		}
		FillVerticesAndIndices(nMaxTriangleCount);
		pMesh->m_objNum.nVertices = m_vertices.size();
		if (m_vertices.size() == 0)
			return;

		pMesh->m_objNum.nBones = m_bones.size();
		pMesh->m_objNum.nAnimations = m_bones.size() > 0 ? m_anims.size() : 0;
		pMesh->m_objNum.nIndices = m_indices.size();
		pMesh->m_header.minExtent = m_minExtent;
		pMesh->m_header.maxExtent = m_maxExtent;
		
		//pMesh->m_RenderMethod = CParaXModel::BMAX_MODEL;
		pMesh->SetRenderMethod(CParaXModel::BMAX_MODEL);
		pMesh->initVertices(m_vertices.size(), &(m_vertices[0]));
		pMesh->initIndices(m_indices.size(), &(m_indices[0]));

		if (m_bones.size() > 0)
		{
			pMesh->bones = new ParaEngine::Bone[m_bones.size()];
			for (int i = 0; i < (int)m_bones.size(); ++i)
			{
				Bone* pBone = m_bones[i]->GetBone();
				if (!pBone->rot.used && !pBone->scale.used && !pBone->trans.used)
				{
					pBone->SetStaticTransform(Matrix4::IDENTITY);
					pBone->bUsePivot = true;
				}
				pMesh->bones[i] = *pBone;
				if (pBone->nBoneID > 0)
					pMesh->m_boneLookup[pBone->nBoneID] = i;
				else if (pBone->IsAttachment())
				{
					pMesh->NewAttachment(true, pBone->GetAttachmentId(), i, pBone->bUsePivot ? pBone->pivot : Vector3::ZERO);
				}
			}
		}

		if (m_bones.size() > 0)
		{
			if (m_anims.size() > 0)
			{
				pMesh->anims = new ModelAnimation[m_anims.size()];
				memcpy(pMesh->anims, &(m_anims[0]), sizeof(ModelAnimation)*m_anims.size());
			}
			pMesh->animBones = true;
			pMesh->animated = true;
		}
		else
		{
			pMesh->animBones = false;
			pMesh->animated = false;
		}

		// add geoset (faces & indices)
		{
			pMesh->geosets = m_geosets;
			pMesh->passes = m_renderPasses;
		}

		if (pMesh->geosets.size() > 0)
		{
			pMesh->showGeosets = new bool[pMesh->geosets.size()];
			memset(pMesh->showGeosets, true, pMesh->geosets.size() * sizeof(bool));
		}
	}

	int BMaxParser::GetBoneIndex(uint16 x, uint16 y, uint16 z)
	{
		int nBoneIndex = -1;
		BMaxNode* pBone = GetNode(x, y, z);
		if (pBone && pBone->ToBoneNode())
		{
			nBoneIndex = pBone->GetBoneIndex();
		}
		return nBoneIndex;
	}

	bool BMaxParser::ReadValue(BMaxXMLElement* node, const char* name, int32_t& value)
	{
		if (node)
		{
			const char* attr = node->Attribute(name);
			if (attr)
			{
				value = atoi(attr);
				return true;
			}
		}
		return false;
	}
	bool BMaxParser::ReadValue(BMaxXMLElement* node, const char* name, float& value)
	{
		if (node)
		{
			const char* attr = node->Attribute(name);
			if (attr)
			{
				value = (float)atof(attr);
				return true;
			}
		}
		return false;
	}
	bool BMaxParser::ReadValue(BMaxXMLElement* node, const char* name, Vector3& value)
	{
		if (node)
		{
			const char* attr = node->Attribute(name);
			std::string attr_str;
			if (attr)
			{
				attr_str = attr;
				std::vector<std::string> ids;
				StringHelper::split(attr_str, ",", ids);
				if (ids.size() > 2)
				{
					value.x = (float)atof(ids[0].c_str());
					value.y = (float)atof(ids[1].c_str());
					value.z = (float)atof(ids[2].c_str());
					return true;
				}
			}
		}
		return false;
	}
	bool BMaxParser::ReadValue(BMaxXMLElement* node, const char* name, std::string& value)
	{
		if (node)
		{
			const char* attr = node->Attribute(name);
			if (attr)
			{
				value = attr;
				return true;
			}
		}
		return false;
	}
	bool BMaxParser::ReadValue(BMaxXMLElement* node, const char* name, Vector4& value)
	{
		if (node)
		{
			const char* attr = node->Attribute(name);
			std::string attr_str;
			if (attr)
			{
				attr_str = attr;
				std::vector<std::string> ids;
				StringHelper::split(attr_str, ",", ids);
				if (ids.size() > 3)
				{
					value.x = (float)atof(ids[0].c_str());
					value.y = (float)atof(ids[1].c_str());
					value.z = (float)atof(ids[2].c_str());
					value.w = (float)atof(ids[3].c_str());
					return true;
				}
			}
		}
		return false;
	}

	int BMaxParser::GetNameAppearanceCount(const std::string& name, bool bAdd /*= true*/)
	{
		int nLastAppearance = 0;
		auto iter = m_name_occurances.find(name);
		if (iter != m_name_occurances.end())
		{
			nLastAppearance = iter->second;
		}
		if (bAdd)
		{
			m_name_occurances[name] = nLastAppearance + 1;
		}
		return nLastAppearance;
	}

	void BMaxParser::ParseBlockAnimations(BMaxXMLDocument& doc)
	{
		BMaxXMLElement* element = doc.FirstChildElement("pe:blocktemplate");
		if (element != NULL)
		{
			BMaxXMLElement* blockanimations_element = element->FirstChildElement("pe:blockanimations");
			if (blockanimations_element != NULL)
			{
				for (BMaxXMLNode* pChild = blockanimations_element->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
				{
					BMaxXMLElement* pElement = pChild->ToElement();
					if (pElement)
					{
						ParseBlockAnimationSet(pElement);
					}
				}
			}
		}
	}
	void BMaxParser::ParseBlockAnimationSet(BMaxXMLElement* node)
	{
		if (node == NULL)
		{
			return;
		}
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = 0;
		int32_t duration;
		if (ReadValue(node, "duration", duration))
		{
			anim.timeEnd = duration;

			m_bHasAnimation = true;
		}
		anim.animID = m_anims.size();
		m_anims.push_back(anim);

		for (BMaxXMLNode* pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
		{
			BMaxXMLElement* pElement = pChild->ToElement();
			if (pElement)
			{
				ParseBlockAnimation(pElement);
			}
		}

	}
	void BMaxParser::ParseBlockAnimation(BMaxXMLElement* node)
	{
		if (node == NULL)
		{
			return;
		}
		int x;
		int y;
		int z;
		auto index = GetIndexFromXmlElement(node, "frame_id", x, y, z);
		if (index > -1)
		{
			int nIndex = 0;
			for (BMaxXMLNode* pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
			{
				BMaxXMLElement* pElement = pChild->ToElement();
				if (pElement)
				{
					ParseBlockAnimationKeys(x, y, z, pElement, nIndex);
					nIndex++;
				}
			}
		}
	}
	void BMaxParser::ParseBlockAnimationKeys(uint16 x, uint16 y, uint16 z, BMaxXMLElement* node, int nIndex)
	{
		int bone_index = GetBoneIndex(x, y, z);
		if (bone_index > -1)
		{
			ParaEngine::Bone & bone = *(m_bones[bone_index]->GetBone());
			bone.flags = ParaEngine::Bone::BONE_USE_PIVOT;
			bone.calc = true;
			std::string value;
			if (ReadValue(node, "property", value))
			{
				for (BMaxXMLNode* pChild = node->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
				{
					BMaxXMLElement* pElement = pChild->ToElement();
					if (pElement)
					{
						ParseBlockAnimationKey(pElement, &bone, value);
					}
				}
				int nAnimId = nIndex;
				bone.scale.ranges.resize(nAnimId + 1, AnimRange(0, 0));
				bone.trans.ranges.resize(nAnimId + 1, AnimRange(0, 0));
				bone.rot.ranges.resize(nAnimId + 1, AnimRange(0, 0));
				bone.scale.ranges[nAnimId] = AnimRange(0, bone.scale.times.size() - 1);
				bone.trans.ranges[nAnimId] = AnimRange(0, bone.trans.times.size() - 1);
				bone.rot.ranges[nAnimId] = AnimRange(0, bone.rot.times.size() - 1);
			}
		}
	}
	void BMaxParser::ParseBlockAnimationKey(BMaxXMLElement* node, Bone* bone, const std::string propertyType)
	{
		if (node == NULL || bone == NULL)
		{
			return;
		}
		if (propertyType == "position")
		{
			bone->trans.used = true;
			int time;
			Vector3 value;
			if (ReadValue(node, "time", time) && ReadValue(node, "value", value))
			{
				bone->trans.times.push_back(time);
				bone->trans.data.push_back(value);
			}
		}
		if (propertyType == "rotation")
		{
			bone->rot.used = true;
			int time;
			Vector4 value;
			if (ReadValue(node, "time", time) && ReadValue(node, "value", value))
			{
				bone->rot.times.push_back(time);
				Quaternion rot(value.x, value.y, value.z, value.w);
				bone->rot.data.push_back(rot);
			}
		}
		if (propertyType == "scale")
		{
			bone->scale.used = true;
			int time;
			Vector3 value;
			if (ReadValue(node, "time", time) && ReadValue(node, "value", value))
			{
				bone->scale.times.push_back(time);
				bone->scale.data.push_back(value);
			}
		}
	}

	BMaxFrameNode* BMaxParser::GetFrameNode(int16 x, int16 y, int16 z)
	{
		BMaxNode* pNode = GetBMaxNode(x, y, z);
		if (pNode)
			return pNode->ToBoneNode();
		else
			return NULL;
	}

	void BMaxParser::ParseBlockFrames()
	{
		// calculate parent bones
		for (auto bone : m_bones)
			bone->UpdatePivot();
		// calculate parent bones
		for (auto bone : m_bones)
			bone->GetParentBone(true);
		// set bone name
		for (auto bone : m_bones)
			bone->AutoSetBoneName();

		GetAnimGenerator()->FillAnimations();

		// create animation
		//CreateDefaultAnimations();
	}

	void BMaxParser::CalculateBoneWeightForBlock(BMaxFrameNode* pBoneNode, BMaxNode* node, bool bMustBeSameColor)
	{
#if 0
		if (node && !node->HasBoneWeight())
		{
			if (node->template_id != BoneBlockId)
			{
				if (!bMustBeSameColor || (node->GetColor() == pBoneNode->GetColor()))
				{
					node->SetBoneIndex(pBoneNode->GetBoneIndex());
					for (int i = 0; i < 6; i++)
					{
						CalculateBoneWeightForBlock(pBoneNode, node->GetNeighbour((BlockDirection::Side)i), bMustBeSameColor);
					}
				}
			}
		}
#else
		std::stack<BMaxNode*> nodeStack;
		if (node)
			nodeStack.push(node);

		while (!nodeStack.empty())
		{
			auto cur = nodeStack.top();
			nodeStack.pop();

			if (!cur->HasBoneWeight()
				&& cur->template_id != BoneBlockId
				&& (!bMustBeSameColor || cur->GetColor() == pBoneNode->GetColor()))
			{
				cur->SetBoneIndex(pBoneNode->GetBoneIndex());
				for (int i = 0; i < 6; i++)
				{
					auto neighbour = cur->GetNeighbour((BlockDirection::Side)i);
					if (neighbour)
						nodeStack.push(neighbour);
				}

			}

		}
#endif
	}

	void BMaxParser::CalculateBoneSkin(BMaxFrameNode* pBoneNode)
	{
		if (pBoneNode->HasBoneWeight())
			return;
		// ensures that parent bone node is always calculated before child node. 
		BMaxFrameNode* pParentBoneNode = pBoneNode->GetParent();
		if (pParentBoneNode && !pParentBoneNode->HasBoneWeight())
			CalculateBoneSkin(pParentBoneNode);

		// add current bone to skin.
		pBoneNode->SetBoneIndex(pBoneNode->GetBoneIndex());

		// calculate bone color if not. 
		DWORD bone_color = pBoneNode->GetColor();

		// add other blocks connected to the bone recursively. 
		BlockDirection::Side mySide = BlockDirection::GetBlockSide(pBoneNode->block_data);
		for (int i = 0; i < 6; i++)
		{
			BlockDirection::Side side = BlockDirection::GetBlockSide(i);
			if (mySide != side)
			{
				CalculateBoneWeightForBlock(pBoneNode, pBoneNode->GetNeighbour(side));
			}
		}
	}

	int BMaxParser::FindRootBoneIndex()
	{
		for (auto pBone : m_bones)
		{
			if (!pBone->HasParent())
				return pBone->GetBoneIndex();
		}
		return 0;
	}

	void BMaxParser::CalculateBoneWeightFromNeighbours(BMaxNode* node)
	{
#ifdef USE_RECURSION_FOR_BONEWEIGHT_CALCULATION
		if (node != NULL && !node->HasBoneWeight())
		{
			bool bFoundBone = false;
			for (int i = 0; i < 6 && !bFoundBone; i++)
			{
				BlockDirection::Side side = BlockDirection::GetBlockSide(i);
				BMaxNode* pNeighbourNode = node->GetNeighbour(side);
				if (pNeighbourNode && pNeighbourNode->HasBoneWeight())
				{
					node->SetBoneIndex(pNeighbourNode->GetBoneIndex());
					bFoundBone = true;
				}
			}
			if (bFoundBone)
			{
				for (int i = 0; i < 6; i++)
				{
					BlockDirection::Side side = BlockDirection::GetBlockSide(i);
					BMaxNode* pNeighbourNode = node->GetNeighbour(side);
					if (pNeighbourNode && !pNeighbourNode->HasBoneWeight())
					{
						CalculateBoneWeightFromNeighbours(pNeighbourNode);
					}
				}
			}
		}
#else
		std::stack<BMaxNode*> nodeStack;
		if (node)
			nodeStack.push(node);

		while (!nodeStack.empty())
		{
			auto cur = nodeStack.top();
			nodeStack.pop();

			if (!cur->HasBoneWeight())
			{
				bool bFoundBone = false;
				for (int i = 0; i < 6 && !bFoundBone; i++)
				{
					BlockDirection::Side side = BlockDirection::GetBlockSide(i);
					BMaxNode* pNeighbourNode = cur->GetNeighbour(side);
					if (pNeighbourNode && pNeighbourNode->HasBoneWeight())
					{
						cur->SetBoneIndex(pNeighbourNode->GetBoneIndex());
						bFoundBone = true;
					}
				}
				if (bFoundBone)
				{
					for (int i = 0; i < 6; i++)
					{
						BlockDirection::Side side = BlockDirection::GetBlockSide(i);
						BMaxNode* pNeighbourNode = cur->GetNeighbour(side);
						if (pNeighbourNode && !pNeighbourNode->HasBoneWeight())
						{
							nodeStack.push(pNeighbourNode);
						}
					}
				}
			}
		}
#endif
	}

	void BMaxParser::CalculateBoneWeights()
	{
		// pass 1: calculate all blocks directly connected to bone block and share the same bone color
		for (auto bone : m_bones)
		{
			CalculateBoneSkin(bone.get());
		}

		// pass 2: from remaining blocks, calculate blocks which are connected to bones, but with different colors to those bones. 
		for (auto bone : m_bones)
		{
			BlockDirection::Side mySide = BlockDirection::GetBlockSide(bone->block_data);
			for (int i = 0; i < 6; i++)
			{
				BlockDirection::Side side = BlockDirection::GetBlockSide(i);
				if (mySide != side)
				{
					CalculateBoneWeightForBlock(bone.get(), bone->GetNeighbour(side), false);
				}
			}
		}

		// pass 3: from remaining blocks, calculate blocks which are connected to other binded blocks, but with different colors to those blocks.
		for (auto& item : m_nodes)
		{
			CalculateBoneWeightFromNeighbours(item.second.get());
		}
	}

	void BMaxParser::CreateDefaultAnimations()
	{
		m_bHasAnimation = true;

		// static animation 0
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = 0;
		anim.timeEnd = 0;
		anim.animID = 0;
		m_anims.push_back(anim);

		// walk animations 
		AutoAddWalkAnimation(ANIM_WALK, 4000, 5000, 4.0f, true);
		AutoAddWalkAnimation(ANIM_WALKBACKWARDS, 13000, 14000, 4.0f, false);
	}

	bool BMaxParser::IsFileNameRecursiveLoaded(const std::string& filename)
	{
		if (!filename.empty())
		{
			if (GetFilename() == filename)
				return true;
			else if (GetParent())
			{
				return GetParent()->IsFileNameRecursiveLoaded(filename);
			}
		}
		return false;
	}

	void BMaxParser::AddAnimation(const ModelAnimation& anim)
	{
		m_anims.push_back(anim);
	}

	int BMaxParser::GetAnimationsCount()
	{
		return (int)m_anims.size();
	}

	void BMaxParser::AutoAddWalkAnimation(int nAnimID, int nStartTime, int nEndTime, float fMoveSpeed, bool bMoveForward)
	{
		ModelAnimation anim;
		memset(&anim, 0, sizeof(ModelAnimation));
		anim.timeStart = nStartTime;
		anim.timeEnd = nEndTime;
		anim.animID = nAnimID;
		anim.moveSpeed = fMoveSpeed;
		int animIndex = (int)m_anims.size();
		int nAnimLength = anim.timeEnd - anim.timeStart;
		for (auto bone : m_bones)
		{
			Bone* pBone = bone->GetBone();
			if (pBone->GetName().find("wheel") != string::npos)
			{
				Vector3 vAxis = bone->GetAxis();
				Quaternion q;
				pBone->rot.used = true;
				int nFirstRotSize = (int)pBone->rot.times.size();

				// for wheels, add rotation animation. 
				q.FromAngleAxis(Radian(0.f), vAxis);
				pBone->rot.times.push_back(anim.timeStart);
				pBone->rot.data.push_back(q);

				float fRotSign = (vAxis.z > 0) ? 1.f : -1.f;
				if (!bMoveForward)
					fRotSign = -fRotSign;
				q.FromAngleAxis(Radian(3.14f*fRotSign), vAxis);
				pBone->rot.times.push_back(anim.timeStart + nAnimLength / 2);
				pBone->rot.data.push_back(q);

				q.FromAngleAxis(Radian(6.28f*fRotSign), vAxis);
				pBone->rot.times.push_back(anim.timeStart + nAnimLength);
				pBone->rot.data.push_back(q);

				pBone->rot.ranges.resize(animIndex + 1, AnimRange(0, 0));
				pBone->rot.ranges[animIndex] = AnimRange(nFirstRotSize, max(nFirstRotSize, (int)pBone->rot.times.size() - 1));
			}
		}
		m_anims.push_back(anim);
	}

	CParaXModel* BMaxParser::CreateGetRefModel(const std::string& sFilename)
	{
		auto it = m_refModels.find(sFilename);
		if (it != m_refModels.end())
		{
			return it->second.get();
		}
		std::string sExt = CParaFile::GetFileExtension(sFilename);
		if (sExt == "bmax")
		{
			ref_ptr<CParaXModel> pModel;

			std::string sFullFilename = CGlobals::GetWorldInfo()->GetWorldDirectory() + sFilename;
			CParaFile file;
			if (file.OpenFile(sFullFilename.c_str()))
			{
				BMaxParser parser(sFilename.c_str(), this);
				parser.Load(file.getBuffer(), file.getSize());
				pModel.reset(parser.ParseParaXModel());
			}
			else
			{
				OUTPUT_LOG("warn: can not find referenced bmax file %s \n", sFilename.c_str());
			}

			m_refModels[sFilename] = pModel;
			return pModel.get();
		}
		if (sExt == "x")
		{
			ref_ptr<CParaXModel> pModel;

			std::string sFullFilename = CGlobals::GetWorldInfo()->GetWorldDirectory() + sFilename;
			CParaFile file;
			if (file.OpenFile(sFullFilename.c_str()))
			{
				CParaXSerializer serializer;
				serializer.SetFilename(sFullFilename);
				pModel.reset((CParaXModel*)serializer.LoadParaXMesh(file));
			}
			else
			{
				OUTPUT_LOG("warn: can not find referenced x file %s \n", sFilename.c_str());
			}

			m_refModels[sFilename] = pModel;
			return pModel.get();
		}
		return NULL;
	}

	void BMaxParser::SetMergeCoplanerBlockFace(bool val)
	{
		m_bMergeCoplanerBlockFace = val;
	}


}

