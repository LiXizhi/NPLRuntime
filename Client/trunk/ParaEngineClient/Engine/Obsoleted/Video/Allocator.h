//------------------------------------------------------------------------------
// File: Allocator.h
//
// Desc: DirectShow sample code - interface for the CAllocator class
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_)
#define AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Windows Header Files:
#include <objbase.h>
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <comdef.h>
#include <atlbase.h>
#include <commdlg.h>
#include <tchar.h>

// DirectShow Header Files
#include <dshow.h>
#include <streams.h>
#include <d3d9.h>
#include <vmr9.h>
#include <Wxutil.h>
#include <time.h>

#pragma warning(push, 2)
#include <vector>
#pragma warning(pop)
using namespace std;

//#include "PlaneScene.h"



class CAllocator  : public  IVMRSurfaceAllocator9, 
                            IVMRImagePresenter9
{
public:
    CAllocator(HRESULT& hr, HWND wnd, IDirect3D9* d3d = NULL, IDirect3DDevice9* d3dd = NULL);
    virtual ~CAllocator();

    // IVMRSurfaceAllocator9
    virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
            /* [in] */ DWORD_PTR dwUserID,
            /* [in] */ VMR9AllocationInfo *lpAllocInfo,
            /* [out][in] */ DWORD *lpNumBuffers);
            
    virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
        /* [in] */ DWORD_PTR dwID);
    
    virtual HRESULT STDMETHODCALLTYPE GetSurface( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ DWORD SurfaceIndex,
        /* [in] */ DWORD SurfaceFlags,
        /* [out] */ IDirect3DSurface9 **lplpSurface);
    
    virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
        /* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

    // IVMRImagePresenter9
    virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
        /* [in] */ DWORD_PTR dwUserID);
    
    virtual HRESULT STDMETHODCALLTYPE PresentImage( 
        /* [in] */ DWORD_PTR dwUserID,
        /* [in] */ VMR9PresentationInfo *lpPresInfo);
    
    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        REFIID riid,
        void** ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

protected:
    HRESULT CreateDevice();

    // a helper function to erase every surface in the vector
    void DeleteSurfaces();

    bool NeedToHandleDisplayChange();

    // This function is here so we can catch the loss of surfaces.
    // All the functions are using the FAIL_RET macro so that they exit
    // with the last error code.  When this returns with the surface lost
    // error code we can restore the surfaces.
    HRESULT PresentHelper(VMR9PresentationInfo *lpPresInfo);

private:
    // needed to make this a thread safe object
    CCritSec    m_ObjectLock;
    HWND        m_window;
    long        m_refCount;

    CComPtr<IDirect3D9>                     m_D3D;
    CComPtr<IDirect3DDevice9>               m_D3DDev;
    CComPtr<IVMRSurfaceAllocatorNotify9>    m_lpIVMRSurfAllocNotify;
    vector<CComPtr<IDirect3DSurface9> >     m_surfaces;
    CComPtr<IDirect3DSurface9>              m_renderTarget;
    CComPtr<IDirect3DTexture9>              m_privateTexture;
//    CPlaneScene                             m_scene;

public:
    IDirect3DTexture9 * getTexture() { return m_privateTexture; }
    IDirect3DSurface9 * getSurface() { return m_renderTarget;   }
    
};

#endif // !defined(AFX_ALLOCATOR_H__F675D766_1E57_4269_A4B9_C33FB672B856__INCLUDED_)
