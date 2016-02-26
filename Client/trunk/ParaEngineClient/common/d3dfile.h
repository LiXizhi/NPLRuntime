//-----------------------------------------------------------------------------
// File: D3DFile.h
//
// Desc: Support code for loading DirectX .X files.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef D3DFILE_H
#define D3DFILE_H
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>





//-----------------------------------------------------------------------------
// Name: class CD3DMesh
// Desc: Class for loading and rendering file-based meshes
//-----------------------------------------------------------------------------
class CD3DMesh
{
public:
    LPD3DXMESH          m_pSysMemMesh;    // SysMem mesh, lives through resize
    LPD3DXMESH          m_pLocalMesh;     // Local mesh, rebuilt on resize
    
    DWORD               m_dwNumMaterials; // Materials for the mesh
    D3DMATERIAL9*       m_pMaterials;
    LPDIRECT3DTEXTURE9* m_pTextures;
    bool                m_bUseMaterials;  // whether to use materials and textures contained in the model file, when rendering
										  // Note, materials are always loaded if they are present in the file.
protected:
	//-- build-in object creation: add by LiXizhi
	HRESULT RestorePolygonMesh(LPDIRECT3DDEVICE9 pd3dDevice,FLOAT Length, UINT Sides, LPD3DXMESH *ppMesh);

public:
    // Rendering
    HRESULT Render( LPDIRECT3DDEVICE9 pd3dDevice, 
                    bool bDrawOpaqueSubsets = true,
                    bool bDrawAlphaSubsets = true );

    // Mesh access
    LPD3DXMESH GetSysMemMesh() { return m_pSysMemMesh; }
    LPD3DXMESH GetLocalMesh()  { return m_pLocalMesh; }

    // Rendering options
    void    UseMeshMaterials( bool bFlag ) { m_bUseMaterials = bFlag; }
    HRESULT SetFVF( LPDIRECT3DDEVICE9 pd3dDevice, DWORD dwFVF );
    HRESULT SetVertexDecl( LPDIRECT3DDEVICE9 pd3dDevice, D3DVERTEXELEMENT9 *pDecl );

    // Initializing
    HRESULT RestoreDeviceObjects( LPDIRECT3DDEVICE9 pd3dDevice );
    HRESULT InvalidateDeviceObjects();

    // Creation/destruction
    HRESULT Create( LPDIRECT3DDEVICE9 pd3dDevice, LPCTSTR strFilename );
    HRESULT Create( LPDIRECT3DDEVICE9 pd3dDevice, LPD3DXFILEDATA pFileData );
    HRESULT Destroy();

	// --Added by LiXizhi
	// -- Get Bounding sphere
	FLOAT ComputeBoundingSphere(D3DXVECTOR3* vObjectCenter,FLOAT* fObjectRadius );
	FLOAT ComputeBoundingBox(D3DXVECTOR3 *pMin, D3DXVECTOR3 *pMax );

    CD3DMesh();
    virtual ~CD3DMesh();
};

#endif



