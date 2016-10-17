//-----------------------------------------------------------------------------
// Class: CParaXStaticModel
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2014.10.3
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SortedFaceGroups.h"
#include "XFileHelper.h"
#include "FBXParser.h"
#include "XFileStaticModelParser.h"
#include "SceneObject.h"

#include "ParaXStaticModel.h"

using namespace ParaEngine;

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParaXStaticModel::CParaXStaticModel(const char*  strName, XFileParser*	pDXFileParser)
	:m_pFileData(NULL), m_pFileDataSize(0)
{
	if (strName != 0)
		m_strName = strName;

	m_dwNumMaterials = 0L;
	m_bUseMaterials = true;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParaXStaticModel::~CParaXStaticModel()
{
	Destroy();
}

HRESULT CParaXStaticModel::Create(RenderDevicePtr pd3dDevice, const char* strFilename, bool bCheckSecondUV)
{
	// Initialize textures from texture file
	if (strFilename != NULL)
		m_strName = strFilename;

	if (m_pFileData)
	{
		return Create(pd3dDevice, (char*)m_pFileData, m_pFileDataSize, bCheckSecondUV);
	}
	else
	{
		CParaFile myFile(m_strName.c_str());
		char* buffer = myFile.getBuffer();
		DWORD nFileSize = (DWORD)myFile.getSize();
		if (buffer == 0)
			return E_FAIL;
		return Create(pd3dDevice, buffer, nFileSize, bCheckSecondUV);
	}
}

HRESULT CParaXStaticModel::Create(RenderDevicePtr pd3dDevice, char* buffer, DWORD nFileSize, bool bCheckSecondUV)
{
	try
	{
		auto nFileType = GetFileTypeByFilename(m_strName);
		if (nFileType == FileType_ParaX)
		{
			XFileStaticModelParser parser(buffer, nFileSize);
			auto pSysMemMesh = parser.ParseParaXStaticModel();
			return Create(pd3dDevice, pSysMemMesh);
		}
#ifdef SUPPORT_FBX_MODEL_FILE
		else if (nFileType == FileType_FBX)
		{
			FBXParser parser(m_strName);
			auto pSysMemMesh = parser.ParseFBXFile();
			return Create(pd3dDevice, pSysMemMesh);
		}
#endif
		else
		{
			OUTPUT_LOG("error: unsupported file format : %s \n", m_strName.c_str());
			return E_FAIL;
		}
	}
	catch (...)
	{
		OUTPUT_LOG("error: failed to create CParaXStaticModel: %s \n", m_strName.c_str());
		return E_FAIL;
	}
}

HRESULT CParaXStaticModel::Create(RenderDevicePtr pd3dDevice, XFile::Scene* pFileData)
{
	if (!pFileData)
		return E_FAIL;
	LoadToSystemBuffer(pFileData);
	SAFE_DELETE(pFileData);

	BuildRenderBuffer();
	return S_OK;
}

void ParaEngine::CParaXStaticModel::RendererRecreated()
{
#ifdef USE_OPENGL_RENDERER
	m_vertexBuffer.RendererRecreated();
	m_indexBuffer.RendererRecreated();
	BuildRenderBuffer();
#endif
}

HRESULT ParaEngine::CParaXStaticModel::ClonePhysicsMesh(DWORD* pNumVertices, Vector3 ** ppVerts, DWORD* pNumTriangles, WORD** ppIndices, int* pnMeshPhysicsGroup /*= NULL*/, int* pnTotalMeshGroupCount /*= NULL*/)
{
	if (m_vertices.empty() || m_indices.empty())
		return E_FAIL;

	if (pnTotalMeshGroupCount)
	{
		int nTotalMeshGroupCount = 0;
		int nPhysicsGroup = -1;
		while ((nPhysicsGroup = GetNextPhysicsGroupID(nPhysicsGroup)) >= 0)
		{
			nTotalMeshGroupCount++;
		}
		*pnTotalMeshGroupCount = nTotalMeshGroupCount;
	}
	
	DWORD dwNumFaces = 0;
	int nVertexCount = 0;
	uint32 nNumMaterials = (uint32)m_passes.size();
	for (uint32 i = 0; i < nNumMaterials; i++)
	{
		auto& pass = m_passes[i];
		CParaXMaterial* pMaterial = &(pass.m_material);

		if (pMaterial->hasPhysics() && (pnMeshPhysicsGroup == 0 || ((*pnMeshPhysicsGroup) == pMaterial->GetPhysicsGroup())))
		{
			dwNumFaces += pass.indexCount / 3;
		}
	}

	if (dwNumFaces == 0)
	{
		// in case, there is no physics faces in the mesh, return immediately.
		if (pNumVertices != 0)
			*pNumVertices = 0;
		if (pNumTriangles != 0)
			*pNumTriangles = 0;
		return S_OK;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// read the vertex buffer
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumVx = 0;
	Vector3 * verts = NULL;
	if (ppVerts != NULL)
	{
		dwNumVx = (int)m_vertices.size();
		verts = new Vector3[dwNumVx];
		auto pVertices = &(m_vertices[0]);

		for (DWORD i = 0; i < dwNumVx; ++i)
		{
			verts[i] = pVertices->p;
			pVertices++;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// read the index buffer
	//////////////////////////////////////////////////////////////////////////
	WORD* indices = NULL;
	if (ppIndices)
	{
		indices = new WORD[dwNumFaces * 3];
		int nD = 0; // destination indices index

		for (uint32 i = 0; i < nNumMaterials; i++)
		{
			auto& pass = m_passes[i];
			CParaXMaterial* pMaterial = &(pass.m_material);

			if (pMaterial->hasPhysics() && (pnMeshPhysicsGroup == 0 || ((*pnMeshPhysicsGroup) == pMaterial->GetPhysicsGroup())))
			{
#ifdef INVERT_PHYSICS_FACE_WINDING
				int16* dest = (int16*)&(indices[nD]);
				int16* src = &(m_indices[pass.indexStart]);
				int nFaceCount = pass.indexCount/3;
				for (int i = 0; i < nFaceCount; ++i)
				{
					// change the triangle winding order
					*dest = *src; ++src;
					*(dest + 2) = *src; ++src;
					*(dest + 1) = *src; ++src;
					dest += 3;
				}
#else
				memcpy(&(indices[nD]), &(m_indices[pass.indexStart]), sizeof(WORD)*pass.indexCount);
#endif
				nD += pass.indexCount;
			}
		}
	}
	// output result
	if (pNumVertices != 0){
		*pNumVertices = dwNumVx;
	}
	if (ppVerts != 0){
		*ppVerts = verts;
	}
	if (pNumTriangles != 0){
		*pNumTriangles = dwNumFaces;
	}
	if (ppIndices != 0){
		*ppIndices = indices;
	}
	return S_OK;
}

HRESULT CParaXStaticModel::LoadToSystemBuffer(XFile::Scene* pFileData)
{
	if (!pFileData)
		return E_FAIL;
	m_header.m_bIsValid = true;
	m_header.m_vMin = pFileData->m_header.minExtent;
	m_header.m_vMax = pFileData->m_header.maxExtent;
	m_header.m_bHasTex2 = false;
	m_header.m_bHasNormal = true;

	if (pFileData->m_XRefObjects.size() > 0)
		m_XRefObjects = pFileData->m_XRefObjects;

	m_vertices.clear();
	m_indices.clear();
	m_passes.clear();

	using namespace XFile;

	// true if the mesh contains at least one translucent face group. 
	bool bNeedSecondPass = false;

	int nRenderPass = 0;

	int nVertexCount = 0;
	int nIndexCount = 0;
	for (Mesh* mesh : pFileData->mGlobalMeshes)
	{
		nVertexCount += (int)mesh->mPositions.size();
		nIndexCount += (int)mesh->mPosFaces.size() * 3;
	}
	if (nIndexCount >= 0xffff)
	{
		OUTPUT_LOG("error: mesh index count exceeding 65535 is currently not supported\n");
		return E_FAIL;
	}
	else if (nIndexCount == 0)
	{
		// empty model
		return S_OK;
	}
	// prepare vertex and index buffer
	m_vertices.resize(nVertexCount);
	mesh_vertex_normal* pVert = !m_vertices.empty() ? &(m_vertices[0]) : nullptr;
	m_indices.resize(nIndexCount);
	int16 *pIndice = !m_indices.empty() ? &(m_indices[0]) : nullptr;
	
	mesh_vertex_normal* pCurVert = pVert;
	int16 *pCurIndice = pIndice;
	for (Mesh* mesh : pFileData->mGlobalMeshes)
	{
		int nCount = (int)mesh->mPositions.size();
		PE_ASSERT(nCount == mesh->mNormals.size());
		PE_ASSERT(nCount == mesh->mTexCoords[0].size());
		// prepare vertex for the sub mesh
		for (int i = 0; i < nCount; ++i)
		{
			mesh_vertex_normal& vert = (*pCurVert);
			pCurVert++;
			vert.p = mesh->mPositions[i];
			vert.n = mesh->mNormals[i];
			vert.uv = mesh->mTexCoords[0][i];
		}
		// prepare indices for the sub mesh
		nCount = (int)mesh->mPosFaces.size();
		if (nCount>0)
			memcpy(pCurIndice, &(mesh->mPosFaces[0]), sizeof(Face)*nCount);
		pCurIndice += nCount;
	}

	int32 nStartIndex = 0;
	// prepare all render passes(materials)
	for (Mesh* mesh : pFileData->mGlobalMeshes)
	{
		int nTotalFaceCount = (int)mesh->mFaceMaterials.size();
		if (nTotalFaceCount == 0)
			continue;
		int32 nMaterialCount = (int32)mesh->mMaterials.size();
		m_passes.resize(m_passes.size() + nMaterialCount);
		
		int nMaterialIndex = mesh->mFaceMaterials[0];
		for (int i = 0; i < nMaterialCount; ++i)
		{
			Material& material = mesh->mMaterials[nMaterialIndex];

			int nFaceCount = 0;
			{ // calculate face count in the material group
				for (int nStartFaceIndex=0; nStartFaceIndex < nTotalFaceCount; ++nStartFaceIndex)
				{
					if (mesh->mFaceMaterials[nStartFaceIndex] == nMaterialIndex)
						++nFaceCount;
					else
					{
						nMaterialIndex = mesh->mFaceMaterials[nStartFaceIndex];
						break;
					}
				}
			}
			
			ParaXStaticModelRenderPass& pass = m_passes[nRenderPass++];
			pass.indexStart = (uint16)(nStartIndex);
			pass.indexCount = nFaceCount * 3;
			nStartIndex = pass.indexStart + pass.indexCount;

			// copy materials
			ParaMaterial mat;
			mat.Diffuse = material.mDiffuse;
			mat.Ambient = LinearColor::Black;
			mat.Emissive = material.mEmissive;
			mat.Specular = material.mSpecular;
			mat.Power = material.mSpecularExponent;
			pass.m_material.SetMaterial(mat);

			// auto enable translucent sorting for z mast and alpha blending surfaces. 
			if (pass.m_material.hasAlphaBlending() && pass.m_material.hasZMask() && !(pass.m_material.hasAlphaTesting()))
				pass.m_material.EnableAttribute(CParaXMaterial::MATERIAL_TRANSLUCENT_SORT);

			if (pass.m_material.hasTranslucentSort() || pass.m_material.hasReflectionMap() || pass.m_material.hasAnyBillboard())
			{
				bNeedSecondPass = true;
			}

			if (!material.mTextures.empty())
			{
				// Initialize textures from texture file
				const std::string& packedFileNames = material.mTextures[0].mName;
				std::string texFile1, texFile2;
				size_t pos = packedFileNames.find(';');
				if (pos != string::npos) {
					texFile1.assign(packedFileNames.c_str(), pos);
					texFile2.assign(packedFileNames.c_str(), pos + 1, packedFileNames.size() - pos - 1);
				}
				else
					texFile1 = packedFileNames;
				int nFileSize = (int)texFile1.size();
				if (nFileSize > 10 && texFile1[nFileSize - 8] == '_' && texFile1[nFileSize - 7] == 'p' && texFile1[nFileSize - 6] == 'g' && texFile1[nFileSize - 4] == '.')
				{
					int nPhysicsGroup = (texFile1[nFileSize - 5] - '0');
					if (nPhysicsGroup >= 0 && nPhysicsGroup <= 9)
					{
						pass.m_material.m_nPhysicsGroup = nPhysicsGroup;
						pass.m_material.EnableAttribute(CParaXMaterial::MATERIAL_SKIP_RENDER, true);
					}
				}
				pass.m_material.m_pTexture1 = LoadTextureByPath(texFile1, TextureEntity::StaticTexture);

				// check if there is any replaceable texture ID in texFile2, texFile2 will be "[0-9]*"
				// generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
				if (!texFile2.empty() && texFile2[0] >= '0' && texFile2[0] <= '9')
				{
					try
					{
						int nID = -1;
						nID = atoi(texFile2.c_str());
						assert(nID < 32);
						pass.m_material.m_nReplaceableTextureID = nID;
						if (nID >= (int)m_ReplaceableTextures.size())
						{
							m_ReplaceableTextures.resize(nID + 1);
						}
						// use the default replaceable texture,which is the main texture stored at the material. 
						SetReplaceableTexture(nID, pass.m_material.m_pTexture1.get());
					}
					catch (...)
					{
					}
				}

				if (pass.m_material.hasReflectionMap())
				{
					// disable lightmap, if reflection map is enabled; 
					pass.m_material.EnableAttribute(CParaXMaterial::MATERIAL_LIGHTMAP, false);
				}

				if (pass.m_material.hasNormalMap())
				{
					// TODO: untested norm map
					string sFilePath = texFile1;
					int nCount = (int)sFilePath.size();
					if (nCount > 5)
					{
						// add Norm after the main texture file to get the normal map. 
						sFilePath.insert(sFilePath.size() - 4, "Norm");
						pass.m_material.m_pTexture2 = LoadTextureByPath(sFilePath, TextureEntity::StaticTexture);
					}
				}
				else if (pass.m_material.hasLightMap())
				{
					// TODO: untested light map
					pass.m_material.m_pTexture2 = LoadTextureByPath(texFile2, TextureEntity::StaticTexture);
				}
				else if (pass.m_material.hasEnvironmentMap())
				{
					if (!texFile2.empty())
					{
						pass.m_material.m_pTexture2 = LoadTextureByPath(texFile2, TextureEntity::CubeTexture);
					}
					else
					{
						// if environment is not in the second texture, we will deduce its name from the first texture, 
						// this is mostly for backward compatibilities.
						string sFilePath = texFile1;
						int nCount = (int)sFilePath.size();
						if (nCount > 5)
						{
							// add Env after the main texture file to get the normal map. 
							sFilePath.insert(sFilePath.size() - 4, "Env");
							pass.m_material.m_pTexture2 = LoadTextureByPath(sFilePath, TextureEntity::CubeTexture);
						}
					}
				}
			}
			else
			{
				// material without texture should skip rendering. 
				pass.m_material.EnableAttribute(CParaXMaterial::MATERIAL_SKIP_RENDER);
			}
		}
	}
	uint32 nNumMaterials = (uint32)m_passes.size();
	if (bNeedSecondPass)
	{
		// build face group.
		ClearFaceGroups();

		for (uint32 i = 0; i < nNumMaterials; i++)
		{
			auto& pass = m_passes[i];

			int nTriangleCount = pass.indexCount / 3;
			int nIndexStart = pass.indexStart;
			if (nTriangleCount <= 0)
				continue;
			if (pass.m_material.hasTranslucentSort())
			{
				CFaceGroup * pFaceGroup = new CFaceGroup();
				m_faceGroups.push_back(pFaceGroup);

				pass.m_material.m_nTranslucentFaceGroupIndex = (int)(m_faceGroups.size() - 1);

				// copy data from mesh and material to face group.
				pFaceGroup->m_pTexture = pass.m_material.m_pTexture1.get();
				pFaceGroup->m_alphaBlending = pass.m_material.hasAlphaBlending();
				pFaceGroup->m_alphaTesting = pass.m_material.hasAlphaTesting();
				pFaceGroup->m_bHasLighting = pass.m_material.hasLighting();
				pFaceGroup->m_disableZWrite = pass.m_material.hasZMask();
				pFaceGroup->m_bAdditive = pass.m_material.hasAdditive();
				pFaceGroup->m_material = pass.m_material.mat;

				// this gives z-write enabled face a high priority to be rendered higher.
				if (!pFaceGroup->m_disableZWrite)
					pFaceGroup->m_order = 0;
				else
					pFaceGroup->m_order = 1;

				pFaceGroup->m_nNumTriangles = nTriangleCount;

				for (int k = 0; k < nTriangleCount; ++k)
				{
					for (int j = 0; j < 3; j++)
					{
						const mesh_vertex_normal& v = pVert[pIndice[nIndexStart + k * 3 + j]];
						pFaceGroup->m_vertices.push_back(v.p);
						pFaceGroup->m_normals.push_back(v.n);
						pFaceGroup->m_UVs.push_back(v.uv);
					}
				}
				pFaceGroup->UpdateCenterPos();
			}
			if (pass.m_material.hasReflectionMap())
			{
				if (nTriangleCount >= 1)
				{
					int k = 0; // use the first triangle height for the reflection map height.
					float fHeight = 0.f;
					for (int j = 0; j < 3; j++)
					{
						const mesh_vertex_normal& v = pVert[pIndice[nIndexStart + k * 3 + j]];
						fHeight += v.p.y;
					}
					fHeight /= 3.f;
					pass.m_material.m_fRelectionHeight = fHeight;
				}
			}
			if (pass.m_material.hasAnyBillboard())
			{
				// get the pivot point, which is the center of the sub mesh
				Vector3 vPivot(0, 0, 0);
				for (int k = 0; k < nTriangleCount; ++k)
				{
					for (int j = 0; j < 3; j++)
					{
						const mesh_vertex_normal& v = pVert[pIndice[nIndexStart + k * 3 + j]];
						vPivot += v.p;
					}
				}
				pass.m_material.m_vPivot = vPivot / (nTriangleCount*3.f);
			}
		}
	}

	return S_OK;
}

void ParaEngine::CParaXStaticModel::BuildRenderBuffer()
{
	m_vertexBuffer.ReleaseBuffer();
	m_indexBuffer.ReleaseBuffer();
	
	int nVertexCount = (int)m_vertices.size();
	if (nVertexCount > 0)
	{
		mesh_vertex_normal* pVert = &(m_vertices[0]);
		m_vertexBuffer.CreateBuffer(sizeof(mesh_vertex_normal)*nVertexCount);
		mesh_vertex_normal* pData = NULL;
		if (m_vertexBuffer.Lock((void**)&pData))
		{
			memcpy(pData, pVert, sizeof(mesh_vertex_normal)*nVertexCount);
			m_vertexBuffer.Unlock();
		}
	}

	int nIndexCount = (int)m_indices.size();
	if (nIndexCount > 0)
	{
		int16 *pIndice = &(m_indices[0]);
		m_indexBuffer.CreateBuffer(sizeof(uint16)*nIndexCount);
		uint16* pIndexData = NULL;
		if (m_indexBuffer.Lock((void**)&pIndexData))
		{
			memcpy(pIndexData, pIndice, sizeof(uint16)*nIndexCount);
			m_indexBuffer.Unlock();
		}
	}
}

TextureEntity*	CParaXStaticModel::LoadTextureByPath(const string& sFilePath, DWORD TextureType)
{
	TextureEntity* pTex = NULL;
	// Initialize textures from texture file
	if (!CParaFile::IsFileName(sFilePath))
		pTex = CGlobals::GetAssetManager()->LoadTexture("", sFilePath, (TextureEntity::_SurfaceType)TextureType);
	else
	{
		string sNewTexPath = CParaFile::GetAbsolutePath(sFilePath, CParaFile::GetParentDirectoryFromPath(m_strName));
		pTex = CGlobals::GetAssetManager()->LoadTexture("", sNewTexPath, (TextureEntity::_SurfaceType)TextureType);
	}
	return pTex;
}

HRESULT CParaXStaticModel::InitDeviceObjects()
{
	return S_OK;
}

HRESULT CParaXStaticModel::DeleteDeviceObjects()
{
	return S_OK;
}


HRESULT CParaXStaticModel::Destroy()
{
	DeleteDeviceObjects();

	m_ReplaceableTextures.clear();

	m_passes.clear();

	m_dwNumMaterials = 0L;

	ClearFaceGroups();

	DeleteMeshFileData();

	m_vertices.clear();
	m_vertices.shrink_to_fit();
	m_indices.clear();
	m_indices.shrink_to_fit();
	return S_OK;
}

void CParaXStaticModel::ClearFaceGroups()
{
	// delete all face group. 
	for (int i = 0; i < (int)m_faceGroups.size(); ++i)
	{
		SAFE_DELETE(m_faceGroups[i]);
	}
	m_faceGroups.clear();
}


void ParaEngine::CParaXStaticModel::DrawRenderPass(int i)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	ParaXStaticModelRenderPass& pass = m_passes[i];
	RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST, 0, 0, 0, pass.indexStart, pass.indexCount / 3);
}

XModelFileType ParaEngine::CParaXStaticModel::GetFileTypeByFilename(const::string& sFilename)
{
	XModelFileType nFileType = FileType_ParaX; 
	int nSize = (int)sFilename.size();
	if (nSize > 4 && sFilename[nSize - 4] == '.' && sFilename[nSize - 3] == 'f' && sFilename[nSize - 2] == 'b' && sFilename[nSize - 1] == 'x')
	{
		nFileType = FileType_FBX;
	}	
	return nFileType;
}


HRESULT CParaXStaticModel::Render(SceneState * pSceneState, CEffectFile *pEffect, bool bDrawOpaqueSubsets, bool bDrawAlphaSubsets, float fAlphaFactor, CParameterBlock* materialParams)
{
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	pd3dDevice->SetStreamSource(0, m_vertexBuffer.GetDevicePointer(), 0, sizeof(mesh_vertex_normal));
	pd3dDevice->SetIndices(m_indexBuffer.GetDevicePointer());

	bool bHasAlphaPass = false;
	int cPasses = pEffect->totalPasses();
	bool bl = false, at = false, bDisableBackFacing = false, zm = false;
	bool bEnvMap = false, bReflectionMap = false, bNormalMap = false, bHasLightMap = false, bHasLighting = true;
	bool bAdditive = false;
	// first, draw the subsets without alpha
	if (bDrawOpaqueSubsets)
	{
		if (pEffect->begin())
		{
			for (int p = 0; p < cPasses; ++p)
			{
				pEffect->BeginPass(p);

				for (int i = 0; i < (int)m_passes.size(); i++)
				{
					CParaXMaterial* pMaterial = &(m_passes[i].m_material);

					if (pMaterial->isSkipRendering())
						continue;
					// skip translucent face group.
					if (pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex >= 0)
					{
						if (!pSceneState->IsIgnoreTransparent())
						{
							// skip shadow pass
							if (!CGlobals::GetEffectManager()->IsUsingShadowMap())
							{
								if (pMaterial->hasAnyBillboard())
								{
									Matrix4 mtrans;
									Math::CreateBillboardMatrix(&mtrans, NULL, &(pMaterial->m_vPivot), pMaterial->hasBillboardAA());
									mtrans = mtrans * CGlobals::GetWorldMatrixStack().SafeGetTop();
									pSceneState->GetFaceGroups()->AddFaceGroup(CFaceGroupInstance(&mtrans, m_faceGroups[pMaterial->m_nTranslucentFaceGroupIndex]));
								}
								else
								{
									pSceneState->GetFaceGroups()->AddFaceGroup(CFaceGroupInstance(&(CGlobals::GetWorldMatrixStack().top()), m_faceGroups[pMaterial->m_nTranslucentFaceGroupIndex]));
								}
							}
						}

						continue;
					}

					TextureEntity* pTexture = 0;
					if (m_bUseMaterials)
					{
						if (pMaterial->mat.Diffuse.a < 1.0f){
							bHasAlphaPass = true;
							continue;
						}
						if (pMaterial->m_pTexture1 != 0)
						{
							if (pMaterial->m_nReplaceableTextureID < 0)
							{
								pTexture = pMaterial->m_pTexture1.get();
							}
							else
							{
								TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
								if (pTextureEntity)
								{
									pTexture = pTextureEntity;
								}
								if (pTexture == 0)
								{
									// use default one if user defined is nil
									pTexture = pMaterial->m_pTexture1.get();
								}
							}
						}

						if (pTexture && pTexture->GetTexture())
						{
							pEffect->applySurfaceMaterial(&pMaterial->mat);

							//////////////////////////////////////////////////////////////////////////
							// environment mapping
							if (bEnvMap != pMaterial->hasEnvironmentMap())
							{
								if (pEffect->EnableEnvironmentMapping(!bEnvMap))
									bEnvMap = !bEnvMap;
							}
							if (bEnvMap)
							{
								if (pMaterial->m_pTexture2 != 0)
								{
									pEffect->SetReflectFactor(pMaterial->GetReflectivity());
									pEffect->setTexture(2, pMaterial->m_pTexture2.get()); // sampler assumed to be on register s2.
								}
							}

							//////////////////////////////////////////////////////////////////////////
							// normal mapping
							if (bNormalMap != pMaterial->hasNormalMap())
							{
								// TODO: untested normal map
								bNormalMap = !bNormalMap;
							}
							if (bNormalMap)
							{
								if (pMaterial->m_pTexture2 != 0)
									pEffect->setTexture(1, pMaterial->m_pTexture2.get());
							}

							//////////////////////////////////////////////////////////////////////////
							// light mapping
							if (bHasLightMap != pMaterial->hasLightMap())
							{
								// TODO: untested normal map
								bHasLightMap = !bHasLightMap;
							}
							if (bHasLightMap)
							{
								if (pMaterial->m_pTexture2 != 0)
								{
									// the reflectivity parameter is used for the light map scale factor. 
									float fLightMapScale = pMaterial->GetReflectivity();
									if (fLightMapScale < 0.99f)
										fLightMapScale *= 10.f;
									pEffect->SetReflectFactor(fLightMapScale);
									pEffect->setTexture(1, pMaterial->m_pTexture2.get());
								}
							}

							//////////////////////////////////////////////////////////////////////////
							// reflection mapping
							if (bReflectionMap != pMaterial->hasReflectionMap())
							{
								// prevent drawing itself?
								//if(!bReflectionMap && !CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
								//	continue;
								if (pEffect->EnableReflectionMapping(!bReflectionMap, pMaterial->m_fRelectionHeight))
									bReflectionMap = !bReflectionMap;

							}
							if (bReflectionMap)
								pEffect->SetReflectFactor(pMaterial->GetReflectivity());

							//////////////////////////////////////////////////////////////////////////
							// lighting
							if (bHasLighting != pMaterial->hasLighting())
							{
								bHasLighting = !bHasLighting;
								pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled() && bHasLighting);
							}


							bool bUseAdditive = pMaterial->hasAdditive();
							/** turn on or off z write: z mask means disable z write */
							if (bUseAdditive && !bAdditive){
								if (pMaterial->hasAlphaBlending())
								{
									pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
									pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
								}
								else
								{
									pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
									pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
								}
								bAdditive = true;
							}
							else if (!bUseAdditive && bAdditive){
								pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
								bAdditive = false;
							}

							/// set blending and alpha testing
							bool blend = pMaterial->hasAlphaBlending() || bUseAdditive;

							if (!blend)
							{
								// enable blending if "g_opacity" is specified and smaller than 1.f
								if (materialParams != NULL)
								{
									CParameter* pParams = materialParams->GetParameter("g_opacity");
									if (pParams && ((float)(*pParams)) < 1.f)
										blend = true;
								}
							}


							bool atest = pMaterial->hasAlphaTesting(); // alpha testing is not dependents on alpha blending now.
							bool zmask = pMaterial->hasZMask();

							/** if either blending or alpha testing is enabled, we will disable back face culling.*/
							if ((blend || atest) && (!bDisableBackFacing))
							{
								CGlobals::GetEffectManager()->SetCullingMode(false);
								bDisableBackFacing = true;
							}
							else if ((!blend && !atest) && bDisableBackFacing)
							{
								CGlobals::GetEffectManager()->SetCullingMode(true);
								bDisableBackFacing = false;
							}

							/** turn on and off blending. */
							if (blend && !bl)
							{
								pEffect->EnableAlphaBlending(blend);
								bl = true;
							}
							else if (!blend && bl)
							{
								pEffect->EnableAlphaBlending(blend);
								bl = false;
							}

							/** turn on and off alpha testing. */
							if (atest && !at)
							{
								pEffect->EnableAlphaTesting(atest);
								at = true;
							}
							else if (!atest && at)
							{
								pEffect->EnableAlphaTesting(atest);
								at = false;
							}
							/** turn on or off z write: z mask means disable z write */
							if (zmask && !zm){
								CGlobals::GetEffectManager()->EnableZWrite(false);
								zm = true;
							}
							else if (!zmask && zm){
								CGlobals::GetEffectManager()->EnableZWrite(true);
								zm = false;
							}

							pEffect->setTexture(0, pTexture);
						}

						if (pMaterial->hasAnyBillboard())
						{
							Matrix4 mtrans;
							Math::CreateBillboardMatrix(&mtrans, NULL, &(pMaterial->m_vPivot));
							mtrans = mtrans * CGlobals::GetWorldMatrixStack().SafeGetTop();
							CGlobals::GetWorldMatrixStack().push(mtrans);
							pEffect->applyCameraMatrices();
						}

						pEffect->CommitChanges();
						DrawRenderPass(i);

						if (pMaterial->hasAnyBillboard())
						{
							CGlobals::GetWorldMatrixStack().pop();
							pEffect->applyCameraMatrices();
						}
					}
				}
				pEffect->EndPass();
			}
			pEffect->end();
		}

	}

	// Then, draw the subsets with alpha
	if (bHasAlphaPass &&  bDrawAlphaSubsets && m_bUseMaterials)
	{
		if (pEffect->begin())
		{
			for (int p = 0; p < cPasses; ++p)
			{
				pEffect->BeginPass(p);
				for (int i = 0; i < (int)m_passes.size(); i++)
				{
					CParaXMaterial* pMaterial = &(m_passes[i].m_material);
					if (pMaterial->isSkipRendering())
						continue;
					// skip translucent face group.
					if (pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex >= 0)
					{
						if (!pSceneState->IsIgnoreTransparent())
						{
							if (m_bUseMaterials && pMaterial->mat.Diffuse.a == 1.0f)
								continue;
							// skip shadow pass
							if (!CGlobals::GetEffectManager()->IsUsingShadowMap())
								pSceneState->GetFaceGroups()->AddFaceGroup(CFaceGroupInstance(&(CGlobals::GetWorldMatrixStack().top()), m_faceGroups[pMaterial->m_nTranslucentFaceGroupIndex]));
						}

						continue;
					}

					TextureEntity* pTexture = 0;
					// set materials
					if (m_bUseMaterials)
					{
						if (pMaterial->mat.Diffuse.a == 1.0f){
							continue;
						}
						if (pMaterial->m_pTexture1 != 0)
						{
							if (pMaterial->m_nReplaceableTextureID < 0)
							{
								pTexture = pMaterial->m_pTexture1.get();
							}
							else
							{
								TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
								if (pTextureEntity)
								{
									pTexture = pTextureEntity;
								}
								if (pTexture == 0)
								{
									// use default one if user defined is nil
									pTexture = pMaterial->m_pTexture1.get();
								}
							}
						}

						if (pTexture && pTexture->GetTexture())
						{
							pEffect->applySurfaceMaterial(&pMaterial->mat);
							pEffect->setTexture(0, pTexture);
							pEffect->CommitChanges();
							DrawRenderPass(i);
						}
					}
				}
				pEffect->EndPass();
			}
			pEffect->end();
		}
	}
	if (bl)
		pEffect->EnableAlphaBlending(false);
	if (at)
		pEffect->EnableAlphaTesting(false);
	if (zm)
		CGlobals::GetEffectManager()->EnableZWrite(true);

	if (bDisableBackFacing)
		CGlobals::GetEffectManager()->SetCullingMode(true);
	if (bEnvMap)
		pEffect->EnableEnvironmentMapping(false);
	if (bReflectionMap)
		pEffect->EnableReflectionMapping(false);
	if (bNormalMap)
		pEffect->EnableNormalMap(false);
	if (bHasLightMap)
		pEffect->EnableLightMap(false);
	if (!bHasLighting)
		pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled());

	if (bAdditive){
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	return S_OK;
}

FLOAT CParaXStaticModel::ComputeBoundingSphere(Vector3* vObjectCenter, FLOAT* fObjectRadius)
{
	return *fObjectRadius;
}

HRESULT CParaXStaticModel::ComputeBoundingBox(Vector3 *pMin, Vector3 *pMax)
{
	if (ParaComputeBoundingBox((Vector3*)(&m_vertices[0]),
		m_vertices.size(),
		sizeof(mesh_vertex_normal),
		pMin, pMax))
	{
		return S_OK;
	}
	return E_FAIL;
}

void CParaXStaticModel::SetMeshFileData(const char * pData, int nSize)
{
	DeleteMeshFileData();
	m_pFileData = (char*)pData;
	m_pFileDataSize = nSize;
}

void CParaXStaticModel::DeleteMeshFileData()
{
	SAFE_DELETE_ARRAY(m_pFileData);
	m_pFileDataSize = 0;
}


MeshHeader& CParaXStaticModel::GetMeshHeader(XFileParser* pFileParser)
{
	// Note: header is always loaded when unlock device. 
	return m_header;
}

bool CParaXStaticModel::GetMeshHeaderFromFile(CParaFile& myFile, XFileParser* pFileParser)
{
	return m_header.m_bIsValid;
}

int CParaXStaticModel::GetNextPhysicsGroupID(int nPhysicsGroup)
{
	int nNextID = -1;
	uint32 nNumMaterials = (uint32)m_passes.size();
	for (uint32 i = 0; i < nNumMaterials; i++)
	{
		auto& pass = m_passes[i];
		CParaXMaterial* pMaterial = &(pass.m_material);
		if (pMaterial->hasPhysics() && pMaterial->GetPhysicsGroup() > nPhysicsGroup)
		{
			if (nNextID > pMaterial->GetPhysicsGroup() || nNextID == -1)
			{
				nNextID = pMaterial->GetPhysicsGroup();
			}
		}
	}
	return nNextID;
}

int CParaXStaticModel::GetNumReplaceableTextures()
{
	return (int)m_ReplaceableTextures.size();
}


TextureEntity* CParaXStaticModel::GetDefaultReplaceableTexture(int ReplaceableTextureID)
{
	if (ReplaceableTextureID >= 0 && (int)m_ReplaceableTextures.size() > ReplaceableTextureID)
	{
		uint32 nNumMaterials = (uint32)m_passes.size();
		for (uint32 i = 0; i<nNumMaterials; i++)
		{
			auto& pass = m_passes[i];
			if (pass.m_material.m_nReplaceableTextureID == ReplaceableTextureID)
			{
				return pass.m_material.m_pTexture1.get();
			}
		}
	}
	return NULL;
}

TextureEntity* CParaXStaticModel::GetReplaceableTexture(int ReplaceableTextureID)
{
	if (ReplaceableTextureID >= 0 && (int)m_ReplaceableTextures.size()>ReplaceableTextureID)
		return	m_ReplaceableTextures[ReplaceableTextureID].get();
	else
		return NULL;
}

bool CParaXStaticModel::SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity)
{
	if ((int)m_ReplaceableTextures.size() > ReplaceableTextureID && ReplaceableTextureID >= 0)
	{
		m_ReplaceableTextures[ReplaceableTextureID] = pTextureEntity;
		return true;
	}
	else
	{
		return false;
	}
}

const ParaXRefObject* CParaXStaticModel::GetXRefObjectsByIndex(int nIndex)
{
	return (nIndex >= 0 && nIndex < GetXRefObjectsCount()) ? &(m_XRefObjects[nIndex]) : NULL;
}

int CParaXStaticModel::GetPolyCount()
{
	return 0;
}

int CParaXStaticModel::GetPhysicsCount()
{
	return 0;
}

const char* CParaXStaticModel::DumpTextureUsage()
{
	static string g_output;
	g_output.clear();
	char temp[200];
	int nCount = 0;
	uint32 nNumMaterials = (uint32)m_passes.size();
	for (uint32 i = 0; i < nNumMaterials; i++)
	{
		auto& pass = m_passes[i];
		CParaXMaterial* pMaterial = &(pass.m_material);
		if (pMaterial)
		{
			if (pMaterial->m_pTexture1.get() != 0)
			{
				nCount++;
				const TextureEntity::TextureInfo * pInfo = pMaterial->m_pTexture1->GetTextureInfo();
				if (pInfo)
				{
					snprintf(temp, 200, "%d*%d(%d)", pInfo->m_width, pInfo->m_height, pInfo->m_format);
					g_output.append(temp);
				}
				g_output.append(pMaterial->m_pTexture1->GetKey());
				g_output.append(";");
			}
		}
	}
	snprintf(temp, 200, "cnt:%d;", nCount);
	g_output = temp + g_output;

	return g_output.c_str();
}

int CParaXStaticModel::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	assert(pClass != NULL);
	pClass->AddField("TextureUsage", FieldType_String, NULL, (void*)DumpTextureUsage_s, NULL, NULL, bOverride);
	pClass->AddField("PolyCount", FieldType_Int, NULL, (void*)GetPolyCount_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsCount", FieldType_Int, NULL, (void*)GetPhysicsCount_s, NULL, NULL, bOverride);
	return S_OK;
}