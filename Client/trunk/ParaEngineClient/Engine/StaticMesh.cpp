//-----------------------------------------------------------------------------
// Class: CParaXStaticMesh
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
// Note: it is based on microsoft's D3DX common file 9.0c
//		textures are globally managed.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SortedFaceGroups.h"

#include <dxfile.h>
// for ParaEngine x file template registration
#include <rmxfguid.h>
#include "mdxfile/ParaXFileGUID.h"

#include "StaticMesh.h"
#include "SceneState.h"
#include "ShadowMap.h"
#include "SceneObject.h"
#include "ParaxSerializer.h"

#include "memdebug.h"
using namespace ParaEngine;

/** @def if defined, force alpha testing whenever alpha blending is enabled. For models exported by ParaEngine Exporter 2006.12.21 or later, this is not needed.*/
// #define FORCE_ALPHATEST_ON_BLENDING
/** @def whether to render faces whose material does not have textures */
//#define RENDER_NO_TEXTURE
/** @def whether to use DrawSubsetEx() function. */
#define USE_DRAW_SUBSET_EX

static vector<D3DXATTRIBUTERANGE> atts(100);

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParaXStaticMesh::CParaXStaticMesh( LPCSTR strName , LPD3DXFILE	pDXFileParser)
:m_pFileData(NULL), m_pFileDataSize(0)
{
	if(strName!=0)
		m_strName = strName;
	m_pDXFileParser = (pDXFileParser != 0) ? pDXFileParser : CGlobals::GetAssetManager()->GetParaXFileParser();
	
    m_pSysMemMesh        = NULL;
    m_pLocalMesh         = NULL;
    m_dwNumMaterials     = 0L;
    m_pMaterials         = NULL;
    m_bUseMaterials      = TRUE;
}

//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CParaXStaticMesh::~CParaXStaticMesh()
{
    Destroy();
}

HRESULT CParaXStaticMesh::Create( LPDIRECT3DDEVICE9 pd3dDevice, const char* strFilename, bool bCheckSecondUV)
{
	// Initialize textures from texture file
	if(strFilename != NULL)
		m_strName = strFilename;
	
	if(m_pFileData)
	{
		return Create( pd3dDevice, (char*)m_pFileData, m_pFileDataSize, bCheckSecondUV );
	}
	else
	{
		CParaFile myFile(m_strName.c_str());
		char* buffer = myFile.getBuffer();
		DWORD nFileSize = (DWORD)myFile.getSize();
		if(buffer == 0)
			return E_FAIL;
		return Create( pd3dDevice, buffer, nFileSize, bCheckSecondUV );
	}
}

HRESULT CParaXStaticMesh::Create( LPDIRECT3DDEVICE9 pd3dDevice, char* buffer, DWORD nFileSize, bool bCheckSecondUV )
{
	HRESULT      hr = S_OK;
	
	if(m_pDXFileParser != NULL )
	{
		D3DXF_FILELOADMEMORY memData;
		memData.dSize = (int)nFileSize;
		memData.lpMemory =  buffer;
		
		ID3DXFileEnumObject* pDXEnum = NULL;
		if(SUCCEEDED(m_pDXFileParser->CreateEnumObject(&memData, D3DXF_FILELOAD_FROMMEMORY, &pDXEnum)))
		{
			bool bLoaded = false;
			LPD3DXFILEDATA pData=NULL;
			SIZE_T nCount;
			pDXEnum->GetChildren(&nCount);
			for(int i = 0; i<(int)nCount;i++)
			{
				pDXEnum->GetChild(i, &pData);
				GUID Type;
				// Get the template type
				if(FAILED(pData->GetType(&Type)))
					break;
				
				if(Type == TID_ParaXRefSection)
				{
					// TODO: read some xref data
					SIZE_T       dwSize;
					const char       *pBuffer=NULL;
					// Get the template data
					if (SUCCEEDED(pData->Lock(&dwSize, (LPCVOID*)(&pBuffer))))
					{
						int nCount = *(DWORD*)(pBuffer);

						pBuffer += sizeof(DWORD);
						m_XRefObjects.resize(nCount, ParaXRefObject());

						for (int k=0;k<nCount;++k)
						{
							ParaXRefObject* pXRefObj = &(m_XRefObjects[k]);
							/*
							template ParaXRefObject {
							<10000000-1234-0000-0000-123456789000>
							string filename;
							DWORD type;
							Matrix4x4 localTransform;
							Vector origin;
							DWORD animID;
							DWORD color;
							DWORD reserved0;
							FLOAT reserved1;
							FLOAT reserved2;
							FLOAT reserved3;
							}
							*/
							// get file name
							pXRefObj->m_filename = (const char*)pBuffer;
							pBuffer+=(pXRefObj->m_filename.size()+1);
							memcpy( &(pXRefObj->m_data), pBuffer, sizeof(ParaXRefObject::ParaXRefObject_Data));
							pBuffer+=sizeof(ParaXRefObject::ParaXRefObject_Data);
						}
					}

					SAFE_RELEASE(pData);
				}
				else if(Type == TID_D3DRMMesh)
				{
					// Load mesh the normal way
					if(!bCheckSecondUV)
					{
						hr = Create(pd3dDevice, pData);
						SAFE_RELEASE(pData);
						bLoaded = true;
					}
					else
					{
						// use D3DXLoadMeshFromXInMemory to parse data. 
						// * 2008.1.25 by LXZ: this is a tricky workaround of D3DXLoadMeshFromXOf can only load a simple mesh from data node. If the mesh contains frames or second UV sets, the function will not work. So in such cases, I fall back to use the original D3DXLoadMeshFromXInMemory, which can handle frames and second UV sets.
					}
				}
				else if(Type == TID_D3DRMFrame)
				{
					// It appears that the D3DXLoadMeshFromXof function can only load Mesh Data without wrapping frames
					// Hence, we will use D3DXLoadMeshFromXInMemory instead
					// X file exported by ParaX exporter will not contain any wrapping frame node, Hence D3DXLoadMeshFromXInMemory is only used for some X models exported from other exporters.
				}
			}
			if(bLoaded)
				return hr;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// the old way provided by DirectX

	char        strPath[MAX_PATH];
	LPD3DXBUFFER pAdjacencyBuffer = NULL;
	LPD3DXBUFFER pMtrlBuffer = NULL;

    // Load the mesh
	if( FAILED( hr = D3DXLoadMeshFromXInMemory( buffer, nFileSize, D3DXMESH_SYSTEMMEM, pd3dDevice, 
                                        &pAdjacencyBuffer, &pMtrlBuffer, NULL,
                                        &m_dwNumMaterials, &m_pSysMemMesh ) ) )
    {
        return hr;
    }

    // Optimize the mesh for performance
    if( FAILED( hr = m_pSysMemMesh->OptimizeInplace(
                        D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
                        (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL ) ) )
    {
        SAFE_RELEASE( pAdjacencyBuffer );
        SAFE_RELEASE( pMtrlBuffer );
        return hr;
    }

    hr = CreateMaterials( strPath, pd3dDevice, pAdjacencyBuffer, pMtrlBuffer );

    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pMtrlBuffer );

    return hr;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::Create( LPDIRECT3DDEVICE9 pd3dDevice,
                          LPD3DXFILEDATA pFileData )
{
    LPD3DXBUFFER pMtrlBuffer = NULL;
    LPD3DXBUFFER pAdjacencyBuffer = NULL;
    HRESULT      hr;

    // Load the mesh from the DXFILEDATA object
    if( FAILED( hr = D3DXLoadMeshFromXof( pFileData, D3DXMESH_SYSTEMMEM, pd3dDevice,
                                          &pAdjacencyBuffer, &pMtrlBuffer, NULL,
                                          &m_dwNumMaterials, &m_pSysMemMesh ) ) )
    {
        return hr;
    }

    // Optimize the mesh for performance
    if( FAILED( hr = m_pSysMemMesh->OptimizeInplace(
                        D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE,
                        (DWORD*)pAdjacencyBuffer->GetBufferPointer(), NULL, NULL, NULL ) ) )
    {
        SAFE_RELEASE( pAdjacencyBuffer );
        SAFE_RELEASE( pMtrlBuffer );
        return hr;
    }

    hr = CreateMaterials( "", pd3dDevice, pAdjacencyBuffer, pMtrlBuffer );

    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pMtrlBuffer );

    return hr;
}


HRESULT CParaXStaticMesh::CreateMaterials( LPCSTR strPath, IDirect3DDevice9 *pd3dDevice, ID3DXBuffer *pAdjacencyBuffer, ID3DXBuffer *pMtrlBuffer )
{
	// we will only store used materials. 
	if(atts.size()<m_dwNumMaterials)
	{
		atts.resize(m_dwNumMaterials);
	}
	m_pSysMemMesh->GetAttributeTable(&(atts[0]), &m_dwNumMaterials);

	// true if the mesh contains at least one translucent face group. 
	bool bNeedSecondPass = false;

    // Get material info for the mesh
    // Get the array of materials out of the buffer
    if( pMtrlBuffer && m_dwNumMaterials > 0 )
    {
		bool  bIsMDXModel = (m_strName.size()>4 && (m_strName[3]=='/' && m_strName[0]=='W' && m_strName[1]=='O' && m_strName[2]=='W'));

        // Allocate memory for the materials and textures
        D3DXMATERIAL* d3dxMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer();
        m_pMaterials = new CParaXMaterial[m_dwNumMaterials];
        if( m_pMaterials == NULL )
            return E_OUTOFMEMORY;

        // Copy each material and create its texture
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
            // Copy the material
            m_pMaterials[i].SetMaterial(reinterpret_cast<const ParaMaterial&>(d3dxMtrls[atts[i].AttribId].MatD3D));

			if(!bIsMDXModel)
			{
				// auto enable translucent sorting for zmast and alpha blending surfaces. 
				if(m_pMaterials[i].hasAlphaBlending() && m_pMaterials[i].hasZMask() && !(m_pMaterials[i].hasAlphaTesting()))
					m_pMaterials[i].EnableAttribute(CParaXMaterial::MATERIAL_TRANSLUCENT_SORT);

				if(m_pMaterials[i].hasTranslucentSort() || m_pMaterials[i].hasReflectionMap() || m_pMaterials[i].hasAnyBillboard())
				{
					bNeedSecondPass = true;
				}
			}
			else
			{
				// turn off z-mask if it is MDX model. 
				if(m_pMaterials[i].hasZMask())
					m_pMaterials[i].dwAttribute &= ~CParaXMaterial::MATERIAL_ZMASKING;
			}
			
            // Create a texture
            if( d3dxMtrls[atts[i].AttribId].pTextureFilename )
            {
				// Initialize textures from texture file

				string packedFileNames = d3dxMtrls[atts[i].AttribId].pTextureFilename;
				string texFile1, texFile2;
				size_t pos=packedFileNames.find(';');
				if (pos!=string::npos) {
					texFile1.assign(packedFileNames.c_str(),pos);
					texFile2.assign(packedFileNames.c_str(),pos+1,packedFileNames.size()-pos-1);
				}
				else
					texFile1=packedFileNames;
				int nFileSize = (int)texFile1.size();
				if (nFileSize > 10 && texFile1[nFileSize-8] == '_' && texFile1[nFileSize-7] == 'p' && texFile1[nFileSize-6] == 'g' && texFile1[nFileSize-4] == '.')
				{
					int nPhysicsGroup = (texFile1[nFileSize-5] - '0');
					if(nPhysicsGroup >=0 && nPhysicsGroup<=9)
					{
						m_pMaterials[i].m_nPhysicsGroup = nPhysicsGroup;
						m_pMaterials[i].EnableAttribute(CParaXMaterial::MATERIAL_SKIP_RENDER, true);
					}
				}
				m_pMaterials[i].m_pTexture1 = LoadTextureByPath(texFile1, TextureEntity::StaticTexture);

				// check if there is any replaceable texture ID in texFile2, texFile2 will be "[0-9]*"
				// generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
				if(!texFile2.empty() && texFile2[0] >= '0' && texFile2[0]<='9')
				{
					try
					{
						int nID = -1;
						nID = atoi(texFile2.c_str());
						assert(nID<32);
						m_pMaterials[i].m_nReplaceableTextureID = nID;
						if(nID>=(int)m_ReplaceableTextures.size())
						{
							m_ReplaceableTextures.resize(nID+1);
						}
						// use the default replaceable texture,which is the main texture stored at the material. 
						SetReplaceableTexture(nID, m_pMaterials[i].m_pTexture1.get());
					}
					catch (...)
					{
					}
				}
				
				if(m_pMaterials[i].hasReflectionMap())
				{
					// disable lightmap, if reflection map is enabled; 
					m_pMaterials[i].EnableAttribute(CParaXMaterial::MATERIAL_LIGHTMAP, false);
				}

				if(m_pMaterials[i].hasNormalMap())
				{
					// TODO: untested norm map
					string sFilePath = texFile1;
					int nCount = (int)sFilePath.size();
					if(nCount > 5)
					{
						// add Norm after the main texture file to get the normal map. 
						sFilePath.insert(sFilePath.size()-4, "Norm");
						m_pMaterials[i].m_pTexture2 = LoadTextureByPath(sFilePath, TextureEntity::StaticTexture);
					}
				}
				else if(m_pMaterials[i].hasLightMap())
				{
					// TODO: untested light map
					m_pMaterials[i].m_pTexture2 = LoadTextureByPath(texFile2, TextureEntity::StaticTexture);
				}
				else if(m_pMaterials[i].hasEnvironmentMap())
				{
					if(!texFile2.empty())
					{
						m_pMaterials[i].m_pTexture2 = LoadTextureByPath(texFile2, TextureEntity::CubeTexture);
					}
					else
					{
						// if environment is not in the second texture, we will deduce its name from the first texture, 
						// this is mostly for backward compatibilities.
						string sFilePath = texFile1;
						int nCount = (int)sFilePath.size();
						if(nCount > 5)
						{
							// add Env after the main texture file to get the normal map. 
							sFilePath.insert(sFilePath.size()-4, "Env");
							m_pMaterials[i].m_pTexture2 = LoadTextureByPath(sFilePath, TextureEntity::CubeTexture);
						}
					}
				}
			}
        }
    }

	if(bNeedSecondPass)
	{
		//////////////////////////////////////////////////////////////////////////
		// build face group.

		// reset
		ClearFaceGroups();

		DWORD dwFVF = m_pSysMemMesh->GetFVF();
		// only build face group for mesh_vertex_normal compatible vertex format. 
		if(dwFVF == mesh_vertex_normal::FVF)
		{
			const mesh_vertex_normal *pVert = NULL;
			if (SUCCEEDED(m_pSysMemMesh->LockVertexBuffer(D3DLOCK_READONLY,(LPVOID *) &pVert) ))
			{
				const WORD* pIndice = NULL;
				if (SUCCEEDED(m_pSysMemMesh->LockIndexBuffer(D3DLOCK_READONLY,(LPVOID *) &pIndice) ))
				{
					for( DWORD i=0; i<m_dwNumMaterials; i++ )
					{
						int nTriangleCount = atts[i].FaceCount;
						int nIndexStart = 3 * atts[i].FaceStart;
						if(nTriangleCount<=0)
							continue;
						if(m_pMaterials[i].hasTranslucentSort())
						{
							CFaceGroup * pFaceGroup = new CFaceGroup();
							m_faceGroups.push_back(pFaceGroup);

							m_pMaterials[i].m_nTranslucentFaceGroupIndex = (int)(m_faceGroups.size()-1);

							// copy data from mesh and material to face group.
							pFaceGroup->m_pTexture = m_pMaterials[i].m_pTexture1.get();
							pFaceGroup->m_alphaBlending = m_pMaterials[i].hasAlphaBlending();
							pFaceGroup->m_alphaTesting = m_pMaterials[i].hasAlphaTesting();
							pFaceGroup->m_bHasLighting = m_pMaterials[i].hasLighting();
							pFaceGroup->m_disableZWrite = m_pMaterials[i].hasZMask();
							pFaceGroup->m_bAdditive = m_pMaterials[i].hasAdditive();
							pFaceGroup->m_material = m_pMaterials[i].mat;

							// this gives zwrite enabled face a high priority to be rendered higher.
							if(!pFaceGroup->m_disableZWrite)
								pFaceGroup->m_order = 0;
							else
								pFaceGroup->m_order = 1;

							pFaceGroup->m_nNumTriangles = nTriangleCount;

							for (int k=0;k<nTriangleCount;++k)
							{
								for (int j=0;j<3;j++)
								{
									const mesh_vertex_normal& v = pVert[pIndice[nIndexStart+k*3+j]];
									pFaceGroup->m_vertices.push_back(v.p);
									pFaceGroup->m_normals.push_back(v.n);
									pFaceGroup->m_UVs.push_back(v.uv);
								}
							}
							pFaceGroup->UpdateCenterPos();
						}
						if(m_pMaterials[i].hasReflectionMap())
						{
							if(nTriangleCount >= 1)
							{
								int k=0; // use the first triangle height for the reflection map height.
								float fHeight = 0.f;
								for (int j=0;j<3;j++)
								{
									const mesh_vertex_normal& v = pVert[pIndice[nIndexStart+k*3+j]];
									fHeight += v.p.y;
								}
								fHeight /= 3.f;
								m_pMaterials[i].m_fRelectionHeight = fHeight;
							}
						}
						if(m_pMaterials[i].hasAnyBillboard())
						{
							// get the pivot point, which is the center of the sub mesh
							Vector3 vPivot (0,0,0);
							for (int k=0;k<nTriangleCount;++k)
							{
								for (int j=0;j<3;j++)
								{
									const mesh_vertex_normal& v = pVert[pIndice[nIndexStart+k*3+j]];
									vPivot += v.p;
								}
							}
							m_pMaterials[i].m_vPivot = vPivot / (nTriangleCount*3.f);
						}
					}
					m_pSysMemMesh->UnlockIndexBuffer();
				}
				m_pSysMemMesh->UnlockVertexBuffer();
			}
		}
		else if(dwFVF == mesh_vertex_normal_tex2::FVF)
		{
			const mesh_vertex_normal_tex2 *pVert = NULL;
			if (SUCCEEDED(m_pSysMemMesh->LockVertexBuffer(D3DLOCK_READONLY,(LPVOID *) &pVert) ))
			{
				const WORD* pIndice = NULL;
				if (SUCCEEDED(m_pSysMemMesh->LockIndexBuffer(D3DLOCK_READONLY,(LPVOID *) &pIndice) ))
				{
					for( DWORD i=0; i<m_dwNumMaterials; i++ )
					{
						int nTriangleCount = atts[i].FaceCount;
						int nIndexStart = 3 * atts[i].FaceStart;

						if(m_pMaterials[i].hasReflectionMap())
						{
							if(nTriangleCount >= 1)
							{
								int k=0; // use the first triangle height for the reflection map height.
								float fHeight = 0.f;
								for (int j=0;j<3;j++)
								{
									const mesh_vertex_normal_tex2& v = pVert[pIndice[nIndexStart+k*3+j]];
									fHeight += v.p.y;
								}
								fHeight /= 3.f;
								m_pMaterials[i].m_fRelectionHeight = fHeight;
							}
						}
					}
					m_pSysMemMesh->UnlockIndexBuffer();
				}
				m_pSysMemMesh->UnlockVertexBuffer();
			}
		}
	}
    return S_OK;
}

TextureEntity*	CParaXStaticMesh::LoadTextureByPath(const string& sFilePath, DWORD TextureType)
{
	TextureEntity* pTex=NULL;
	// Initialize textures from texture file
	if(!CParaFile::IsFileName(sFilePath))
		pTex = CGlobals::GetAssetManager()->LoadTexture("", sFilePath, (TextureEntity::_SurfaceType)TextureType);
	else
	{
		string sNewTexPath = CParaFile::GetAbsolutePath(sFilePath, CParaFile::GetParentDirectoryFromPath(m_strName));
		pTex = CGlobals::GetAssetManager()->LoadTexture("", sNewTexPath, (TextureEntity::_SurfaceType)TextureType);
	}
	return pTex;
}
//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::SetFVF( LPDIRECT3DDEVICE9 pd3dDevice, DWORD dwFVF )
{
    LPD3DXMESH pTempSysMemMesh = NULL;
    LPD3DXMESH pTempLocalMesh  = NULL;

    if( m_pSysMemMesh )
    {
        if( FAILED( m_pSysMemMesh->CloneMeshFVF( m_pSysMemMesh->GetOptions(), dwFVF,
                                                 pd3dDevice, &pTempSysMemMesh ) ) )
            return E_FAIL;
    }
    if( m_pLocalMesh )
    {
        if( FAILED( m_pLocalMesh->CloneMeshFVF( m_pLocalMesh->GetOptions(), dwFVF, pd3dDevice,
                                                &pTempLocalMesh ) ) )
        {
            SAFE_RELEASE( pTempSysMemMesh );
            return E_FAIL;
        }
    }

    DWORD dwOldFVF = m_pSysMemMesh->GetFVF();

    SAFE_RELEASE( m_pSysMemMesh );
    SAFE_RELEASE( m_pLocalMesh );

    if( pTempSysMemMesh ) m_pSysMemMesh = pTempSysMemMesh;
    if( pTempLocalMesh )  m_pLocalMesh  = pTempLocalMesh;

    // Compute normals if they are being requested and
    // the old mesh does not have them.
    if( !(dwOldFVF & D3DFVF_NORMAL) && dwFVF & D3DFVF_NORMAL )
    {
        if( m_pSysMemMesh )
            D3DXComputeNormals( m_pSysMemMesh, NULL );
        if( m_pLocalMesh )
            D3DXComputeNormals( m_pLocalMesh, NULL );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CParaXStaticMesh::SetVertexDecl
// Desc: Convert the mesh to the format specified by the given vertex
//       declarations.
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::SetVertexDecl( LPDIRECT3DDEVICE9 pd3dDevice, const D3DVERTEXELEMENT9 *pDecl )
{
    LPD3DXMESH pTempSysMemMesh = NULL;
    LPD3DXMESH pTempLocalMesh  = NULL;

    if( m_pSysMemMesh )
    {
        if( FAILED( m_pSysMemMesh->CloneMesh( m_pSysMemMesh->GetOptions(), pDecl,
                                              pd3dDevice, &pTempSysMemMesh ) ) )
            return E_FAIL;
    }

    if( m_pLocalMesh )
    {
        if( FAILED( m_pLocalMesh->CloneMesh( m_pLocalMesh->GetOptions(), pDecl, pd3dDevice,
                                             &pTempLocalMesh ) ) )
        {
            SAFE_RELEASE( pTempSysMemMesh );
            return E_FAIL;
        }
    }

    // Check if the old declaration contains a normal.
    bool bHadNormal = false;
    D3DVERTEXELEMENT9 aOldDecl[MAX_FVF_DECL_SIZE];
    if( SUCCEEDED( m_pSysMemMesh->GetDeclaration( aOldDecl ) ) )
    {
        for( UINT index = 0; index < D3DXGetDeclLength( aOldDecl ); ++index )
            if( aOldDecl[index].Usage == D3DDECLUSAGE_NORMAL )
            {
                bHadNormal = true;
                break;
            }
    }

    // Check if the new declaration contains a normal.
    bool bHaveNormalNow = false;
    D3DVERTEXELEMENT9 aNewDecl[MAX_FVF_DECL_SIZE];
    if( SUCCEEDED( pTempSysMemMesh->GetDeclaration( aNewDecl ) ) )
    {
        for( UINT index = 0; index < D3DXGetDeclLength( aNewDecl ); ++index )
            if( aNewDecl[index].Usage == D3DDECLUSAGE_NORMAL )
            {
                bHaveNormalNow = true;
                break;
            }
    }

    SAFE_RELEASE( m_pSysMemMesh );
    SAFE_RELEASE( m_pLocalMesh );

    if( pTempSysMemMesh )
    {
        m_pSysMemMesh = pTempSysMemMesh;

        if( !bHadNormal && bHaveNormalNow )
        {
            // Compute normals in case the meshes have them
            D3DXComputeNormals( m_pSysMemMesh, NULL );
        }
    }

    if( pTempLocalMesh )
    {
        m_pLocalMesh = pTempLocalMesh;

        if( !bHadNormal && bHaveNormalNow )
        {
            // Compute normals in case the meshes have them
            D3DXComputeNormals( m_pLocalMesh, NULL );
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::InitDeviceObjects()
{
    if( NULL == m_pSysMemMesh )
        return E_FAIL;

	if(m_pLocalMesh == 0)
	{
		// Make a local memory version of the mesh. Note: because we are passing in
		// no flags, the default behavior is to clone into local memory.
		
		if( FAILED( m_pSysMemMesh->CloneMeshFVF( D3DXMESH_MANAGED | ( m_pSysMemMesh->GetOptions() & ~D3DXMESH_SYSTEMMEM ),
												 m_pSysMemMesh->GetFVF(), CGlobals::GetRenderDevice(), &m_pLocalMesh ) ) )
			return E_FAIL;
	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pLocalMesh );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::Destroy()
{
    DeleteDeviceObjects();
	
	m_ReplaceableTextures.clear();

    SAFE_DELETE_ARRAY( m_pMaterials );

    SAFE_RELEASE( m_pSysMemMesh );

    m_dwNumMaterials = 0L;

	ClearFaceGroups();

	DeleteMeshFileData();
    return S_OK;
}

void CParaXStaticMesh::ClearFaceGroups()
{
	//////////////////////////////////////////////////////////////////////////
	// delete all face group. 
	for(int i=0;i<(int)m_faceGroups.size();++i)
	{
		SAFE_DELETE(m_faceGroups[i]);
	}
	m_faceGroups.clear();
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::Render( SceneState * pSceneState, LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawOpaqueSubsets,
                          bool bDrawAlphaSubsets,float fAlphaFactor)
{
    if( NULL == m_pLocalMesh )
        return E_FAIL;

	pd3dDevice->SetTransform( D3DTS_WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
	
	bool bHasAlphaPass = false;

	/// bl=bBlending and at=bAlphaTesting 
	bool bl = false, at= false, bDisableBackFacing = false,zm=false;
	bool bAdditive = false;
    // Fist, draw the subsets without alpha
    if( bDrawOpaqueSubsets )
    {
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
			CParaXMaterial* pMaterial = &(m_pMaterials[i]);
			if(pMaterial->isSkipRendering())
				continue;
			// skip translucent face group.
			if(pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex>=0)
			{
				if(! pSceneState->IsIgnoreTransparent())
				{
					// skip shadow pass
					if(!CGlobals::GetEffectManager()->IsUsingShadowMap())
					{
						if(pMaterial->hasAnyBillboard())
						{
							Matrix4 mtrans;
							Math::CreateBillboardMatrix(&mtrans,NULL, &(pMaterial->m_vPivot), pMaterial->hasBillboardAA());
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

			LPDIRECT3DTEXTURE9 pTexture = NULL;
			if(pMaterial->m_pTexture1!=0)
			{
				if(pMaterial->m_nReplaceableTextureID<0)
				{
					pTexture = pMaterial->m_pTexture1->GetTexture();
				}
				else
				{
					TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
					if(pTextureEntity)
					{
						pTexture = pTextureEntity->GetTexture();
					}
					if(pTexture==0)
					{
						// use default one if user defined is nil
						pTexture = pMaterial->m_pTexture1->GetTexture();
					}
				}
			}

            if( m_bUseMaterials )
            {
				float fOldAlpha = pMaterial->mat.Diffuse.a;
				if( fOldAlpha < 1.0f){
					bHasAlphaPass = true;
                    continue;
				}

				if(pTexture)
				{
					CGlobals::GetEffectManager()->applySurfaceMaterial(&pMaterial->mat);
					bool bUseAdditive = pMaterial->hasAdditive();
					/** turn on or off z write: z mask means disable z write */
					if(bUseAdditive && !bAdditive){
						if(pMaterial->hasAlphaBlending())
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
					else if(!bUseAdditive && bAdditive){
						pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
						pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
						bAdditive = false;
					}

					/// set blending and alpha testing
					bool blend = /*TODO:(fAlphaFactor<1.0f) ||*/ pMaterial->hasAlphaBlending() || bUseAdditive;
#ifdef FORCE_ALPHATEST_ON_BLENDING
					bool atest = blend || /* make alpha test whenever alpha blend*/pMaterial->hasAlphaTesting();
#else
					bool atest = pMaterial->hasAlphaTesting(); // alpha testing is not dependents on alpha blending now.
#endif
					bool zmask = pMaterial->hasZMask();

					/** if either blending or alpha testing is enabled, we will disable back face culling.*/
					if((blend || atest) && (!bDisableBackFacing))
					{
						CGlobals::GetEffectManager()->SetCullingMode(false);
						bDisableBackFacing = true;
					}
					else if((!blend && !atest) && bDisableBackFacing)
					{
						CGlobals::GetEffectManager()->SetCullingMode(true);
						bDisableBackFacing = false;
					}

					/** turn on and off blending. */
					if (blend && !bl ) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
						bl = true;
					}
					else if (!blend && bl) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
						bl = false;
					}
					/** turn on and off alpha testing. */
					if (atest && !at) 
					{
						pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
						at = true;
					}
					else if (!atest && at) 
					{
						pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

						at = false;
					}
					/** turn on or off z write: z mask means disable z write */
					/*if(zmask && !zm){
					CGlobals::GetEffectManager()->EnableZWrite(false);
					zm = true;
					}
					else if(!zmask && zm){
					CGlobals::GetEffectManager()->EnableZWrite(true);
					zm = false;
					}*/
					
					pd3dDevice->SetTexture( 0, pTexture);
				}
				else
					pd3dDevice->SetTexture(0,NULL);
            }
			if(pMaterial->hasAnyBillboard())
			{
				Matrix4 mtrans;
				Math::CreateBillboardMatrix(&mtrans,NULL, &(pMaterial->m_vPivot));
				mtrans = mtrans * CGlobals::GetWorldMatrixStack().SafeGetTop();
				CGlobals::GetWorldMatrixStack().push(mtrans);
				pd3dDevice->SetTransform( D3DTS_WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
			}
			if(pTexture)
				m_pLocalMesh->DrawSubset( i );
			else
			{
#ifdef RENDER_NO_TEXTURE
				pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
				m_pLocalMesh->DrawSubset( i );
				pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif
			}
			if(pMaterial->hasAnyBillboard())
			{
				CGlobals::GetWorldMatrixStack().pop();
				pd3dDevice->SetTransform( D3DTS_WORLD, CGlobals::GetWorldMatrixStack().SafeGetTop().GetConstPointer());
			}
        }
    }

    // Then, draw the subsets with alpha
    if( bHasAlphaPass && bDrawAlphaSubsets && m_bUseMaterials )
    {
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
			CParaXMaterial* pMaterial = &(m_pMaterials[i]);
			if(pMaterial->isSkipRendering())
				continue;
            if( pMaterial->mat.Diffuse.a == 1.0f )
                continue;

			// skip translucent face group.
			if(pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex>=0)
			{
				if(! pSceneState->IsIgnoreTransparent())
				{
					// skip shadow pass
					if(!CGlobals::GetEffectManager()->IsUsingShadowMap())
						pSceneState->GetFaceGroups()->AddFaceGroup(CFaceGroupInstance(&(CGlobals::GetWorldMatrixStack().top()), m_faceGroups[pMaterial->m_nTranslucentFaceGroupIndex]));
				}
				continue;
			}

			LPDIRECT3DTEXTURE9 pTexture = NULL;
			if(pMaterial->m_pTexture1!=0)
			{
				if(pMaterial->m_nReplaceableTextureID<0)
				{
					pTexture = pMaterial->m_pTexture1->GetTexture();
				}
				else
				{
					TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
					if(pTextureEntity)
					{
						pTexture = pTextureEntity->GetTexture();
					}
					if(pTexture==0)
					{
						// use default one if user defined is nil
						pTexture = pMaterial->m_pTexture1->GetTexture();
					}
				}
			}
			// Set the material and texture
            pd3dDevice->SetTexture( 0, pTexture);
			if(pTexture)
			{
				CGlobals::GetEffectManager()->applySurfaceMaterial(&pMaterial->mat);
				m_pLocalMesh->DrawSubset( i );
			}
			else
			{
#ifdef RENDER_NO_TEXTURE
				pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
				m_pLocalMesh->DrawSubset( i );
				pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif
			}
        }
    }
	if(bl)
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	if(at)
		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	if(zm)
		CGlobals::GetEffectManager()->EnableZWrite(true);
	if(bDisableBackFacing)
		CGlobals::GetEffectManager()->SetCullingMode(true);
	if(bAdditive){
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
    return S_OK;
}

HRESULT CParaXStaticMesh::DrawSubSetEx(int nIndex, D3DXATTRIBUTERANGE* pAtts)
{
	 RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
	 const D3DXATTRIBUTERANGE& att = pAtts[nIndex];
	 return RenderDevice::DrawIndexedPrimitive(pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLELIST, 0, att.VertexStart, att.VertexCount, att.FaceStart*3, att.FaceCount);
}


//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CParaXStaticMesh::Render( SceneState * pSceneState, CEffectFile *pEffect,
                           bool bDrawOpaqueSubsets,
                           bool bDrawAlphaSubsets,
						   float fAlphaFactor, CParameterBlock* materialParams)
{
    if( NULL == m_pLocalMesh )
        return E_FAIL;
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	if(atts.size()<m_dwNumMaterials)
	{
		atts.resize(m_dwNumMaterials);
	}
	m_pLocalMesh->GetAttributeTable(&(atts[0]), &m_dwNumMaterials);

#ifdef USE_DRAW_SUBSET_EX
	LPDIRECT3DINDEXBUFFER9  pIndexBuffer = NULL;
	m_pLocalMesh->GetIndexBuffer(&pIndexBuffer);
	LPDIRECT3DVERTEXBUFFER9  pVertexBuffer = NULL;
	m_pLocalMesh->GetVertexBuffer(&pVertexBuffer);
	pd3dDevice->SetStreamSource(0, pVertexBuffer, 0, m_pLocalMesh->GetNumBytesPerVertex());
	pd3dDevice->SetIndices(pIndexBuffer);
#endif


	bool bHasAlphaPass = false;
    UINT cPasses = (UINT)pEffect->totalPasses();
	bool bl=false,at= false, bDisableBackFacing = false,zm=false;
	bool bEnvMap = false, bReflectionMap = false, bNormalMap = false, bHasLightMap = false, bHasLighting = true;
	bool bAdditive = false;
    // first, draw the subsets without alpha
    if( bDrawOpaqueSubsets )
    {
        if(pEffect->begin())
		{
			for( UINT p = 0; p < cPasses; ++p )
			{
				pEffect->BeginPass( p );

				// TODO: maybe this is not necessary if shadow texture can be shared between passes, but i have no idea how to do it.
				if(CGlobals::GetEffectManager()->IsUsingShadowMap())
				{
					CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
					if (pShadowMap !=NULL)
					{
						if(pShadowMap->UsingBlurredShadowMap())
						{
							pEffect->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
							pShadowMap->SetShadowTexture(*pEffect, 2, 1);
						}
						else
						{
							pEffect->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
							pShadowMap->SetShadowTexture(*pEffect, 2);
						}
						// pShadowMap->SetShadowTexture(*pEffect, 2);
					}
				}
				for( DWORD i=0; i<m_dwNumMaterials; i++ )
				{
					CParaXMaterial* pMaterial = &(m_pMaterials[i]);

					if(pMaterial->isSkipRendering())
						continue;
					// skip translucent face group.
					if(pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex>=0)
					{
						if(! pSceneState->IsIgnoreTransparent())
						{
							// skip shadow pass
							if(!CGlobals::GetEffectManager()->IsUsingShadowMap())
							{
								if(pMaterial->hasAnyBillboard())
								{
									Matrix4 mtrans;
									Math::CreateBillboardMatrix(&mtrans,NULL, &(pMaterial->m_vPivot), pMaterial->hasBillboardAA());
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

					LPDIRECT3DTEXTURE9 pTexture = NULL;
					if( m_bUseMaterials )
					{
						if( pMaterial->mat.Diffuse.a < 1.0f ){
							bHasAlphaPass = true;
							continue;
						}
						if(pMaterial->m_pTexture1!=0)
						{
							if(pMaterial->m_nReplaceableTextureID<0)
							{
								pTexture = pMaterial->m_pTexture1->GetTexture();
							}
							else
							{
								TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
								if(pTextureEntity)
								{
									pTexture = pTextureEntity->GetTexture();
								}
								if(pTexture==0)
								{
									// use default one if user defined is nil
									pTexture = pMaterial->m_pTexture1->GetTexture();
								}
							}
						}

						if(pTexture)
						{
							pEffect->applySurfaceMaterial( &pMaterial->mat );

							//////////////////////////////////////////////////////////////////////////
							// environment mapping
							if(bEnvMap != pMaterial->hasEnvironmentMap())
							{
								if(pEffect->EnableEnvironmentMapping(!bEnvMap))
									bEnvMap = !bEnvMap;
							}
							if(bEnvMap)
							{
								if(pMaterial->m_pTexture2!=0)
								{
									pEffect->SetReflectFactor(pMaterial->GetReflectivity());
									pEffect->setTexture( 2, pMaterial->m_pTexture2.get()); // sampler assumed to be on register s2.
								}
							}

							//////////////////////////////////////////////////////////////////////////
							// normal mapping
							if(bNormalMap != pMaterial->hasNormalMap())
							{
								// TODO: untested normal map
								bNormalMap = !bNormalMap;
							}
							if(bNormalMap)
							{
								if(pMaterial->m_pTexture2!=0)
									pEffect->setTexture( 1, pMaterial->m_pTexture2.get());
							}

							//////////////////////////////////////////////////////////////////////////
							// light mapping
							if(bHasLightMap != pMaterial->hasLightMap())
							{
								// TODO: untested normal map
								bHasLightMap = !bHasLightMap;
							}
							if(bHasLightMap)
							{
								if(pMaterial->m_pTexture2!=0)
								{
									// the reflectivity parameter is used for the light map scale factor. 
									float fLightMapScale = pMaterial->GetReflectivity();
									if(fLightMapScale<0.99f)
										fLightMapScale *= 10.f;
									pEffect->SetReflectFactor(fLightMapScale);
									pEffect->setTexture( 1, pMaterial->m_pTexture2.get());
								}
							}

							//////////////////////////////////////////////////////////////////////////
							// reflection mapping
							if(bReflectionMap != pMaterial->hasReflectionMap())
							{
								// prevent drawing itself?
								//if(!bReflectionMap && !CGlobals::GetEffectManager()->IsReflectionRenderingEnabled())
								//	continue;
								if(pEffect->EnableReflectionMapping(!bReflectionMap, pMaterial->m_fRelectionHeight))
									bReflectionMap = !bReflectionMap;

							}
							if(bReflectionMap)
								pEffect->SetReflectFactor(pMaterial->GetReflectivity());

							//////////////////////////////////////////////////////////////////////////
							// lighting
							if(bHasLighting != pMaterial->hasLighting())
							{
								bHasLighting = !bHasLighting;
								pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled() &&  bHasLighting);
							}

							
							bool bUseAdditive = pMaterial->hasAdditive();
							/** turn on or off z write: z mask means disable z write */
							if(bUseAdditive && !bAdditive){
								if(pMaterial->hasAlphaBlending())
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
							else if(!bUseAdditive && bAdditive){
								pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
								pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
								bAdditive = false;
							}

							/// set blending and alpha testing
							bool blend = pMaterial->hasAlphaBlending() || bUseAdditive;

							if(!blend)
							{
								// enable blending if "g_opacity" is specified and smaller than 1.f
								if(materialParams != NULL)
								{
									CParameter* pParams = materialParams->GetParameter("g_opacity");
									if(pParams && ((float)(*pParams)) < 1.f )
										blend = true;
								}
							}

							
#ifdef FORCE_ALPHATEST_ON_BLENDING
							bool atest = blend || /* make alpha test whenever alpha blend*/pMaterial->hasAlphaTesting();
#else
							bool atest = pMaterial->hasAlphaTesting(); // alpha testing is not dependents on alpha blending now.
#endif
	
							bool zmask = pMaterial->hasZMask();

							/** if either blending or alpha testing is enabled, we will disable back face culling.*/
							if((blend || atest) && (!bDisableBackFacing))
							{
								CGlobals::GetEffectManager()->SetCullingMode(false);
								bDisableBackFacing = true;
							}
							else if((!blend && !atest) && bDisableBackFacing)
							{
								CGlobals::GetEffectManager()->SetCullingMode(true);
								bDisableBackFacing = false;
							}

							/** turn on and off blending. */
							if (blend && !bl ) 
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
							if(zmask && !zm){
								CGlobals::GetEffectManager()->EnableZWrite(false);
								zm = true;
							}
							else if(!zmask && zm){
								CGlobals::GetEffectManager()->EnableZWrite(true);
								zm = false;
							}

							pEffect->setTexture( 0, pTexture);
						}
					}

					if(pMaterial->hasAnyBillboard())
					{
						Matrix4 mtrans;
						Math::CreateBillboardMatrix(&mtrans,NULL, &(pMaterial->m_vPivot));
						mtrans = mtrans * CGlobals::GetWorldMatrixStack().SafeGetTop();
						CGlobals::GetWorldMatrixStack().push(mtrans);
						pEffect->applyCameraMatrices();
					}

					pEffect->CommitChanges();
					if(pTexture)
					{
#ifdef USE_DRAW_SUBSET_EX
						DrawSubSetEx(i, &atts[0]);
#else
						m_pLocalMesh->DrawSubset( i );
#endif
					}
					else
					{
#ifdef RENDER_NO_TEXTURE
						pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
#ifdef USE_DRAW_SUBSET_EX
						DrawSubSetEx(i, &atts[0]);
#else
						m_pLocalMesh->DrawSubset( i );
#endif
						pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif
					}
					if(pMaterial->hasAnyBillboard())
					{
						CGlobals::GetWorldMatrixStack().pop();
						pEffect->applyCameraMatrices();
					}
				}
				pEffect->EndPass();
			}
			pEffect->end();
		}
        
    }

    // Then, draw the subsets with alpha
    if( bHasAlphaPass &&  bDrawAlphaSubsets && m_bUseMaterials )
    {
        if(pEffect->begin())
		{
			for( UINT p = 0; p < cPasses; ++p )
			{
				pEffect->BeginPass( p );
				for( DWORD i=0; i<m_dwNumMaterials; i++ )
				{
					CParaXMaterial* pMaterial = &(m_pMaterials[i]);
					if(pMaterial->isSkipRendering())
						continue;
					// skip translucent face group.
					if(pSceneState->m_bEnableTranslucentFaceSorting && pMaterial->hasTranslucentSort() && pMaterial->m_nTranslucentFaceGroupIndex>=0)
					{
						if(! pSceneState->IsIgnoreTransparent())
						{
							if( m_bUseMaterials && pMaterial->mat.Diffuse.a == 1.0f )
								continue;
							// skip shadow pass
							if(!CGlobals::GetEffectManager()->IsUsingShadowMap())
								pSceneState->GetFaceGroups()->AddFaceGroup(CFaceGroupInstance(&(CGlobals::GetWorldMatrixStack().top()), m_faceGroups[pMaterial->m_nTranslucentFaceGroupIndex]));
						}
						
						continue;
					}

					LPDIRECT3DTEXTURE9 pTexture = NULL;
					// TODO: set materials
					if( m_bUseMaterials )
					{
						if( pMaterial->mat.Diffuse.a == 1.0f ){
							continue;
						}
						if(pMaterial->m_pTexture1!=0)
						{
							if(pMaterial->m_nReplaceableTextureID<0)
							{
								pTexture = pMaterial->m_pTexture1->GetTexture();
							}
							else
							{
								TextureEntity* pTextureEntity = GetReplaceableTexture(pMaterial->m_nReplaceableTextureID);
								if(pTextureEntity)
								{
									pTexture = pTextureEntity->GetTexture();
								}
								if(pTexture==0)
								{
									// use default one if user defined is nil
									pTexture = pMaterial->m_pTexture1->GetTexture();
								}
							}
						}

						if(pTexture)
						{
							pEffect->applySurfaceMaterial( &pMaterial->mat );
							pEffect->setTexture( 0, pTexture );
							pEffect->CommitChanges();
						}
					}
					if(pTexture)
					{
#ifdef USE_DRAW_SUBSET_EX
						DrawSubSetEx(i, &atts[0]);
#else
						m_pLocalMesh->DrawSubset( i );
#endif
					}
					else
					{
#ifdef RENDER_NO_TEXTURE
						pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
#ifdef USE_DRAW_SUBSET_EX
						DrawSubSetEx(i, &atts[0]);
#else
						m_pLocalMesh->DrawSubset( i );
#endif
						pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
#endif
					}
				}
				pEffect->EndPass();
			}
			pEffect->end();
		}
    }
	if(bl)
		pEffect->EnableAlphaBlending(false);
	if(at)
		pEffect->EnableAlphaTesting(false);
	if(zm)
		CGlobals::GetEffectManager()->EnableZWrite(true);
		
	if(bDisableBackFacing)
		CGlobals::GetEffectManager()->SetCullingMode(true);
	if(bEnvMap)
		pEffect->EnableEnvironmentMapping(false);
	if(bReflectionMap)
		pEffect->EnableReflectionMapping(false);
	if(bNormalMap)
		pEffect->EnableNormalMap(false);
	if(bHasLightMap)
		pEffect->EnableLightMap(false);
	if(!bHasLighting)
		pEffect->EnableSunLight(CGlobals::GetScene()->IsLightEnabled());

	if(bAdditive){
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

#ifdef USE_DRAW_SUBSET_EX	
	SAFE_RELEASE(pIndexBuffer);
	SAFE_RELEASE(pVertexBuffer);
#endif
	return S_OK;
}



//TODO: there seems to be something wrong with this function
FLOAT CParaXStaticMesh::ComputeBoundingSphere(Vector3* vObjectCenter,FLOAT* fObjectRadius )
{
	HRESULT hr;
	// Lock the vertex buffer, to generate a simple bounding sphere
	LPDIRECT3DVERTEXBUFFER9 pMeshVB   = NULL;

	hr = m_pSysMemMesh->GetVertexBuffer( &pMeshVB );
	if( SUCCEEDED(hr) )
	{
		VOID *       pVertices;
		hr = pMeshVB->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
		if( SUCCEEDED(hr) )
		{
			hr = D3DXComputeBoundingSphere((DeviceVector3 *)pVertices,
				m_pSysMemMesh->GetNumVertices(),
				D3DXGetFVFVertexSize(m_pSysMemMesh->GetFVF()),
				(DeviceVector3*)vObjectCenter,
				fObjectRadius );
			pMeshVB->Unlock();
		}
		pMeshVB->Release();
	}
	if( FAILED(hr) )
		return 0;   
	return *fObjectRadius;
}

//---------------------------------------------------------------------------------
// Name: ComputeBoundingBox
// Desc: 
// pMin:[out] Pointer to a Vector3 structure, describing the returned lower-left 
// corner of the bounding box. See Remarks. 
// pMax:[out] Pointer to a Vector3 structure, describing the returned upper-right 
// corner of the bounding box. See Remarks.
//---------------------------------------------------------------------------------
HRESULT CParaXStaticMesh::ComputeBoundingBox(Vector3 *pMin, Vector3 *pMax )
{
	HRESULT hr = S_OK;

	if(m_pSysMemMesh!=0)
	{
		// Lock the vertex buffer, to generate a simple bounding sphere
		LPDIRECT3DVERTEXBUFFER9 pMeshVB   = NULL;

		hr = m_pSysMemMesh->GetVertexBuffer( &pMeshVB );
		if( SUCCEEDED(hr) )
		{
			VOID *       pVertices;
			hr = pMeshVB->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
			if( SUCCEEDED(hr) )
			{
				hr = ParaComputeBoundingBox((Vector3*)pVertices,
					m_pSysMemMesh->GetNumVertices(),
					D3DXGetFVFVertexSize(m_pSysMemMesh->GetFVF()),
					pMin,
					pMax);
				pMeshVB->Unlock();
			}
			pMeshVB->Release();
		}
	}
	else
		return E_FAIL;
	return hr;
}

void CParaXStaticMesh::SetMeshFileData(void * pData, int nSize)
{
	DeleteMeshFileData();
	m_pFileData = pData;
	m_pFileDataSize = nSize;
}
void CParaXStaticMesh::DeleteMeshFileData()
{
	SAFE_DELETE_ARRAY(m_pFileData);
	m_pFileDataSize = 0;
}


MeshHeader& CParaXStaticMesh::GetMeshHeader(LPD3DXFILE pFileParser)
{
	if(!m_header.m_bIsValid)
	{
		// let us compute it on the fly. 
		if(pFileParser == NULL)
			pFileParser = m_pDXFileParser;
		
		if(m_pFileData)
		{
			CParaFile myFile((char*)m_pFileData, m_pFileDataSize, false);
			GetMeshHeaderFromFile(myFile, pFileParser);
		}
		else
		{
			CParaFile myFile(m_strName.c_str());
			GetMeshHeaderFromFile(myFile, pFileParser);
		}
	}
	return m_header;
}

bool CParaXStaticMesh::GetMeshHeaderFromFile(CParaFile& myFile, LPD3DXFILE pFileParser)
{
	HRESULT      hr = S_OK;
	if(myFile.isEof())
		return false;

	ParaXParser p(myFile, pFileParser);
	CParaXSerializer s;
	if(s.LoadParaX_Header(p))
	{
		if(p.m_bHeaderLoaded)
		{
			m_header.m_bIsValid = true;
			// if there is a ParaX header, just get everything from it.
			m_header.m_vMin = p.m_xheader.minExtent;
			m_header.m_vMax = p.m_xheader.maxExtent;
			// get these data from other fields of header
			if(p.m_xheader.nModelFormat == 0)
			{
				m_header.m_bHasNormal = true;
				m_header.m_bHasTex2 = false;
			}
			else
			{
				// TODO: UNTESTED 
				m_header.m_bHasNormal = (p.m_xheader.nModelFormat & PARAX_FORMAT_NORMAL)>0;
				m_header.m_bHasTex2 = (p.m_xheader.nModelFormat & PARAX_FORMAT_UV2)>0;
			}
		}
		else
		{
			// if no available header, we will needs to load and compute at runtime.
			if(m_pSysMemMesh!=0)
			{
				m_header.m_bIsValid = true;

				// Lock the vertex buffer, to generate a simple bounding sphere
				LPDIRECT3DVERTEXBUFFER9 pMeshVB   = NULL;

				hr = m_pSysMemMesh->GetVertexBuffer( &pMeshVB );
				if( SUCCEEDED(hr) )
				{
					VOID *       pVertices;
					hr = pMeshVB->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
					if( SUCCEEDED(hr) )
					{
						hr = ParaComputeBoundingBox( (Vector3 *)pVertices, 
							m_pSysMemMesh->GetNumVertices(),
							D3DXGetFVFVertexSize(m_pSysMemMesh->GetFVF()),
							&(m_header.m_vMin),
							&(m_header.m_vMax));
						pMeshVB->Unlock();

						m_header.m_bHasNormal = false;
						m_header.m_bHasTex2 = false;
						int nTexture = 0;
						D3DVERTEXELEMENT9 aOldDecl[MAX_FVF_DECL_SIZE];
						if( SUCCEEDED( m_pSysMemMesh->GetDeclaration( aOldDecl ) ) )
						{
							int nCount = D3DXGetDeclLength( aOldDecl );
							for( int index = 0; index < nCount; ++index )
							{
								if( aOldDecl[index].Usage == D3DDECLUSAGE_NORMAL )
								{
									m_header.m_bHasNormal = true;
								}
								else if( aOldDecl[index].Usage == D3DDECLUSAGE_TEXCOORD )
								{
									nTexture++;
								}
							}
							m_header.m_bHasTex2 = (nTexture>=2);
						}
					}
					pMeshVB->Release();
				}
			}
			else
				return false;
		}
	}
	return m_header.m_bIsValid;
}

/**
* NOT USED any more, use the member function instead: this will get the bounding box of the mesh without loading it.
*/
HRESULT CParaXStaticMesh::GetMeshHeader( LPCSTR strFilename, LPD3DXFILE pFileParser, Vector3& vMin, Vector3& vMax, bool& bHasNormal,bool& bHasTex2  )
{
	HRESULT      hr = S_OK;

	CParaFile myFile(strFilename);
	if(myFile.isEof())
		return E_FAIL;

	ParaXParser p(myFile, pFileParser);
	CParaXSerializer s;
	if(s.LoadParaX_Header(p))
	{
		if(p.m_bHeaderLoaded)
		{
			// if there is a ParaX header, just get everything from it.
			vMin = p.m_xheader.minExtent;
			vMax = p.m_xheader.maxExtent;
			// get these data from other fields of header
			if(p.m_xheader.nModelFormat == 0)
			{
				bHasNormal = true;
				bHasTex2 = false;
			}
			else
			{
				// TODO: UNTESTED 
				bHasNormal = (p.m_xheader.nModelFormat & PARAX_FORMAT_NORMAL)>0;
				bHasTex2 = (p.m_xheader.nModelFormat & PARAX_FORMAT_UV2)>0;
			}
		}
		else
		{
			// if no available header, we will needs to load and compute at runtime.
			LPD3DXMESH pMesh=NULL;

			if(p.m_pD3DMesh!=NULL)
			{
				// Load the mesh from the DXFILEDATA object if mesh data is at top level,meaning there is no d3d frame wrapping it. 
				LPD3DXBUFFER pMtrlBuffer = NULL;
				LPD3DXBUFFER pAdjacencyBuffer = NULL;
				DWORD dwNumMaterials = 0;

				// LXZ 2008.1.8: D3DXLoadMeshFromXof requires &pAdjacencyBuffer, &pMtrlBuffer, however D3DXLoadMeshFromXInMemory does not need them. 
				if( FAILED( hr = D3DXLoadMeshFromXof( p.m_pD3DMesh, D3DXMESH_SYSTEMMEM, CGlobals::GetRenderDevice(),
					&pAdjacencyBuffer, &pMtrlBuffer, NULL,&dwNumMaterials, &pMesh ) ) )
				{
					return hr;
				}

				SAFE_RELEASE( pAdjacencyBuffer );
				SAFE_RELEASE( pMtrlBuffer );

				// * 2008.1.25 by LXZ: this is a tricky workaround of D3DXLoadMeshFromXOf can only load a simple mesh from data node. If the mesh contains frames or second UV sets, the function will not work. So in such cases, I fall back to use the original D3DXLoadMeshFromXInMemory, which can handle frames and second UV sets.
				// Note: this will not detect the second UV sets. Currently, the second UV set is always written in x header file. Old ParaX static files without header will not be rendered properly unless they have frame node around the mesh node. 
			}
			else
			{
				// if the mesh is inside some d3d frames, we will load the DirectX way, by collapsing all frames into one mesh
				if( FAILED( hr = D3DXLoadMeshFromXInMemory( myFile.getBuffer(), (int) (myFile.getSize()), D3DXMESH_SYSTEMMEM, CGlobals::GetRenderDevice(), 
					NULL, NULL, NULL,NULL, &pMesh ) ) )
				{
					return hr;
				}
			}
	
			// Lock the vertex buffer, to generate a simple bounding sphere
			LPDIRECT3DVERTEXBUFFER9 pMeshVB   = NULL;

			hr = pMesh->GetVertexBuffer( &pMeshVB );
			if( SUCCEEDED(hr) )
			{
				VOID *       pVertices;
				hr = pMeshVB->Lock( 0, 0, &pVertices, D3DLOCK_NOSYSLOCK );
				if( SUCCEEDED(hr) )
				{
					hr = ParaComputeBoundingBox( (Vector3 *)pVertices, 
						pMesh->GetNumVertices(),
						D3DXGetFVFVertexSize(pMesh->GetFVF()),
						&vMin,
						&vMax);
					pMeshVB->Unlock();

					bHasNormal = false;
					bHasTex2 = false;
					int nTexture = 0;
					D3DVERTEXELEMENT9 aOldDecl[MAX_FVF_DECL_SIZE];
					if( SUCCEEDED( pMesh->GetDeclaration( aOldDecl ) ) )
					{
						int nCount = D3DXGetDeclLength( aOldDecl );
						for( int index = 0; index < nCount; ++index )
						{
							if( aOldDecl[index].Usage == D3DDECLUSAGE_NORMAL )
							{
								bHasNormal = true;
							}
							else if( aOldDecl[index].Usage == D3DDECLUSAGE_TEXCOORD )
							{
								nTexture++;
							}
						}
						bHasTex2 = (nTexture>=2);
					}
					// define TEST_UV2 to output the second UV set
#ifdef  TEST_UV2
					DWORD FVF = pMesh->GetFVF();
					int nSize = D3DXGetFVFVertexSize(FVF);
					int nCount = pMesh->GetNumVertices();
					struct MyVertex 
					{
						Vector3 pos;
						Vector3 norm;
						Vector2 uv1;
						Vector2 uv2;
					};
					if(sizeof(MyVertex) == nSize)
					{
						for (int i=0;i<nCount;++i)
						{
							MyVertex v = *(((MyVertex*)pVertices)+i);
							OUTPUT_LOG("%f,%f,\r\n", v.uv2.x, v.uv2.y);
						}
					}
#endif
				}
				pMeshVB->Release();
			}

			pMesh->Release();
		}
	}
	return hr;
}


int CParaXStaticMesh::GetNextPhysicsGroupID(int nPhysicsGroup)
{
	int nNextID = -1;
	for( DWORD i=0; i<m_dwNumMaterials; i++ )
	{
		CParaXMaterial* pMaterial = &(m_pMaterials[i]);
		if(pMaterial->hasPhysics() && pMaterial->GetPhysicsGroup()>nPhysicsGroup )
		{
			if(nNextID>pMaterial->GetPhysicsGroup() || nNextID == -1)
			{
				nNextID = pMaterial->GetPhysicsGroup();
			}
		}
	}
	return nNextID;
}

HRESULT CParaXStaticMesh::ClonePhysicsMesh( DWORD* pNumVertices, Vector3 ** ppVerts, DWORD* pNumTriangles, WORD** ppIndices, int* pnMeshPhysicsGroup, int* pnTotalMeshGroupCount)
{
	if(pnTotalMeshGroupCount)
	{
		int nTotalMeshGroupCount = 0;
		int nPhysicsGroup = -1;
		while( (nPhysicsGroup = GetNextPhysicsGroupID(nPhysicsGroup)) >=0)
		{
			nTotalMeshGroupCount++;
		}
		*pnTotalMeshGroupCount = nTotalMeshGroupCount;
	}

	// get physics mesh data to pSysMesh with only vertex position vector
	LPD3DXMESH pSysMesh = GetSysMemMesh();
	if(pSysMesh==NULL)
	{
		if(pnTotalMeshGroupCount)
		{
			*pnTotalMeshGroupCount = 0;
		}
		return E_FAIL;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// get the physics faces count
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumFaces = 0;
	if(atts.size()<m_dwNumMaterials)
	{
		atts.resize(m_dwNumMaterials);
	}
	pSysMesh->GetAttributeTable(&(atts[0]), &m_dwNumMaterials);

	
	for( DWORD i=0; i<m_dwNumMaterials; i++ )
	{
		CParaXMaterial* pMaterial = &(m_pMaterials[i]);
		if(pMaterial->hasPhysics() && (pnMeshPhysicsGroup==0 || ((*pnMeshPhysicsGroup) == pMaterial->GetPhysicsGroup()) ) )
		{
			const D3DXATTRIBUTERANGE& att = atts[i];
			dwNumFaces+=att.FaceCount;
		}
	}
	if(dwNumFaces==0)
	{
		// in case, there is no physics faces in the mesh, return immediately.
		if(pNumVertices!=0)
			*pNumVertices = 0;
		if(pNumTriangles!=0)
			*pNumTriangles = 0;
		return S_OK;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// read the vertex buffer
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumVx = 0;
	Vector3 * verts = NULL;
	HRESULT hr = S_OK;
	if(ppVerts!=NULL)
	{
		LPDIRECT3DVERTEXBUFFER9 pMeshVB   = NULL;
		dwNumVx = pSysMesh->GetNumVertices();
		verts = new Vector3[dwNumVx];

		hr = pSysMesh->GetVertexBuffer( &pMeshVB );
		if( SUCCEEDED(hr) )
		{
			D3DVERTEXBUFFER_DESC desc;
			pMeshVB->GetDesc(&desc);
			int nBytesStride = desc.Size/dwNumVx;

			byte*  pVertices = NULL;
			hr = pMeshVB->Lock( 0, 0, (VOID **)&pVertices, D3DLOCK_NOSYSLOCK );
			if( SUCCEEDED(hr) )
			{
				for( DWORD i = 0; i < dwNumVx; ++ i )
				{
					// verts[i] = pVertices->vPos;
					// scale each vertex before hand
					verts[i] = *((Vector3*)pVertices);
					pVertices=pVertices+nBytesStride;
				}
				pMeshVB->Unlock();
			}
			pMeshVB->Release();
		}
		else
		{
			SAFE_DELETE_ARRAY(verts);
			return E_FAIL;
		}
	}
	

	//////////////////////////////////////////////////////////////////////////
	// read the index buffer
	//////////////////////////////////////////////////////////////////////////
	WORD* indices = NULL;
	if(ppIndices)
	{
		LPDIRECT3DINDEXBUFFER9 pMeshIB = NULL;
		hr = pSysMesh->GetIndexBuffer( &pMeshIB );
		indices = new WORD[dwNumFaces *3];
		if( SUCCEEDED(hr) )
		{
			WORD*       pIndex = NULL;
			hr = pMeshIB->Lock( 0, 0, (VOID **)&pIndex, D3DLOCK_NOSYSLOCK );
			if( SUCCEEDED(hr) )
			{
				int nD=0; // destination indices index
				for( DWORD i=0; i<m_dwNumMaterials; i++ )
				{
					CParaXMaterial* pMaterial = &(m_pMaterials[i]);
					if(pMaterial->hasPhysics() && (pnMeshPhysicsGroup==0 || ((*pnMeshPhysicsGroup) == pMaterial->GetPhysicsGroup()) ) )
					{
						const D3DXATTRIBUTERANGE& att = atts[i];
						int nNum = att.FaceCount*3;
						memcpy(&(indices[nD]), &(pIndex[att.FaceStart*3]), sizeof(WORD)*att.FaceCount*3);
						nD += att.FaceCount*3;
					}
				}
				pMeshIB->Unlock();
			}
			pMeshIB->Release();
		}
		else
		{
			SAFE_DELETE_ARRAY(indices);
			return E_FAIL;
		}
	}
	// output result
	if(pNumVertices!=0){
		*pNumVertices = dwNumVx;
	}
	if(ppVerts!=0){
		*ppVerts = verts;
	}
	if(pNumTriangles!=0){
		*pNumTriangles = dwNumFaces;
	}
	if(ppIndices!=0){
		*ppIndices = indices;
	}
	return hr;
}


int CParaXStaticMesh::GetNumReplaceableTextures()
{
	return (int)m_ReplaceableTextures.size();
}


TextureEntity* CParaXStaticMesh::GetDefaultReplaceableTexture( int ReplaceableTextureID )
{
	if(ReplaceableTextureID>=0 && (int)m_ReplaceableTextures.size()>ReplaceableTextureID)
	{
		for( DWORD i=0; i<m_dwNumMaterials; i++ )
		{
			if(m_pMaterials[i].m_nReplaceableTextureID == ReplaceableTextureID)
			{
				return m_pMaterials[i].m_pTexture1.get();
			}
		}
	}
	return NULL;
}

TextureEntity* CParaXStaticMesh::GetReplaceableTexture( int ReplaceableTextureID )
{
	if(ReplaceableTextureID>=0 && (int)m_ReplaceableTextures.size()>ReplaceableTextureID)
		return	m_ReplaceableTextures[ReplaceableTextureID].get();
	else
		return NULL;
}

bool CParaXStaticMesh::SetReplaceableTexture( int ReplaceableTextureID, TextureEntity* pTextureEntity )
{
	if((int)m_ReplaceableTextures.size()>ReplaceableTextureID && ReplaceableTextureID>=0)
	{
		m_ReplaceableTextures[ReplaceableTextureID] = pTextureEntity;
		return true;
	}
	else
	{
		return false;
	}
}

const ParaXRefObject* CParaXStaticMesh::GetXRefObjectsByIndex(int nIndex) 
{ 
	return (nIndex>=0 && nIndex< GetXRefObjectsCount()) ?  &(m_XRefObjects[nIndex]):NULL; 
}

int CParaXStaticMesh::GetPolyCount()
{
	// get physics mesh data to pSysMesh with only vertex position vector
	LPD3DXMESH pSysMesh = GetSysMemMesh();
	if(pSysMesh==NULL)
		return E_FAIL;

	//////////////////////////////////////////////////////////////////////////
	// get the physics faces count
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumFaces = 0;
	if(atts.size()<m_dwNumMaterials)
	{
		atts.resize(m_dwNumMaterials);
	}
	pSysMesh->GetAttributeTable(&(atts[0]), &m_dwNumMaterials);

	for( DWORD i=0; i<m_dwNumMaterials; i++ )
	{
		CParaXMaterial* pMaterial = &(m_pMaterials[i]);
		const D3DXATTRIBUTERANGE& att = atts[i];
		dwNumFaces+=att.FaceCount;
	}
	return dwNumFaces;
}

int CParaXStaticMesh::GetPhysicsCount()
{
	// get physics mesh data to pSysMesh with only vertex position vector
	LPD3DXMESH pSysMesh = GetSysMemMesh();
	if(pSysMesh==NULL)
		return E_FAIL;

	//////////////////////////////////////////////////////////////////////////
	// get the physics faces count
	//////////////////////////////////////////////////////////////////////////
	DWORD dwNumFaces = 0;
	if(atts.size()<m_dwNumMaterials)
	{
		atts.resize(m_dwNumMaterials);
	}
	pSysMesh->GetAttributeTable(&(atts[0]), &m_dwNumMaterials);

	for( DWORD i=0; i<m_dwNumMaterials; i++ )
	{
		CParaXMaterial* pMaterial = &(m_pMaterials[i]);
		if(pMaterial->hasPhysics())
		{
			const D3DXATTRIBUTERANGE& att = atts[i];
			dwNumFaces+=att.FaceCount;
		}
	}
	return dwNumFaces;
}

const char* CParaXStaticMesh::DumpTextureUsage()
{
	static string g_output;
	g_output.clear();
	char temp[200];
	int nCount = 0;
	for( DWORD i=0; i<m_dwNumMaterials; i++ )
	{
		CParaXMaterial* pMaterial = &(m_pMaterials[i]);
		if(pMaterial)
		{
			if(pMaterial->m_pTexture1.get() != 0)
			{
				nCount++;
				const TextureEntity::TextureInfo * pInfo = pMaterial->m_pTexture1->GetTextureInfo();
				if(pInfo)
				{
					snprintf(temp, 200, "%d*%d(%d)", pInfo->m_width, pInfo->m_height, pInfo->m_format);
					g_output.append(temp);
				}
				g_output.append(pMaterial->m_pTexture1->GetKey());
				g_output.append(";");
			}
		}
	}
	snprintf(temp, 200, "cnt:%d;",nCount);
	g_output = temp+g_output;

	return g_output.c_str();
}

int CParaXStaticMesh::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IAttributeFields::InstallFields(pClass, bOverride);

	assert(pClass!=NULL);
	pClass->AddField("TextureUsage",FieldType_String, NULL, DumpTextureUsage_s, NULL, NULL, bOverride);
	pClass->AddField("PolyCount",FieldType_Int, NULL, GetPolyCount_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsCount",FieldType_Int, NULL, GetPhysicsCount_s, NULL, NULL, bOverride);
	return S_OK;
}