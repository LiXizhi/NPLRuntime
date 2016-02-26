// Class:	CD3DMesh
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ZJU Right reserved
// Date:	2004.3.8
// Revised: 2004.3.8

// Desc: It's based on the common lib provided in directxSDK. I have modefied this 
// file as I need it. They includes:
// build-in mesh creation. See CParaWorldAssets for details
// Support code for loading DirectX .X mesh files.
//
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <dxfile.h>
//#include <rmxfguid.h>
//#include <rmxftmpl.h>
#include "D3DFile.h"



//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CD3DMesh::CD3DMesh( )
{
    m_pSysMemMesh        = NULL;
    m_pLocalMesh         = NULL;
    m_dwNumMaterials     = 0L;
    m_pMaterials         = NULL;
    m_pTextures          = NULL;
    m_bUseMaterials      = TRUE;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
CD3DMesh::~CD3DMesh()
{
    Destroy();
}




//-----------------------------------------------------------------------------
// Name:
// Desc: create from a file or build-in object
// See paraworldassets for more details.
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::Create( LPDIRECT3DDEVICE9 pd3dDevice, LPCTSTR strFilename )
{
	// --Paraworld: Create built-in objects
	// -- modified by LiXizhi@yeah.net
	if(strFilename[0] == '_')
	{
		if(strcmp(strFilename,_T("_floor")) == 0)
		{
			RestorePolygonMesh(pd3dDevice, 1, 4, &(m_pSysMemMesh));
			return S_OK;
		}
	}

	// -- file based mesh object creation. Below are original code by directx sdk sample
    TCHAR        strPath[MAX_PATH];
    LPD3DXBUFFER pAdjacencyBuffer = NULL;
    LPD3DXBUFFER pMtrlBuffer = NULL;
    HRESULT      hr;

    // Find the path for the file, and convert it to ANSI (for the D3DX API)
    DXUtil_FindMediaFileCb( strPath, sizeof(strPath), strFilename );

    // Load the mesh
    if( FAILED( hr = D3DXLoadMeshFromX( strPath, D3DXMESH_SYSTEMMEM, pd3dDevice, 
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

    // Get material info for the mesh
    // Get the array of materials out of the buffer
    if( pMtrlBuffer && m_dwNumMaterials > 0 )
    {
        // Allocate memory for the materials and textures
        D3DXMATERIAL* d3dxMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer();
        m_pMaterials = new D3DMATERIAL9[m_dwNumMaterials];
        if( m_pMaterials == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto LEnd;
        }
        m_pTextures  = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];
        if( m_pTextures == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto LEnd;
        }

        // Copy each material and create its texture
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
            // Copy the material
            m_pMaterials[i]         = d3dxMtrls[i].MatD3D;
            m_pTextures[i]          = NULL;

            // Create a texture
            if( d3dxMtrls[i].pTextureFilename )
            {
                TCHAR strTexture[MAX_PATH];
                TCHAR strTextureTemp[MAX_PATH];
                DXUtil_ConvertAnsiStringToGenericCb( strTextureTemp, d3dxMtrls[i].pTextureFilename, sizeof(strTextureTemp) );
                DXUtil_FindMediaFileCb( strTexture, sizeof(strTexture), strTextureTemp );

                if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, strTexture, 
                                                       &m_pTextures[i] ) ) )
                    m_pTextures[i] = NULL;
            }
        }
    }
    hr = S_OK;

LEnd:
    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pMtrlBuffer );

    return hr;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::Create( LPDIRECT3DDEVICE9 pd3dDevice,
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

    // Get material info for the mesh
    // Get the array of materials out of the buffer
    if( pMtrlBuffer && m_dwNumMaterials > 0 )
    {
        // Allocate memory for the materials and textures
        D3DXMATERIAL* d3dxMtrls = (D3DXMATERIAL*)pMtrlBuffer->GetBufferPointer();
        m_pMaterials = new D3DMATERIAL9[m_dwNumMaterials];
        if( m_pMaterials == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto LEnd;
        }
        m_pTextures  = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];
        if( m_pTextures == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto LEnd;
        }

        // Copy each material and create its texture
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
            // Copy the material
            m_pMaterials[i]         = d3dxMtrls[i].MatD3D;
            m_pTextures[i]          = NULL;

            // Create a texture
            if( d3dxMtrls[i].pTextureFilename )
            {
                TCHAR strTexture[MAX_PATH];
                TCHAR strTextureTemp[MAX_PATH];
                DXUtil_ConvertAnsiStringToGenericCb( strTextureTemp, d3dxMtrls[i].pTextureFilename, sizeof(strTextureTemp) );
                DXUtil_FindMediaFileCb( strTexture, sizeof(strTexture), strTextureTemp );

                if( FAILED( D3DXCreateTextureFromFile( pd3dDevice, strTexture, 
                                                       &m_pTextures[i] ) ) )
                    m_pTextures[i] = NULL;
            }
        }
    }
    hr = S_OK;

LEnd:
    SAFE_RELEASE( pAdjacencyBuffer );
    SAFE_RELEASE( pMtrlBuffer );

    return hr;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::SetFVF( LPDIRECT3DDEVICE9 pd3dDevice, DWORD dwFVF )
{
    LPD3DXMESH pTempSysMemMesh = NULL;
    LPD3DXMESH pTempLocalMesh  = NULL;

    if( m_pSysMemMesh )
    {
        if( FAILED( m_pSysMemMesh->CloneMeshFVF( D3DXMESH_SYSTEMMEM, dwFVF,
                                                 pd3dDevice, &pTempSysMemMesh ) ) )
            return E_FAIL;
    }
    if( m_pLocalMesh )
    {
        if( FAILED( m_pLocalMesh->CloneMeshFVF( 0L, dwFVF, pd3dDevice,
                                                &pTempLocalMesh ) ) )
        {
            SAFE_RELEASE( pTempSysMemMesh );
            return E_FAIL;
        }
    }

    SAFE_RELEASE( m_pSysMemMesh );
    SAFE_RELEASE( m_pLocalMesh );

    if( pTempSysMemMesh ) m_pSysMemMesh = pTempSysMemMesh;
    if( pTempLocalMesh )  m_pLocalMesh  = pTempLocalMesh;

    // Compute normals in case the meshes have them
    if( m_pSysMemMesh )
        D3DXComputeNormals( m_pSysMemMesh, NULL );
    if( m_pLocalMesh )
        D3DXComputeNormals( m_pLocalMesh, NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CD3DMesh::SetVertexDecl
// Desc: Convert the mesh to the format specified by the given vertex
//       declarations.
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::SetVertexDecl( LPDIRECT3DDEVICE9 pd3dDevice, D3DVERTEXELEMENT9 *pDecl )
{
    LPD3DXMESH pTempSysMemMesh = NULL;
    LPD3DXMESH pTempLocalMesh  = NULL;

    if( m_pSysMemMesh )
    {
        if( FAILED( m_pSysMemMesh->CloneMesh( D3DXMESH_SYSTEMMEM, pDecl,
                                              pd3dDevice, &pTempSysMemMesh ) ) )
            return E_FAIL;
    }

    if( m_pLocalMesh )
    {
        if( FAILED( m_pLocalMesh->CloneMesh( 0L, pDecl, pd3dDevice,
                                             &pTempLocalMesh ) ) )
        {
            SAFE_RELEASE( pTempSysMemMesh );
            return E_FAIL;
        }
    }

    SAFE_RELEASE( m_pSysMemMesh );
    SAFE_RELEASE( m_pLocalMesh );

    if( pTempSysMemMesh )
    {
        m_pSysMemMesh = pTempSysMemMesh;
        // Compute normals in case the meshes have them
        D3DXComputeNormals( m_pSysMemMesh, NULL );
    }

    if( pTempLocalMesh )
    {
        m_pLocalMesh = pTempLocalMesh;
        // Compute normals in case the meshes have them
        D3DXComputeNormals( m_pLocalMesh, NULL );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice )
{
    if( NULL == m_pSysMemMesh )
        return E_FAIL;

    // Added by LiXzihi: only clone the mesh when it's not been cloned yet
	if(m_pLocalMesh == NULL)
	{
		// Make a local memory version of the mesh. Note: because we are passing in
		// no flags, the default behavior is to clone into local memory.
		if( FAILED( m_pSysMemMesh->CloneMeshFVF( D3DXMESH_MANAGED, m_pSysMemMesh->GetFVF(),
												pd3dDevice, &m_pLocalMesh ) ) )
			return E_FAIL;
	}

    return S_OK;

}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::InvalidateDeviceObjects()
{
    SAFE_RELEASE( m_pLocalMesh );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::Destroy()
{
    InvalidateDeviceObjects();
    for( UINT i=0; i<m_dwNumMaterials; i++ )
        SAFE_RELEASE( m_pTextures[i] );
    SAFE_DELETE_ARRAY( m_pTextures );
    SAFE_DELETE_ARRAY( m_pMaterials );

    SAFE_RELEASE( m_pSysMemMesh );

    m_dwNumMaterials = 0L;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name:
// Desc:
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::Render( LPDIRECT3DDEVICE9 pd3dDevice, bool bDrawOpaqueSubsets,
                          bool bDrawAlphaSubsets )
{
    if( NULL == m_pLocalMesh )
        return E_FAIL;

    // Frist, draw the subsets without alpha
    if( bDrawOpaqueSubsets )
    {
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
            if( m_bUseMaterials )
            {
                if( m_pMaterials[i].Diffuse.a < 1.0f )
                    continue;
                pd3dDevice->SetMaterial( &m_pMaterials[i] );
                pd3dDevice->SetTexture( 0, m_pTextures[i] );
            }
            m_pLocalMesh->DrawSubset( i );
        }
    }

    // Then, draw the subsets with alpha
    if( bDrawAlphaSubsets && m_bUseMaterials )
    {
        for( DWORD i=0; i<m_dwNumMaterials; i++ )
        {
            if( m_pMaterials[i].Diffuse.a == 1.0f )
                continue;

            // Set the material and texture
            pd3dDevice->SetMaterial( &m_pMaterials[i] );
            pd3dDevice->SetTexture( 0, m_pTextures[i] );
            m_pLocalMesh->DrawSubset( i );
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestorePolygonMesh()
// Desc: Initialize device dependent objects.
// Params:
// pd3dDevice
//	[in] Pointer to an IDirect3DDevice9 interface, representing the device associated with the created polygon mesh. 
// Length
//	[in] Length of each side. 
// Sides
//	[in] Number of sides for the polygon. Value must be greater than or equal to 3. 
// ppMesh
//	[out] Address of a pointer to the output shape, an ID3DXMesh interface. 
//-----------------------------------------------------------------------------
HRESULT CD3DMesh::RestorePolygonMesh(LPDIRECT3DDEVICE9 pd3dDevice,FLOAT Length, UINT Sides, LPD3DXMESH *ppMesh)
{
	HRESULT hr;
	// create the floor geometry
    LPD3DXMESH pMesh;
    hr = D3DXCreatePolygon( pd3dDevice, Length, Sides, & pMesh, NULL );
    if( FAILED( hr ) )
        return hr;

    hr = pMesh->CloneMeshFVF( D3DXMESH_SYSTEMMEM, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, pd3dDevice, ppMesh );
    pMesh->Release();
    if( FAILED( hr ) )
        return hr;

    LPD3DXMESH pMeshFloor = *ppMesh;				// rename variable
    
	DWORD dwNumVx = pMeshFloor->GetNumVertices();

    struct Vx
    {
        D3DXVECTOR3 vPos;
        D3DXVECTOR3 vNorm;
        float fTex[ 2 ];
    };

    // Initialize its texture coordinates
	const int FLOOR_TILECOUNT = 2;
	Vx * pVx;
    hr = pMeshFloor->LockVertexBuffer( 0, (VOID **) & pVx );
    if( FAILED( hr ) )
        return hr;

    for( DWORD i = 0; i < dwNumVx; ++ i )
    {
        if( fabs( pVx->vPos.x ) < 0.01 )
        {
            if( pVx->vPos.y > 0 )
            {
                pVx->fTex[ 0 ] = 0.0f;
                pVx->fTex[ 1 ] = 0.0f;
            } else
            if( pVx->vPos.y < 0.0f )
            {
                pVx->fTex[ 0 ] = 1.0f * FLOOR_TILECOUNT;
                pVx->fTex[ 1 ] = 1.0f * FLOOR_TILECOUNT;
            } else
            {
                pVx->fTex[ 0 ] = 0.5f * FLOOR_TILECOUNT;
                pVx->fTex[ 1 ] = 0.5f * FLOOR_TILECOUNT;
            }
        } else
        if( pVx->vPos.x > 0.0f )
        {
            pVx->fTex[ 0 ] = 1.0f * FLOOR_TILECOUNT;
            pVx->fTex[ 1 ] = 0.0f;
        } else
        {
            pVx->fTex[ 0 ] = 0.0f;
            pVx->fTex[ 1 ] = 1.0f * FLOOR_TILECOUNT;
        }

        ++ pVx;
    }

    pMeshFloor->UnlockVertexBuffer();
	return S_OK;
}

//TODO: there seems to be something wrong with this function
FLOAT CD3DMesh::ComputeBoundingSphere(D3DXVECTOR3* vObjectCenter,FLOAT* fObjectRadius )
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
            hr = D3DXComputeBoundingSphere( (D3DXVECTOR3 *)pVertices, 
                                            m_pSysMemMesh->GetNumVertices(),
                                            D3DXGetFVFVertexSize(m_pSysMemMesh->GetFVF()),
                                            vObjectCenter,
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
// pMin:[out] Pointer to a D3DXVECTOR3 structure, describing the returned lower-left 
// corner of the bounding box. See Remarks. 
// pMax:[out] Pointer to a D3DXVECTOR3 structure, describing the returned upper-right 
// corner of the bounding box. See Remarks.
//---------------------------------------------------------------------------------
FLOAT CD3DMesh::ComputeBoundingBox(D3DXVECTOR3 *pMin, D3DXVECTOR3 *pMax )
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
            hr = D3DXComputeBoundingBox( (D3DXVECTOR3 *)pVertices, 
                                            m_pSysMemMesh->GetNumVertices(),
                                            D3DXGetFVFVertexSize(m_pSysMemMesh->GetFVF()),
                                            pMin,
											pMax);
            pMeshVB->Unlock();
        }
        pMeshVB->Release();
    }
    if( FAILED(hr) )
		return 0;   
	return 1;
}