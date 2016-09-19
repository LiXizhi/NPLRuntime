//-----------------------------------------------------------------------------
// Class:Block max model parser	
// Authors:	leio, LiXizhi
// Emails:	lixizhi@yeah.net
// Date:	2015.6.29
// Desc: at most 256*256*256 blocks can be loaded
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
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

namespace ParaEngine
{
	const int BMaxParser::MaxBoneLengthHorizontal = 50;
	const int BMaxParser::MaxBoneLengthVertical = 100;

	BMaxParser::BMaxParser(const char* pBuffer, int32 nSize, const char* filename, BMaxParser* pParent)
		: m_bAutoScale(true), m_nHelperBlockId(90), m_pAnimGenerator(NULL), m_pParent(pParent),
		m_bHasAnimation(false), m_centerPos(0, 0, 0), m_fScale(1.f)
	{
		if (filename)
			SetFilename(filename);
		Load(pBuffer, nSize);
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
		BMaxXMLDocument doc;
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
				ParseVisibleBlocks();
				if (m_bAutoScale)
					ScaleModels();
			}
		}
	}

	void BMaxParser::ParseBlocks_Internal(const char* value)
	{
		auto pBlockWorld = BlockWorldClient::GetInstance();
		vector<BMaxNodePtr> nodes;
		CShapeBox aabb;
		NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(value);
		bool bHasBoneBlock = false;
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
									node->SetFacing((float)((double)attr["facing"]));
									nodes.push_back(BMaxNodePtr(node.get()));
								}
							}
						}
					}
				}
				else if (template_id == BoneBlockId)
				{
					bHasBoneBlock = true;
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
									if (nCount > 0){
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
				else if (!pBlockTemplate->isSolidBlock())
				{
					// other non-solid blocks will not be rendered. but can be used to connect bones
					template_id = TransparentBlockId;
					BMaxBlockModelNodePtr node(new BMaxBlockModelNode(this, x, y, z, template_id, block_data));
					nodes.push_back(BMaxNodePtr(node.get()));
				}
				else
				{
					BMaxNodePtr node(new BMaxNode(this, x, y, z, template_id, block_data));
					nodes.push_back(node);
				}
			}
		}

		// set AABB and center
		m_blockAABB = aabb;
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
		}

		// set scaling;
		if (m_bAutoScale)
		{
			float fMaxLength = Math::Max(Math::Max(m_blockAABB.GetHeight(), m_blockAABB.GetWidth()), m_blockAABB.GetDepth()) + 1.f;
			m_fScale = CalculateScale(fMaxLength);
			if (bHasBoneBlock)
			{
				// for animated models, it is by default 1-2 blocks high, for static models, it is 0-1 block high. 
				m_fScale *= 2.f;
			}
		}
	}

	void BMaxParser::ParseVisibleBlocks()
	{
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
						m_blockModels.push_back(tessellatedModel);
						m_blockModelsMapping[tessellatedModel] = node;
					}
					else
					{
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

	int BMaxParser::GetIndexFromXmlElement(BMaxXMLElement* node, const char* name, int& x, int& y, int& z)
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

				int index = GetNodeIndex(x, y, z);
				return index;
			}
		}
		return -1;
	}

	BMaxNode* BMaxParser::GetBMaxNode(int x, int y, int z)
	{
		if (x >= 0 && y >= 0 && z >= 0 && x < 256 && y < 256 && z < 256)
			return GetNode((uint16)x, (uint16)y, (uint16)z);
		else
			return NULL;
	}

	int BMaxParser::InsertNode(BMaxNodePtr& nodePtr)
	{
		int index = nodePtr->GetIndex();
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
		CParaXModel* pMesh = NULL;
		ParaXHeaderDef m_xheader;
		pMesh = new CParaXModel(m_xheader);
		FillParaXModelData(pMesh);
		pMesh->SetBmaxModel();
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

	void BMaxParser::FillVerticesAndIndices()
	{
		if (m_blockModels.size() == 0)
		{
			return;
		}
		ModelGeoset* geoset = AddGeoset();
		ModelRenderPass* pass = AddRenderPass();
		pass->geoset = geoset->id;

		int32 nStartIndex = 0;

		int nRootBoneIndex = FindRootBoneIndex();
		CShapeAABB aabb;
		int total_count = 0;
		int nStartVertex = 0;
		for (uint32 i = 0; i < m_blockModels.size(); i++)
		{
			BlockModel* model = m_blockModels.at(i);
			BMaxNode* node = m_blockModelsMapping[model];
			if (!node)
				continue;

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
				geoset->SetVertexStart(total_count);
				nStartVertex = 0;
			}
			
			geoset->vstart += nVertices;
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
				pVertices->GetNormal(modelVertex.normal);
				modelVertex.color0 = pVertices->color2;
				//set bone and weight, only a single bone
				modelVertex.bones[0] = nBoneIndex;
				modelVertex.weights[0] = vertex_weight;
				
				m_vertices.push_back(modelVertex);
				aabb.Extend(modelVertex.pos);
			}

			for (int k = 0; k < nFace; k++)
			{
				int start_index = k * 4 + nStartVertex;
				m_indices.push_back(start_index + 0);
				m_indices.push_back(start_index + 1);
				m_indices.push_back(start_index + 2);
				m_indices.push_back(start_index + 0);
				m_indices.push_back(start_index + 2);
				m_indices.push_back(start_index + 3);
			}
			total_count += nVertices;
			nStartVertex += nVertices;
		}
		aabb.GetMin(m_minExtent);
		aabb.GetMax(m_maxExtent);
	}
	void BMaxParser::FillParaXModelData(CParaXModel *pMesh)
	{
		if (pMesh == NULL)
		{
			return;
		}
		FillVerticesAndIndices();
		pMesh->m_objNum.nVertices = m_vertices.size();
		pMesh->m_objNum.nBones = m_bones.size();
		pMesh->m_objNum.nAnimations = m_bones.size() > 0 ? m_anims.size() : 0;
		pMesh->m_objNum.nIndices = m_indices.size();
		pMesh->m_header.minExtent = m_minExtent;
		pMesh->m_header.maxExtent = m_maxExtent;

		if (m_vertices.size() == 0)
			return;
		pMesh->initVertices(m_vertices.size(), &(m_vertices[0]));
		pMesh->initIndices(m_indices.size(), &(m_indices[0]));
		pMesh->m_RenderMethod = CParaXModel::BMAX_MODEL;

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

		if (m_anims.size() > 0 && m_bones.size() > 0)
		{
			pMesh->anims = new ModelAnimation[m_anims.size()];
			memcpy(pMesh->anims, &(m_anims[0]), sizeof(ModelAnimation)*m_anims.size());
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
			memset(pMesh->showGeosets, true, pMesh->geosets.size()*sizeof(bool));
		}
	}

	int BMaxParser::GetBoneIndex(uint16 x, uint16 y, uint16 z)
	{
		int nBoneIndex = -1;
		BMaxNode* pBone = GetNode(x,y,z);
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
		int index = GetIndexFromXmlElement(node, "frame_id", x, y, z);
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
	}

	void BMaxParser::CalculateBoneWeights()
	{
		// pass 1: calculate all blocks directly connected to bone block and share the same bone color
		for (auto bone: m_bones)
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
		if (CParaFile::GetFileExtension(sFilename) == "bmax")
		{
			ref_ptr<CParaXModel> pModel;

			std::string sFullFilename = CGlobals::GetWorldInfo()->GetWorldDirectory() + sFilename;
			CParaFile file;
			if (file.OpenFile(sFullFilename.c_str()))
			{
				BMaxParser parser(file.getBuffer(), file.getSize(), sFilename.c_str(), this);
				pModel.reset(parser.ParseParaXModel());
			}
			else
			{
				OUTPUT_LOG("warn: can not find referenced bmax file %s \n", sFilename.c_str());
			}

			m_refModels[sFilename] = pModel;
			return pModel.get();
		}
		return NULL;
	}


}

