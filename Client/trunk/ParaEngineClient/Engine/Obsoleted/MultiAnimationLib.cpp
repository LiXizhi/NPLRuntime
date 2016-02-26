//-----------------------------------------------------------------------------
// File: MultiAnimationLib.cpp
//
// Desc: Implementation of the CMultiAnim class. This class manages the animation
//       data (frames and meshes) obtained from a single X file.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "XFileMultiAnim.h"
#include "memdebug.h"
using namespace ParaEngine;



//-----------------------------------------------------------------------------
// Name: MultiAnimMC::SetupBonePtrs()
// Desc: Initialize the m_apmxBonePointers member to point to the bone matrices
//       so that we can access the bones by index easily.  Called from
//       CMultiAnim::SetupBonePtrs().
//-----------------------------------------------------------------------------
HRESULT MultiAnimMC::SetupBonePtrs( D3DXFRAME * pFrameRoot )
{
    if( pSkinInfo )
    {
        if( m_apmxBonePointers )
            delete [] m_apmxBonePointers;

        DWORD dwNumBones = pSkinInfo->GetNumBones();

        m_apmxBonePointers = new Matrix4* [ dwNumBones ];
        if( m_apmxBonePointers == NULL )
            return E_OUTOFMEMORY;

        for( DWORD i = 0; i < dwNumBones; ++ i )
        {
            MultiAnimFrame *pFrame = (MultiAnimFrame *) D3DXFrameFind( pFrameRoot, pSkinInfo->GetBoneName( i ) );
            if( pFrame == NULL )
                return E_FAIL;

            m_apmxBonePointers[ i ] = (Matrix4*)(&pFrame->TransformationMatrix);
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::CreateInstance()
// Desc: Create a new animation instance based on our animation frames and
//       animation controller.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::CreateInstance( CAnimInstance ** ppAnimInstance )
{
    * ppAnimInstance = NULL;

    LPD3DXANIMATIONCONTROLLER pNewAC = NULL;
    HRESULT hr;
    CAnimInstance * pAI = NULL;

    // Clone the original AC.  This clone is what we will use to animate
    // this mesh; the original never gets used except to clone, since we
    // always need to be able to add another instance at any time.
    hr = m_pAC->CloneAnimationController( m_pAC->GetMaxNumAnimationOutputs(),
                                          m_pAC->GetMaxNumAnimationSets(),
                                          m_pAC->GetMaxNumTracks(),
                                          m_pAC->GetMaxNumEvents(),
                                          &pNewAC );
    if( SUCCEEDED( hr ) )
    {
        // create the new AI
        pAI = new CAnimInstance( this );
        if( pAI == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto e_Exit;
        }

        // set it up
        hr = pAI->Setup( pNewAC );
        if( FAILED( hr ) )
            goto e_Exit;

        * ppAnimInstance = pAI;
    }

e_Exit:

    if( FAILED( hr ) )
    {
        if( pAI )
            delete pAI;

        if( pNewAC )
            pNewAC->Release();
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::SetupBonePtrs()
// Desc: Recursively initialize the bone pointers for all the mesh
//       containers in the hierarchy.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::SetupBonePtrs( MultiAnimFrame * pFrame )
{
    assert( pFrame != NULL );

    HRESULT hr;

    if( pFrame->pMeshContainer )
    {
        // call setup routine
        hr = ( (MultiAnimMC *) pFrame->pMeshContainer )->SetupBonePtrs( m_pFrameRoot );
        if(FAILED( hr ) )
            return hr;
    }
    
    if( pFrame->pFrameSibling )
    {
        // recursive call
        hr = SetupBonePtrs( (MultiAnimFrame *) pFrame->pFrameSibling );
        if(FAILED( hr ) )
            return hr;
    }
    
    if( pFrame->pFrameFirstChild )
    {
        // recursive call
        hr = SetupBonePtrs( (MultiAnimFrame *) pFrame->pFrameFirstChild );
        if(FAILED( hr ) )
            return hr;
    }

    return S_OK;
}

CMultiAnim::CMultiAnim() :
    m_pDevice( NULL ),
    m_pEffect( NULL ),
    m_dwWorkingPaletteSize( 0 ),
    m_amxWorkingPalette( NULL ),
    m_pFrameRoot( NULL ),
    m_pAC( NULL )
{
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::~CMultiAnim()
// Desc: Destructor for CMultiAnim
//-----------------------------------------------------------------------------
CMultiAnim::~CMultiAnim()
{
    vector< CAnimInstance* >::iterator itCur, itEnd = m_v_pAnimInstances.end();
    for( itCur = m_v_pAnimInstances.begin(); itCur != itEnd; ++ itCur )
    {
        ( * itCur )->Cleanup();
        delete * itCur;
    }

    m_v_pAnimInstances.clear();
}



//-----------------------------------------------------------------------------
// Name: CMultiAnim::Setup()
// Desc: The class is initialized with this method.
//       We create the effect from the fx file, and load the animation mesh
//       from the given X file.  We then call SetupBonePtrs() to initialize
//       the mesh containers to enable bone matrix lookup by index.  The
//       Allocation Hierarchy is passed by pointer to allow an app to subclass
//       it for its own implementation.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::Setup( LPDIRECT3DDEVICE9 pDevice,
                           TCHAR sXFile[],
                           TCHAR sFxFile[],
                           CMultiAnimAllocateHierarchy *pAH,
                           LPD3DXLOADUSERDATA pLUD )
{
    assert( pDevice != NULL );
    assert( sXFile );
    assert( sFxFile );
    assert( pAH );

    // set the MA instance for CMultiAnimAllocateHierarchy
    pAH->SetMA( this );

    // set the device
    m_pDevice = pDevice;
    m_pDevice->AddRef();

    HRESULT hr;
    Vector3 vCenter;
    LPD3DXEFFECTCOMPILER pEC = NULL;

    // Increase the palette size if the shader allows it. We are sort
    // of cheating here since we know tiny has 35 bones. The alternative
    // is use the maximum number that vs_2_0 allows.
    D3DXMACRO mac[2] =
    {
        { "MATRIX_PALETTE_SIZE_DEFAULT", "35" },
        { NULL,                          NULL }
    };

    // If we support VS_2_0, increase the palette size; else, use the default
    // of 26 bones from the .fx file by passing NULL
    D3DCAPS9 caps;
    D3DXMACRO *pmac = NULL;
    m_pDevice->GetDeviceCaps( & caps );
    if( caps.VertexShaderVersion > D3DVS_VERSION( 1, 1 ) )
        pmac = mac;

    // create effect -- do this first, so LMHFX has access to the palette size
    TCHAR tszPath[ MAX_PATH ];
    hr = DXUtil_FindMediaFileCch( tszPath, MAX_PATH, sFxFile );
    if( FAILED( hr ) )
        goto e_Exit;

    // Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the shader debugger.  
    // Debugging vertex shaders requires either REF or software vertex processing, and debugging 
    // pixel shaders requires REF.  The D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug 
    // experience in the shader debugger.  It enables source level debugging, prevents instruction 
    // reordering, prevents dead code elimination, and forces the compiler to compile against the next 
    // higher available software target, which ensures that the unoptimized shaders do not exceed 
    // the shader model limitations.  Setting these flags will cause slower rendering since the shaders 
    // will be unoptimized and forced into software.  See the DirectX documentation for more information 
    // about using the shader debugger.
    {
        DWORD dwShaderFlags = 0;
        #ifdef DEBUG_VS
            dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
        #endif
        #ifdef DEBUG_PS
            dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
        #endif

        hr = D3DXCreateEffectFromFile( m_pDevice,
                                       tszPath,
                                       pmac,
                                       NULL,
                                       dwShaderFlags,
                                       NULL,
                                       &m_pEffect,
                                       NULL );

        if( FAILED( hr ) )
            goto e_Exit;
    }

    // create the mesh, frame hierarchy, and animation controller from the x file
    hr = DXUtil_FindMediaFileCch( tszPath, MAX_PATH, sXFile );
    if( FAILED( hr ) )
        goto e_Exit;

    hr = D3DXLoadMeshHierarchyFromX( tszPath,
                                     0,
                                     m_pDevice,
                                     pAH,
                                     pLUD,
                                     (LPD3DXFRAME *) &m_pFrameRoot,
                                     &m_pAC );
    if( FAILED( hr ) )
        goto e_Exit;

    if( !m_pAC )
    {
        hr = E_FAIL;
        MessageBox( NULL,
                    _T("The sample is attempting to load a mesh without animation or incompatible animation.  This sample requires tiny_4anim.x or a mesh with identical animation sets.  The program will now exit."),
                    _T("Mesh Load Error"), MB_OK );
        goto e_Exit;
    }

    // set up bone pointers
    hr = SetupBonePtrs( m_pFrameRoot );
    if( FAILED( hr ) )
        goto e_Exit;

    // get bounding radius
    hr = D3DXFrameCalculateBoundingSphere( m_pFrameRoot, (D3DXVECTOR3*)& vCenter, & m_fBoundingRadius );
    if( FAILED( hr ) )
        goto e_Exit;

    // If there are existing instances, update their animation controllers.
    {
        vector< CAnimInstance* >::iterator itCur, itEnd = m_v_pAnimInstances.end();
        for( itCur = m_v_pAnimInstances.begin(); itCur != itEnd; ++ itCur )
        {
            LPD3DXANIMATIONCONTROLLER pNewAC = NULL;
            hr = m_pAC->CloneAnimationController( m_pAC->GetMaxNumAnimationOutputs(),
                                                  m_pAC->GetMaxNumAnimationSets(),
                                                  m_pAC->GetMaxNumTracks(),
                                                  m_pAC->GetMaxNumEvents(),
                                                  &pNewAC );
            // Release existing animation controller
            if( ( * itCur )->m_pAC )
                ( * itCur )->m_pAC->Release();
            ( * itCur )->Setup( pNewAC );
        }
    }


e_Exit:

    if( FAILED( hr ) )
    {
        if( m_amxWorkingPalette )
        {
            delete [] m_amxWorkingPalette;
            m_amxWorkingPalette = NULL;
            m_dwWorkingPaletteSize = 0;
        }

        if( m_pAC )
        {
            m_pAC->Release();
            m_pAC = NULL;
        }

        if( m_pFrameRoot )
        {
            D3DXFrameDestroy( m_pFrameRoot, pAH );
            m_pFrameRoot = NULL;
        }

        if( m_pEffect )
        {
            m_pEffect->Release();
            m_pEffect = NULL;
        }

        if( pEC )
            pEC->Release();

        m_pDevice->Release();
        m_pDevice = NULL;
    }

    return hr;
}



//-----------------------------------------------------------------------------
// Name: CMultiAnim::Cleanup()
// Desc: Performs clean up work and free up memory.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::Cleanup( CMultiAnimAllocateHierarchy * pAH )
{
    if( m_amxWorkingPalette )
    {
        delete [] m_amxWorkingPalette;
        m_amxWorkingPalette = NULL;
        m_dwWorkingPaletteSize = 0;
    }

    if( m_pAC )
    {
        m_pAC->Release();
        m_pAC = NULL;
    }

    if( m_pFrameRoot )
    {
        D3DXFrameDestroy( m_pFrameRoot, pAH );
        m_pFrameRoot = NULL;
    }

    if( m_pEffect )
    {
        m_pEffect->Release();
        m_pEffect = NULL;
    }

    if( m_pDevice )
    {
        m_pDevice->Release();
        m_pDevice = NULL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::GetDevice()
// Desc: Returns the D3D device we work with.  The caller must call Release()
//       on the pointer when done with it.
//-----------------------------------------------------------------------------
LPDIRECT3DDEVICE9 CMultiAnim::GetDevice()
{
    m_pDevice->AddRef();
    return m_pDevice;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::GetEffect()
// Desc: Returns the D3D effect object that the mesh is rendered with.  The
//       caller must call Release() when done.
//-----------------------------------------------------------------------------
LPD3DXEFFECT CMultiAnim::GetEffect()
{
    if( m_pEffect )
        m_pEffect->AddRef();

    return m_pEffect;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::GetNumInstance()
// Desc: Returns the number of animation instances using our animation frames.
//-----------------------------------------------------------------------------
DWORD CMultiAnim::GetNumInstances()
{
    return (DWORD) m_v_pAnimInstances.size();
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::GetInstance()
// Desc: Returns a CAnimInstance object by index.
//-----------------------------------------------------------------------------
CAnimInstance * CMultiAnim::GetInstance( DWORD dwIndex )
{
    assert( dwIndex < m_v_pAnimInstances.size() );
    return m_v_pAnimInstances[ dwIndex ];
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::GetBoundingRadius()
// Desc: Returns the bounding radius for the mesh object.
//-----------------------------------------------------------------------------
float CMultiAnim::GetBoundingRadius()
{
    return m_fBoundingRadius;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::CreateNewInstance()
// Desc: Creates a new animation instance and adds it to our instance array.
//       Then returns the index of the newly created instance.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::CreateNewInstance( DWORD * pdwNewIdx )
{
    // create the AI
    CAnimInstance * pAI;
    HRESULT hr = CreateInstance( & pAI );
    if( FAILED( hr ) )
        goto e_Exit;

    // add it
    try
    {
        m_v_pAnimInstances.push_back( pAI );
    }
    catch( ... )
    {
        hr = E_OUTOFMEMORY;
        goto e_Exit;
    }

    * pdwNewIdx = (DWORD) m_v_pAnimInstances.size() - 1;

e_Exit:

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::SetTechnique()
// Desc: Sets the name of the technique to render the mesh in.
//-----------------------------------------------------------------------------
void CMultiAnim::SetTechnique( char * sTechnique )
{
    m_sTechnique = sTechnique;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::SetSWVP()
// Desc: Sets the vertex processing type
//-----------------------------------------------------------------------------
void CMultiAnim::SetSWVP( bool bSWVP )
{
    m_bSWVP = bSWVP;
}




//-----------------------------------------------------------------------------
// Name: CMultiAnim::Draw()
// Desc: Render all animtion instances using our mesh frames.
//-----------------------------------------------------------------------------
HRESULT CMultiAnim::Draw()
{
    // TODO: modify this for much faster bulk rendering

    HRESULT hr = S_OK, hrT;

    vector< CAnimInstance* >::iterator itCur, itEnd = m_v_pAnimInstances.end();
    for( itCur = m_v_pAnimInstances.begin(); itCur != itEnd; ++ itCur )
    {
        if( FAILED( hrT = ( * itCur )->Draw() ) )
            hr = hrT;
    }

    return hr;
}
