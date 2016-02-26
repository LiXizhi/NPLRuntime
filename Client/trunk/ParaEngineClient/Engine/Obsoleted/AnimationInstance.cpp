#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "XFileMultiAnim.h"

#include "memdebug.h"

using namespace ParaEngine;

//-----------------------------------------------------------------------------
// Name: CAnimInstance::Setup()
/// Desc: Initialize ourselves to use the animation controller passed in. Then
///       initialize the animation controller.
//-----------------------------------------------------------------------------
HRESULT CAnimInstance::Setup( LPD3DXANIMATIONCONTROLLER pAC )
{
    PE_ASSERT( pAC != NULL );

    DWORD i, dwTracks;

    m_pAC = pAC;

    // Start with all tracks disabled
    dwTracks = m_pAC->GetMaxNumTracks();
    for( i = 0; i < dwTracks; ++ i )
        m_pAC->SetTrackEnable( i, FALSE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::UpdateFrame()
/// Desc: Recursively walk the animation frame hierarchy and for each frame,
///       transform the frame by its parent, starting with a world transform to
///       place the mesh in world space.  This has the effect of a hierarchical
///       transform over all the frames.
//-----------------------------------------------------------------------------
void CAnimInstance::UpdateFrames( MultiAnimFrame * pFrame, Matrix4 * pmxBase )
{
    PE_ASSERT( pFrame != NULL );
    PE_ASSERT( pmxBase != NULL );

    // transform the bone
	ParaMatrixMultiply((Matrix4*)&pFrame->TransformationMatrix, (Matrix4*)&pFrame->TransformationMatrix, pmxBase);

    // transform siblings by the same matrix
    if( pFrame->pFrameSibling )
        UpdateFrames( (MultiAnimFrame *) pFrame->pFrameSibling, pmxBase );

    // transform children by the transformed matrix - hierarchical transformation
    if( pFrame->pFrameFirstChild )
		UpdateFrames((MultiAnimFrame *)pFrame->pFrameFirstChild, (Matrix4*)&pFrame->TransformationMatrix);
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::DrawFrames()
/// Desc: Recursively walk the frame hierarchy and draw each mesh container as
///       we find it.
//-----------------------------------------------------------------------------
void CAnimInstance::DrawFrames( MultiAnimFrame * pFrame )
{
    if( pFrame->pMeshContainer )
        DrawMeshFrame( pFrame );

    if( pFrame->pFrameSibling )
        DrawFrames( (MultiAnimFrame *) pFrame->pFrameSibling );

    if( pFrame->pFrameFirstChild )
        DrawFrames( (MultiAnimFrame *) pFrame->pFrameFirstChild );
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::DrawMeshFrame()
/// Desc: Renders a mesh container.  Here we go through each attribute group
///       and set up the matrix palette for each group by multiplying the
///       bone offsets to its bone transformation.  This gives us the completed
///       bone matrices that can be used and blended by the pipeline.  We then
///       set up the effect and render the mesh.
//-----------------------------------------------------------------------------
void CAnimInstance::DrawMeshFrame( MultiAnimFrame * pFrame )
{
    MultiAnimMC * pMC = (MultiAnimMC *) pFrame->pMeshContainer;
    Matrix4 mx;
    LPDIRECT3DDEVICE9 pDevice = m_pMultiAnim->m_pDevice;

    if( pMC->pSkinInfo == NULL )
        return;

    // get bone combinations
    LPD3DXBONECOMBINATION pBC = ( LPD3DXBONECOMBINATION )( pMC->m_pBufBoneCombos->GetBufferPointer() );
    DWORD dwAttrib, dwPalEntry;

    // for each palette
    for( dwAttrib = 0; dwAttrib < pMC->m_dwNumAttrGroups; ++ dwAttrib )
    {
        // set each transform into the palette
        for( dwPalEntry = 0; dwPalEntry < pMC->m_dwNumPaletteEntries; ++ dwPalEntry )
        {
            DWORD dwMatrixIndex = pBC[ dwAttrib ].BoneId[ dwPalEntry ];
            if( dwMatrixIndex != UINT_MAX )
                ParaMatrixMultiply( &m_pMultiAnim->m_amxWorkingPalette[ dwPalEntry ],
                                    &( pMC->m_amxBoneOffsets[ dwMatrixIndex ] ),
                                    pMC->m_apmxBonePointers[ dwMatrixIndex ] );
        }

        // set the matrix palette into the effect
        m_pMultiAnim->m_pEffect->SetMatrixArray( "amPalette",
                                                 m_pMultiAnim->m_amxWorkingPalette->GetPointer(),
                                                 pMC->m_dwNumPaletteEntries );

        // we're pretty much ignoring the materials we got from the x-file; just set
        // the texture here
        pDevice->SetTexture( 0, pMC->m_apTextures[ pBC[ dwAttrib ].AttribId ] );

        // set the current number of bones; this tells the effect which shader to use
        m_pMultiAnim->m_pEffect->SetInt( "CurNumBones", pMC->m_dwMaxNumFaceInfls - 1 );

        // set the technique we use to draw
        if( FAILED( m_pMultiAnim->m_pEffect->SetTechnique( m_pMultiAnim->m_sTechnique ) ) )
            return;

        UINT uiPasses, uiPass;

        // if we need to do this in software, set it
        if( m_pMultiAnim->m_bSWVP )
            m_pMultiAnim->m_pDevice->SetSoftwareVertexProcessing( TRUE );

        // run through each pass and draw
        m_pMultiAnim->m_pEffect->Begin( & uiPasses, D3DXFX_DONOTSAVESTATE );
        for( uiPass = 0; uiPass < uiPasses; ++ uiPass )
        {
            m_pMultiAnim->m_pEffect->BeginPass( uiPass );
            pMC->m_pWorkingMesh->DrawSubset( dwAttrib );
			m_pMultiAnim->m_pEffect->EndPass();
        }
        m_pMultiAnim->m_pEffect->End();

        if( m_pMultiAnim->m_bSWVP )
            m_pMultiAnim->m_pDevice->SetSoftwareVertexProcessing( FALSE );
    }
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::CAnimInstance()
// Desc: Constructor of CAnimInstance
//-----------------------------------------------------------------------------
CAnimInstance::CAnimInstance( CMultiAnim * pMultiAnim )
:   m_pMultiAnim( pMultiAnim ),
    m_pAC( NULL )
{
    PE_ASSERT( pMultiAnim != NULL );
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::~CAnimInstance()
// Desc: Destructor of CAnimInstance
//-----------------------------------------------------------------------------
CAnimInstance::~CAnimInstance()
{
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::Cleanup()
// Desc: Performs the cleanup task
//-----------------------------------------------------------------------------
void CAnimInstance::Cleanup()
{
    if( m_pAC )
        m_pAC->Release();
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::GetMultiAnim()
// Desc: Returns the CMultiAnimation object that this instance uses.
//-----------------------------------------------------------------------------
CMultiAnim * CAnimInstance::GetMultiAnim()
{
    return m_pMultiAnim;
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::GetAnimController()
// Desc: Returns the animation controller for this instance.  The caller
//       must call Release() when done with it.
//-----------------------------------------------------------------------------
void CAnimInstance::GetAnimController( LPD3DXANIMATIONCONTROLLER * ppAC )
{
    PE_ASSERT( ppAC != NULL );
    m_pAC->AddRef();
    * ppAC = m_pAC;
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::GetWorldTransform()
// Desc: Returns the world transformation matrix for this animation instance.
//-----------------------------------------------------------------------------
Matrix4 CAnimInstance::GetWorldTransform()
{
    return m_mxWorld;
}



//-----------------------------------------------------------------------------
// Name: CAnimInstance::SetWorldTransform()
// Desc: Sets the world transformation matrix for this instance.
//-----------------------------------------------------------------------------
void CAnimInstance::SetWorldTransform( const Matrix4 * pmxWorld )
{
    PE_ASSERT( pmxWorld != NULL );
    m_mxWorld = * pmxWorld;
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::AdvanceTime()
// Desc: Advance the local time of this instance by dTimeDelta with a
//       callback handler to handle possible callback from the animation
//       controller.  Then propagate the animations down the hierarchy while
//       transforming it into world space.
//-----------------------------------------------------------------------------
HRESULT CAnimInstance::AdvanceTime( DOUBLE dTimeDelta, ID3DXAnimationCallbackHandler * pCH )
{
    HRESULT hr;

    // apply all the animations to the bones in the frame hierarchy.
    hr = m_pAC->AdvanceTime( dTimeDelta, pCH );
    if( FAILED( hr ) )
        return hr;

    // apply the animations recursively through the hierarchy, and set the world.
    UpdateFrames( m_pMultiAnim->m_pFrameRoot, & m_mxWorld );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::ResetTime()
// Desc: Resets the local time for this instance.
//-----------------------------------------------------------------------------
HRESULT CAnimInstance::ResetTime()
{
    return m_pAC->ResetTime();
}




//-----------------------------------------------------------------------------
// Name: CAnimInstance::Draw()
// Desc: Renders the frame hierarchy of our CMultiAnimation object.  This is
//       normally called right after AdvanceTime() so that we render the
//       mesh with the animation for this instance.
//-----------------------------------------------------------------------------
HRESULT CAnimInstance::Draw()
{
    DrawFrames( m_pMultiAnim->m_pFrameRoot );

    return S_OK;
}
#endif