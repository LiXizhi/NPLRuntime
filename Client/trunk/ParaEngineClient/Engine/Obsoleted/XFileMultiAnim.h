#pragma once

namespace ParaEngine
{
class CMultiAnim;
class CAnimInstance;

/**
* Inheriting from ID3DXAllocateHierarchy, this class handles the
*       allocation and release of the memory used by animation frames and
*       meshes.  Applications derive their own version of this class so
*       that they can customize the behavior of allocation and release.
*/
class CMultiAnimAllocateHierarchy : public ID3DXAllocateHierarchy
{
    /// callback to create a D3DXFRAME-derived object and initialize it
    STDMETHOD( CreateFrame )( THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame );
    /// callback to create a D3DXMESHCONTAINER-derived object and initialize it
    STDMETHOD( CreateMeshContainer )( THIS_ LPCSTR Name, CONST D3DXMESHDATA * pMeshData, 
                            CONST D3DXMATERIAL * pMaterials, CONST D3DXEFFECTINSTANCE * pEffectInstances,
                            DWORD NumMaterials, CONST DWORD * pAdjacency, LPD3DXSKININFO pSkinInfo, 
                            LPD3DXMESHCONTAINER * ppNewMeshContainer );
    /// callback to release a D3DXFRAME-derived object
    STDMETHOD( DestroyFrame )( THIS_ LPD3DXFRAME pFrameToFree );
    /// callback to release a D3DXMESHCONTAINER-derived object
    STDMETHOD( DestroyMeshContainer )( THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree );

public:
    CMultiAnimAllocateHierarchy();

    /// Setup method
    STDMETHOD( SetMA )( THIS_ CMultiAnim *pMA );

private:
    CMultiAnim *m_pMA;
};




/** Inherits from D3DXFRAME.  This represents an animation frame, or bone.*/
struct MultiAnimFrame : public D3DXFRAME
{
};




/**
* Desc: Inherits from D3DXMESHCONTAINER.  This represents a mesh object
*       that gets its vertices blended and rendered based on the frame
*       information in its hierarchy.
*/
struct MultiAnimMC : public D3DXMESHCONTAINER
{
    LPDIRECT3DTEXTURE9 *m_apTextures;
    LPD3DXMESH          m_pWorkingMesh;
    Matrix4 *        m_amxBoneOffsets;  // Bone offset matrices retrieved from pSkinInfo
    Matrix4 **       m_apmxBonePointers;  // Provides index to bone matrix lookup

    DWORD               m_dwNumPaletteEntries;
    DWORD               m_dwMaxNumFaceInfls;
    DWORD               m_dwNumAttrGroups;
    LPD3DXBUFFER        m_pBufBoneCombos;

    HRESULT SetupBonePtrs( D3DXFRAME * pFrameRoot );
};




/**
* This class encapsulates a mesh hierarchy (typically loaded from an
*       .X file).  It has a list of CAnimInstance objects that all share
*       the mesh hierarchy here, as well as using a copy of our animation
*       controller.  CMultiAnim loads and keeps an effect object that it
*       renders the meshes with.
*/
class CMultiAnim
{
    friend class CMultiAnimAllocateHierarchy;
    friend class CAnimInstance;
    friend struct MultiAnimFrame;
    friend struct MultiAnimMC;

protected:

    LPDIRECT3DDEVICE9         m_pDevice;

    LPD3DXEFFECT              m_pEffect;
    char *                    m_sTechnique;           // character rendering technique
    bool                      m_bSWVP;                // software vertext processing
    DWORD                     m_dwWorkingPaletteSize;
    Matrix4 *              m_amxWorkingPalette;

    vector< CAnimInstance* >  m_v_pAnimInstances;     // must be at lesat 1; otherwise, clear all

    MultiAnimFrame *          m_pFrameRoot;           // shared between all instances
    LPD3DXANIMATIONCONTROLLER m_pAC;                  // AC that all children clone from -- to clone clean, no keys

    // useful data an app can retrieve
    float                     m_fBoundingRadius;

private:

            HRESULT           CreateInstance( CAnimInstance ** ppAnimInstance );
            HRESULT           SetupBonePtrs( MultiAnimFrame * pFrame );

public:

                              CMultiAnim();
    virtual                   ~CMultiAnim();

    virtual HRESULT           Setup( LPDIRECT3DDEVICE9 pDevice, TCHAR sXFile[], TCHAR sFxFile[], CMultiAnimAllocateHierarchy *pAH, LPD3DXLOADUSERDATA pLUD = NULL );
    virtual HRESULT           Cleanup( CMultiAnimAllocateHierarchy * pAH );

            LPDIRECT3DDEVICE9 GetDevice();
            LPD3DXEFFECT      GetEffect();
            DWORD             GetNumInstances();
            CAnimInstance *   GetInstance( DWORD dwIdx );
            float             GetBoundingRadius();

    virtual HRESULT           CreateNewInstance( DWORD * pdwNewIdx );

    virtual void              SetTechnique( char * sTechnique );
    virtual void              SetSWVP( bool bSWVP );

    virtual HRESULT           Draw();
};




/**
* Encapsulates an animation instance, with its own animation controller.
*/
class CAnimInstance
{
    friend class CMultiAnim;

protected:

    CMultiAnim                *m_pMultiAnim;
    Matrix4                 m_mxWorld;
    LPD3DXANIMATIONCONTROLLER  m_pAC;

private:

    virtual HRESULT     Setup( LPD3DXANIMATIONCONTROLLER pAC );
    virtual void        UpdateFrames( MultiAnimFrame * pFrame, Matrix4 * pmxBase );
    virtual void        DrawFrames( MultiAnimFrame * pFrame );
    virtual void        DrawMeshFrame( MultiAnimFrame * pFrame );

public:

                        CAnimInstance( CMultiAnim * pMultiAnim );
    virtual             ~CAnimInstance();

    virtual void        Cleanup();

            CMultiAnim* GetMultiAnim();
            void        GetAnimController( LPD3DXANIMATIONCONTROLLER * ppAC );

            Matrix4  GetWorldTransform();
            void        SetWorldTransform( const Matrix4 * pmxWorld );

    virtual HRESULT     AdvanceTime( DOUBLE dTimeDelta, ID3DXAnimationCallbackHandler * pCH );
    virtual HRESULT     ResetTime();
    virtual HRESULT     Draw();
};
}