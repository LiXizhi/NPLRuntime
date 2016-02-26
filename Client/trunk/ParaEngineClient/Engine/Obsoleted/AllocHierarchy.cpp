#include "ParaEngine.h"
#include "XFileMultiAnim.h"
#include "memdebug.h"
using namespace ParaEngine;

#ifndef min 
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

//-----------------------------------------------------------------------------
// Name: HeapCopy()
// Desc: Allocate buffer in memory and copy the content of sName to the
//       buffer, then return the address of the buffer.
//-----------------------------------------------------------------------------
CHAR *HeapCopy( CHAR *sName )
{
    DWORD dwLen = (DWORD) strlen( sName );
    CHAR * sNewName = new CHAR[ dwLen + 1 ];
    strncpy( sNewName, sName, dwLen );
    sNewName[ dwLen ] = '\0';
    return sNewName;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::CMultiAnimAllocateHierarchy()
// Desc: Constructor of CMultiAnimAllocateHierarchy
//-----------------------------------------------------------------------------
CMultiAnimAllocateHierarchy::CMultiAnimAllocateHierarchy() :
    m_pMA( NULL )
{
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::SetMA()
// Desc: Sets the member CMultiAnimation pointer.  This is the CMultiAnimation
//       we work with during the callbacks from D3DX.
//-----------------------------------------------------------------------------
HRESULT CMultiAnimAllocateHierarchy::SetMA( THIS_ CMultiAnim *pMA )
{
    m_pMA = pMA;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::CreateFrame()
// Desc: Called by D3DX during the loading of a mesh hierarchy.  The app can
//       customize its behavior.  At a minimum, the app should allocate a
//       D3DXFRAME or a child of it and fill in the Name member.
//-----------------------------------------------------------------------------
HRESULT CMultiAnimAllocateHierarchy::CreateFrame( THIS_ LPCSTR Name, LPD3DXFRAME * ppFrame )
{
    assert( m_pMA );

    * ppFrame = NULL;

    MultiAnimFrame * pFrame = new MultiAnimFrame;
    if( pFrame == NULL )
    {
        DestroyFrame( pFrame );
        return E_OUTOFMEMORY;
    }

    ZeroMemory( pFrame, sizeof( MultiAnimFrame ) );

    if( Name )
        pFrame->Name = (CHAR *) HeapCopy( (CHAR *) Name );
    else
    {
        // TODO: Add a counter to append to the string below
        //       so that we are using a different name for
        //       each bone.
        pFrame->Name = (CHAR *) HeapCopy( "<no_name>" );
    }

    if( pFrame->Name == NULL )
    {
        DestroyFrame( pFrame );
        return E_OUTOFMEMORY;
    }

    * ppFrame = pFrame;
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::CreateMeshContainer()
// Desc: Called by D3DX during the loading of a mesh hierarchy. At a minumum,
//       the app should allocate a D3DXMESHCONTAINER or a child of it and fill
//       in the members based on the parameters here.  The app can further
//       customize the allocation behavior here.  In our case, we initialize
//       m_amxBoneOffsets from the skin info for convenience reason.
//       Then we call ConvertToIndexedBlendedMesh to obtain a new mesh object
//       that's compatible with the palette size we have to work with.
//-----------------------------------------------------------------------------
HRESULT CMultiAnimAllocateHierarchy::CreateMeshContainer( THIS_
    LPCSTR Name,
    CONST D3DXMESHDATA *pMeshData,
    CONST D3DXMATERIAL *pMaterials,
    CONST D3DXEFFECTINSTANCE *pEffectInstances,
    DWORD NumMaterials,
    CONST DWORD *pAdjacency,
    LPD3DXSKININFO pSkinInfo, 
    LPD3DXMESHCONTAINER *ppNewMeshContainer )
{
    assert( m_pMA );

    * ppNewMeshContainer = NULL;

    HRESULT hr = S_OK;

    MultiAnimMC * pMC = new MultiAnimMC;
    if( pMC == NULL )
    { hr = E_OUTOFMEMORY; goto e_Exit; }

    ZeroMemory( pMC, sizeof( MultiAnimMC ) );

    // if this is a static mesh, exit early; we're only interested in skinned meshes
    if( pSkinInfo == NULL )
    {
        hr = S_OK;
        goto e_Exit;
    }

    // only support mesh type
    if( pMeshData->Type != D3DXMESHTYPE_MESH )
    { hr = E_FAIL; goto e_Exit; }

    if( Name )
        pMC->Name = (CHAR *) HeapCopy( (CHAR *) Name );
    else
        pMC->Name = (CHAR *) HeapCopy( "<no_name>" );

    // copy the mesh over
    pMC->MeshData.Type = pMeshData->Type;
    pMC->MeshData.pMesh = pMeshData->pMesh;
    pMC->MeshData.pMesh->AddRef();

    // copy adjacency over
    {
        DWORD dwNumFaces = pMC->MeshData.pMesh->GetNumFaces();
        pMC->pAdjacency = new DWORD[ 3 * dwNumFaces ];
        if( pMC->pAdjacency == NULL )
        { hr = E_OUTOFMEMORY; goto e_Exit; }

        CopyMemory( pMC->pAdjacency, pAdjacency, 3 * sizeof( DWORD ) * dwNumFaces );
    }

    // ignore effects instances
    pMC->pEffects = NULL;

    // alloc and copy materials
    pMC->NumMaterials = max( 1, NumMaterials );
    pMC->pMaterials = new D3DXMATERIAL           [ pMC->NumMaterials ];
    pMC->m_apTextures = new LPDIRECT3DTEXTURE9   [ pMC->NumMaterials ];
    if( pMC->pMaterials == NULL || pMC->m_apTextures == NULL )
    { hr = E_OUTOFMEMORY; goto e_Exit; }

    if( NumMaterials > 0 )
    {
        CopyMemory( pMC->pMaterials, pMaterials, NumMaterials * sizeof( D3DXMATERIAL ) );
        for( DWORD i = 0; i < NumMaterials; ++ i )
        {
            if( pMC->pMaterials[ i ].pTextureFilename )
            {
                // CALLBACK to get valid filename
                TCHAR sNewPath[ MAX_PATH ];
                TCHAR tszTexName[ MAX_PATH ];
                if( FAILED( DXUtil_ConvertAnsiStringToGenericCch( tszTexName,
                    pMC->pMaterials[ i ].pTextureFilename,
                    MAX_PATH ) ) )
                    pMC->m_apTextures[ i ] = NULL;
                else
                    if( SUCCEEDED( DXUtil_FindMediaFileCch( sNewPath, MAX_PATH, tszTexName ) ) )
                    {
                        // create the D3D texture
                        if( FAILED( D3DXCreateTextureFromFile( m_pMA->m_pDevice,
                            sNewPath,
                            &pMC->m_apTextures[ i ] ) ) )
                            pMC->m_apTextures[ i ] = NULL;
                    }
                    else
                        pMC->m_apTextures[ i ] = NULL;
            }
            else
                pMC->m_apTextures[ i ] = NULL;
        }
    }
    else    // mock up a default material and set it
    {
        ZeroMemory( & pMC->pMaterials[ 0 ].MatD3D, sizeof( ParaMaterial ) );
        pMC->pMaterials[ 0 ].MatD3D.Diffuse.r = 0.5f;
        pMC->pMaterials[ 0 ].MatD3D.Diffuse.g = 0.5f;
        pMC->pMaterials[ 0 ].MatD3D.Diffuse.b = 0.5f;
        pMC->pMaterials[ 0 ].MatD3D.Specular = pMC->pMaterials[ 0 ].MatD3D.Diffuse;
        pMC->pMaterials[ 0 ].pTextureFilename = NULL;
    }

    // save the skininfo object
    pMC->pSkinInfo = pSkinInfo;
    pSkinInfo->AddRef();

    // Get the bone offset matrices from the skin info
    pMC->m_amxBoneOffsets = new Matrix4[ pSkinInfo->GetNumBones() ];
    if( pMC->m_amxBoneOffsets == NULL )
    { hr = E_OUTOFMEMORY; goto e_Exit; }
    {
        for( DWORD i = 0; i < pSkinInfo->GetNumBones(); ++ i )
            pMC->m_amxBoneOffsets[ i ] = * (Matrix4 *) pSkinInfo->GetBoneOffsetMatrix( i );
    }

    //
    // Determine the palette size we need to work with, then call ConvertToIndexedBlendedMesh
    // to set up a new mesh that is compatible with the palette size.
    //
    {
        UINT iPaletteSize = 0;
        m_pMA->m_pEffect->GetInt( "MATRIX_PALETTE_SIZE", (INT *) & iPaletteSize );
        pMC->m_dwNumPaletteEntries = min( iPaletteSize, pMC->pSkinInfo->GetNumBones() );
    }

    // generate the skinned mesh - creates a mesh with blend weights and indices
    hr = pMC->pSkinInfo->ConvertToIndexedBlendedMesh( pMC->MeshData.pMesh,
        D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
        pMC->m_dwNumPaletteEntries,
        pMC->pAdjacency,
        NULL,
        NULL,
        NULL,
        &pMC->m_dwMaxNumFaceInfls,
        &pMC->m_dwNumAttrGroups,
        &pMC->m_pBufBoneCombos,
        &pMC->m_pWorkingMesh );
    if( FAILED( hr ) )
        goto e_Exit;

    // Make sure the working set is large enough for this mesh.
    // This is a bone array used for all mesh containers as a working
    // set during drawing.  If one was created previously that isn't 
    // large enough for this mesh, we have to reallocate.
    if( m_pMA->m_dwWorkingPaletteSize < pMC->m_dwNumPaletteEntries )
    {
        if( m_pMA->m_amxWorkingPalette )
            delete [] m_pMA->m_amxWorkingPalette;

        m_pMA->m_dwWorkingPaletteSize = pMC->m_dwNumPaletteEntries;
        m_pMA->m_amxWorkingPalette = new Matrix4[ m_pMA->m_dwWorkingPaletteSize ];
        if( m_pMA->m_amxWorkingPalette == NULL )
        {
            m_pMA->m_dwWorkingPaletteSize = 0;
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }
    }

    // ensure the proper vertex format for the mesh
    {
        DWORD dwOldFVF = pMC->m_pWorkingMesh->GetFVF();
        DWORD dwNewFVF = ( dwOldFVF & D3DFVF_POSITION_MASK ) | D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4;
        if( dwNewFVF != dwOldFVF )
        {
            LPD3DXMESH pMesh;
            hr = pMC->m_pWorkingMesh->CloneMeshFVF( pMC->m_pWorkingMesh->GetOptions(),
                dwNewFVF,
                m_pMA->m_pDevice,
                &pMesh );
            if( FAILED( hr ) )
                goto e_Exit;

            pMC->m_pWorkingMesh->Release();
            pMC->m_pWorkingMesh = pMesh;

            // if the loaded mesh didn't contain normals, compute them here
            if( ! ( dwOldFVF & D3DFVF_NORMAL ) )
            {
                hr = D3DXComputeNormals( pMC->m_pWorkingMesh, NULL );
                if( FAILED( hr ) )
                    goto e_Exit;
            }
        }
    }

    // Interpret the UBYTE4 as a Color.
    // The GeForce3 doesn't support the UBYTE4 decl type.  So, we convert any
    // blend indices to a Color semantic, and later in the shader convert
    // it back using the D3DCOLORtoUBYTE4() intrinsic.  Note that we don't
    // convert any data, just the declaration.
    D3DVERTEXELEMENT9 pDecl[ MAX_FVF_DECL_SIZE ];
    D3DVERTEXELEMENT9 * pDeclCur;
    hr = pMC->m_pWorkingMesh->GetDeclaration( pDecl );
    if( FAILED( hr ) )
        goto e_Exit;

    pDeclCur = pDecl;
    while( pDeclCur->Stream != 0xff )
    {
        if( ( pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES ) && ( pDeclCur->UsageIndex == 0 ) )
            pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
        pDeclCur++;
    }

    hr = pMC->m_pWorkingMesh->UpdateSemantics( pDecl );
    if( FAILED( hr ) )
        goto e_Exit;

e_Exit:

    if( FAILED( hr ) )
    {
        if( pMC )
            DestroyMeshContainer( pMC );
    }
    else
        * ppNewMeshContainer = pMC;

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::DestroyFrame()
// Desc: Called by D3DX during the release of a mesh hierarchy.  Here we should
//       free all resources allocated in CreateFrame().
//-----------------------------------------------------------------------------
HRESULT CMultiAnimAllocateHierarchy::DestroyFrame( THIS_ LPD3DXFRAME pFrameToFree )
{
    assert( m_pMA );

    MultiAnimFrame * pFrame = (MultiAnimFrame *) pFrameToFree;

    if( pFrame->Name )
        delete [] pFrame->Name;

    delete pFrame;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnimAllocateHierarchy::DestroyMeshContainer()
// Desc: Called by D3DX during the release of a mesh hierarchy.  Here we should
//       free all resources allocated in CreateMeshContainer().
//-----------------------------------------------------------------------------
HRESULT CMultiAnimAllocateHierarchy::DestroyMeshContainer( THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree )
{
    assert( m_pMA );

    MultiAnimMC * pMC = (MultiAnimMC *) pMeshContainerToFree;

    if( pMC->Name )
        delete [] pMC->Name;

    if( pMC->MeshData.pMesh )
        pMC->MeshData.pMesh->Release();

    if( pMC->pAdjacency )
        delete [] pMC->pAdjacency;

    if( pMC->pMaterials )
        delete [] pMC->pMaterials;

    for( DWORD i = 0; i < pMC->NumMaterials; ++ i )
    {
        if( pMC->m_apTextures[ i ] )
            pMC->m_apTextures[ i ]->Release();
    }

    if( pMC->m_apTextures )
        delete [] pMC->m_apTextures;

    if( pMC->pSkinInfo )
        pMC->pSkinInfo->Release();

    if( pMC->m_amxBoneOffsets )
        delete [] pMC->m_amxBoneOffsets;

    if( pMC->m_pWorkingMesh )
    {
        pMC->m_pWorkingMesh->Release();
        pMC->m_pWorkingMesh = NULL;
    }

    pMC->m_dwNumPaletteEntries = 0;
    pMC->m_dwMaxNumFaceInfls = 0;
    pMC->m_dwNumAttrGroups = 0;

    if( pMC->m_pBufBoneCombos )
    {
        pMC->m_pBufBoneCombos->Release();
        pMC->m_pBufBoneCombos = NULL;
    }

    if( pMC->m_apmxBonePointers )
        delete [] pMC->m_apmxBonePointers;

    delete pMeshContainerToFree;

    return S_OK;
}
