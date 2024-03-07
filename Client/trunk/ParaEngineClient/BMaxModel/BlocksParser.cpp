#include "ParaEngine.h"
#include "BlocksParser.h"
#include "ParaXSerializer.h"
#include "BlockEngine/BlockModel.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockTemplate.h"
#include "BlockEngine/BlockWorldClient.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/ParaXBone.h"
#include "NPLHelper.h"
#include "NPLTable.h"
#include "BMaxBlockModelNode.h"
#include "BMaxAnimGenerator.h"
#include "BMaxGlassModelNode.h"
#include "TextureEntity.h"

namespace ParaEngine
{
	BlocksParser::BlocksParser() : BMaxParser(nullptr, nullptr)
	{
	}

	BlocksParser::~BlocksParser(void)
	{
	}

	void BlocksParser::Load(const char* blocks)
	{
		ParseBlocks_Internal(blocks);
		MergeCoplanerBlockFace();
	}

	ParaEngine::CParaXModel* BlocksParser::ParseParaXModel()
	{
		ClearModel();
		m_textures.clear();
		CParaXModel* pMesh = NULL;
		ParaXHeaderDef m_xheader;
		pMesh = new CParaXModel(m_xheader);
		FillParaXModelData(pMesh);
		return pMesh;
	}

	void BlocksParser::ParseBlocks_Internal(const char* value)
	{
		auto pBlockWorld = BlockWorldClient::GetInstance();
		NPL::NPLObjectProxy msg = NPL::NPLHelper::StringToNPLTable(value);
		for (NPL::NPLTable::IndexIterator_Type itCur = msg.index_begin(); itCur != msg.index_end(); ++itCur)
		{
			NPL::NPLObjectProxy& block = itCur->second;
			int x = (int)((double)block[1]);
			int y = (int)((double)block[2]);
			int z = (int)((double)block[3]);
			int template_id = (int)((double)block[4]);
			int block_data = (int)((double)block[5]);

			auto pBlockTemplate = pBlockWorld->GetBlockTemplate(template_id);

			if (pBlockTemplate)
			{
				BMaxNodePtr node;
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
									BMaxBlockModelNodePtr blockNode(new BMaxBlockModelNode(this, x, y, z, template_id, block_data));
									blockNode->SetFilename(sFilename);
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

									if (bHasTransform)
										blockNode->SetTransform(matLocalTrans);

									node = BMaxNodePtr(blockNode.get());
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
					node = BMaxNodePtr(pFrameNode.get());

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
				else if (!pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_solid))
				{
					if (pBlockTemplate->GetBlockModelByData(block_data).IsUniformLighting())
					{
						node = BMaxNodePtr(new BMaxNode(this, x, y, z, template_id, block_data));
						node->setSolid(false);
					}
					else
					{
						//non-solid, non-obstruction, glass models
						BMaxGlassModelNodePtr glassNode(new BMaxGlassModelNode(this, x, y, z, template_id, block_data));
						node = BMaxNodePtr(glassNode.get());
						node->setSolid(false);
					}
				}
				else
				{
					if (!pBlockTemplate->IsMatchAttribute(BlockTemplate::batt_transparent))
					{
						// treat as standard cube
						node = BMaxNodePtr(new BMaxNode(this, x, y, z, template_id, block_data));
					}
					else
					{
						//solid, non-obstruction, leaves
						BMaxGlassModelNodePtr glassNode(new BMaxGlassModelNode(this, x, y, z, template_id, block_data));
						node = BMaxNodePtr(glassNode.get());
						node->setSolid(false);
					}
				}

				InsertNode(node);
			}
		}
	}

	void BlocksParser::ParseVisibleBlocks()
	{
		for (auto& item : m_nodes)
		{
			BMaxNode* node = item.second.get();
			if (node != NULL)
			{
				BlockModel* tessellatedModel = new BlockModel();
				if (node->isSolid())
				{
					int32_t uvPattern = 0;
					auto block_template = BlockWorldClient::GetInstance()->GetBlockTemplate(node->template_id);
					if (block_template->IsMatchAttribute(BlockTemplate::batt_threeSideTex))
						uvPattern = 3;
					else if (block_template->IsMatchAttribute(BlockTemplate::batt_fourSideTex))
						uvPattern = 4;
					else if (block_template->IsMatchAttribute(BlockTemplate::batt_sixSideTex))
						uvPattern = 6;
					tessellatedModel->LoadModelByTexture(uvPattern);
				}
				if (node->TessellateBlock(tessellatedModel) > 0)
				{
					node->SetBlockModel(tessellatedModel);
				}
				else
				{
					node->SetBlockModel(NULL);
					delete tessellatedModel;
				}
			}
		}
	}

	void BlocksParser::MergeCoplanerBlockFace()
	{
		ParseVisibleBlocks();

		blockRectangles.clear();
		for (auto& item : m_nodes)
		{
			BMaxNode* node = item.second.get();
			auto block_template = BlockWorldClient::GetInstance()->GetBlockTemplate(node->template_id);
			if (block_template != nullptr)
				blockTextures[node->template_id] = block_template->GetTexture0(node->block_data);
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
			else
			{
				blockNodes[node->template_id].push_back(node);
			}
		}
	}

	void BlocksParser::FindCoplanerFace(BMaxNode* node, uint32 nFaceIndex)
	{
		const uint16 nVertexCount = 4;

		BMaxNodePtr nodes[nVertexCount] =
		{
			BMaxNodePtr(node), BMaxNodePtr(node), BMaxNodePtr(node), BMaxNodePtr(node)
		};

		RectanglePtr rectangle(new Rectangle(nodes, nFaceIndex));
		for (uint32 i = 0; i < nVertexCount; i++)
		{
			//FindNeighbourFace(rectangle.get(), i, nFaceIndex);
			node->SetFaceUsed(nFaceIndex);
		}

		rectangle->CloneNodes();
		blockRectangles[node->template_id].push_back(rectangle);
	}

	void BlocksParser::FillParaXModelData(CParaXModel* pMesh, int32 nMaxTriangleCount /*= -1*/)
	{
		if (pMesh == NULL)
		{
			return;
		}
		FillVerticesAndIndices(nMaxTriangleCount);
		pMesh->m_objNum.nVertices = m_vertices.size();
		pMesh->m_objNum.nBones = m_bones.size();
		pMesh->m_objNum.nAnimations = m_bones.size() > 0 ? m_anims.size() : 0;
		pMesh->m_objNum.nIndices = m_indices.size();
		pMesh->m_objNum.nTextures = m_textures.size();
		pMesh->m_header.minExtent = m_minExtent;
		pMesh->m_header.maxExtent = m_maxExtent;

		if (m_vertices.size() == 0)
			return;

		pMesh->m_header.type = PARAX_MODEL_STATIC;
		pMesh->initVertices(m_vertices.size(), &(m_vertices[0]));
		pMesh->initIndices(m_indices.size(), &(m_indices[0]));
		pMesh->textures = new asset_ptr<TextureEntity>[pMesh->m_objNum.nTextures];
		for (uint32 i = 0; i < pMesh->m_objNum.nTextures; i++)
		{
			pMesh->textures[i] = m_textures[i];
		}

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
				memcpy(pMesh->anims, &(m_anims[0]), sizeof(ModelAnimation) * m_anims.size());
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

	void BlocksParser::FillVerticesAndIndices(int32 nMaxTriangleCount /*= -1*/)
	{
		int32 nStartIndex = 0;

		CShapeAABB aabb;
		int total_count = 0;
		int nStartVertex = 0;

		int nRootBoneIndex = 0;
		for (auto block_rectangles : blockRectangles)
		{
			nStartIndex = (int32)m_indices.size();
			ModelGeoset* geoset = AddGeoset();
			ModelRenderPass* pass = AddRenderPass();
			pass->geoset = geoset->id;
			pass->SetStartIndex(nStartIndex);
			pass->tex = m_textures.size();
			geoset->SetVertexStart(total_count);
			nStartVertex = 0;

			for (uint32 i = 0; i < block_rectangles.second.size(); i++)
			{
				Rectangle* rectangle = block_rectangles.second[i].get();
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
					pass->tex = m_textures.size();
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
					pVertices->GetTexcoord(modelVertex.texcoords.x, modelVertex.texcoords.y);

					modelVertex.color0 = pVertices->color2;
					modelVertex.color1 = 0xffffffff; // unused
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

			m_textures.push_back(blockTextures[block_rectangles.first]);
		}

		for (auto& block_nodes : blockNodes)
		{
			auto block_template = BlockWorldClient::GetInstance()->GetBlockTemplate(block_nodes.first);
			nStartIndex = (int32)m_indices.size();
			ModelGeoset* geoset = AddGeoset();
			ModelRenderPass* pass = AddRenderPass();
			pass->geoset = geoset->id;
			pass->tex = m_textures.size();
			pass->cull = block_template->IsMatchAttribute(BlockTemplate::batt_solid) ||
				!block_template->GetBlockModel().IsUniformLighting() ||
				!block_template->IsMatchAttribute(BlockTemplate::batt_transparent);
			pass->SetStartIndex(nStartIndex);
			geoset->SetVertexStart((int32)m_vertices.size());

			for (uint32_t i = 0; i < block_nodes.second.size(); i++)
			{
				BMaxNode* node = block_nodes.second[i];
				if (node->GetParaXModel() == 0)
				{
					BlockModel* model = node->GetBlockModel();
					if (model != 0)
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
							pass->tex = m_textures.size();
							pass->cull = block_template->IsMatchAttribute(BlockTemplate::batt_solid) ||
								!block_template->GetBlockModel().IsUniformLighting() ||
								!block_template->IsMatchAttribute(BlockTemplate::batt_transparent);
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
							pVertices->GetTexcoord(modelVertex.texcoords.x, modelVertex.texcoords.y);
							modelVertex.color0 = pVertices->color;
							modelVertex.color1 = pVertices->color2;
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

					ModelVertex* ov = pModel->m_origVertices;
					int nVertices = pModel->GetObjectNum().nVertices;
					for (int i = 0; i < nVertices; i++)
					{
						auto ov = pModel->m_origVertices + i;
						ModelVertex modelVertex;
						memset(&modelVertex, 0, sizeof(ModelVertex));

						if (m_bHasTransform)
						{
							modelVertex.pos = (ov->pos * matLocalTrans + vOffset);
							modelVertex.normal = (ov->normal * matLocalTrans);
						}
						else
						{
							modelVertex.pos = (ov->pos + vOffset);
							modelVertex.normal = (ov->normal);
						}

						modelVertex.pos *= m_fScale;
						modelVertex.texcoords = ov->texcoords;
						modelVertex.color0 = ov->color0;
						modelVertex.color1 = ov->color1;
						//set bone and weight, only a single bone
						modelVertex.bones[0] = nBoneIndex;
						modelVertex.weights[0] = vertex_weight;

						m_vertices.push_back(modelVertex);
						aabb.Extend(modelVertex.pos);
					}

					for (int nPass = 0; nPass < nPasses; nPass++)
					{
						ModelRenderPass& p = pModel->passes[nPass];
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
								pass->tex = m_textures.size();
								geoset->SetVertexStart(total_count);
							}

							geoset->icount += nIndexCount;
							pass->indexCount += nIndexCount;
							pass->tex = m_textures.size();

							int nVertexOffset = p.GetVertexStart(pModel) + total_count;

							for (int i = 0; i < p.indexCount; ++i)
							{
								int a = pModel->m_indices[nIndexOffset + i] + nVertexOffset;
								m_indices.push_back((uint16)a);
							}
						}
					}
				}
			}
			m_textures.push_back(blockTextures[block_nodes.first]);
		}

		aabb.GetMin(m_minExtent);
		aabb.GetMax(m_maxExtent);
	}

}

