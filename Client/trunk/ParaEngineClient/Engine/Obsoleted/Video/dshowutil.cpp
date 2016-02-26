//------------------------------------------------------------------------------
// File: DShowUtil.cpp
//
// Desc: DirectShow sample code - utility functions.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//#include "stdafx.h"
#include <d3dx9.h>
#include <d3d9.h>
#include <windows.h>
#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>

#include <d3d9types.h>

#include <atlbase.h>
#include <dshow.h>
#include <mtype.h>
#include <wxdebug.h>
#include <reftime.h>

#include <assert.h>

#include "dshowutil.h"

HRESULT AddFilterByCLSID( IGraphBuilder *pGB,     // Pointer to the Filter Graph Manager.
                          const GUID& clsid,      // CLSID of the filter to create.
                          LPCWSTR wszName,        // A name for the filter.
                          IBaseFilter **ppFilter) // Receives a pointer to the filter.
{
    if (!pGB || ! ppFilter) return E_POINTER;
    *ppFilter = 0;
    IBaseFilter *pFilter = 0;
    HRESULT hr = CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER,
        IID_IBaseFilter, reinterpret_cast<void**>(&pFilter));
    if (SUCCEEDED(hr))
    {
        hr = pGB->AddFilter(pFilter, wszName);
        if (SUCCEEDED(hr))
            *ppFilter = pFilter;
        else
            pFilter->Release();
    }
    return hr;
}

// Get the first upstream or downstream filter
HRESULT GetNextFilter(  IBaseFilter *pFilter, // Pointer to the starting filter
                        PIN_DIRECTION Dir,    // Direction to search (upstream or downstream)
                        IBaseFilter **ppNext, // Receives a pointer to the next filter.
                        IPin **ppPin)          // Receives a pointer to pin of the filter
{
    if (!pFilter || !ppNext) return E_POINTER;

    IEnumPins *pEnum = 0;
    IPin *pPin = 0;
    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)) return hr;
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        // See if this pin matches the specified direction.
        PIN_DIRECTION ThisPinDir;
        hr = pPin->QueryDirection(&ThisPinDir);
        if (FAILED(hr))
        {
            // Something strange happened.
            hr = E_UNEXPECTED;
            pPin->Release();
            break;
        }
        if (ThisPinDir == Dir)
        {
            // Check if the pin is connected to another pin.
            IPin *pPinNext = 0;
            hr = pPin->ConnectedTo(&pPinNext);
            if (SUCCEEDED(hr))
            {
                // Get the filter that owns that pin.
                PIN_INFO PinInfo;
                hr = pPinNext->QueryPinInfo(&PinInfo);
                pPinNext->Release();
                pEnum->Release();

                if (ppPin == NULL) {
                    pPin->Release();
                } else {
                    **ppPin = *pPin;  // now the client needs to release this pin
                }

                if (FAILED(hr) || (PinInfo.pFilter == NULL))
                {
                    // Something strange happened.
                    return E_UNEXPECTED;
                }
                // This is the filter we're looking for.
                *ppNext = PinInfo.pFilter; // Client must release.
                return S_OK;
            }
        }
    }
    pEnum->Release();
    // Did not find a matching filter.
    return E_FAIL;
}



HRESULT FindRenderer(IGraphBuilder *pGB, const GUID *mediatype, IBaseFilter **ppFilter)
{
    HRESULT hr;
    IEnumFilters *pEnum = NULL;
    IBaseFilter *pFilter = NULL;
    IPin *pPin;
    ULONG ulFetched, ulInPins, ulOutPins;
    BOOL bFound=FALSE;

    // Verify graph builder interface
    if (!pGB)
        return E_NOINTERFACE;

    // Verify that a media type was passed
    if (!mediatype)
        return E_POINTER;

    // Clear the filter pointer in case there is no match
    if (ppFilter)
        *ppFilter = NULL;

    // Get filter enumerator
    hr = pGB->EnumFilters(&pEnum);
    if (FAILED(hr))
        return hr;

    pEnum->Reset();

    // Enumerate all filters in the graph
    while(!bFound && (pEnum->Next(1, &pFilter, &ulFetched) == S_OK))
    {
#ifdef DEBUG
        // Read filter name for debugging purposes
        FILTER_INFO FilterInfo;
        TCHAR szName[256];
    
        hr = pFilter->QueryFilterInfo(&FilterInfo);
        if (SUCCEEDED(hr))
        {
            // Show filter name in debugger
#ifdef UNICODE
            lstrcpyn(szName, FilterInfo.achName, 256);
#else
            WideCharToMultiByte(CP_ACP, 0, FilterInfo.achName, -1, szName, 256, 0, 0);
#endif
            FilterInfo.pGraph->Release();
        }
        szName[255] = 0;        // Null-terminate
#endif

        // Find a filter with one input and no output pins
        hr = CountFilterPins(pFilter, &ulInPins, &ulOutPins);
        if (FAILED(hr))
            break;

        if ((ulInPins == 1) && (ulOutPins == 0))
        {
            // Get the first pin on the filter
            pPin=0;
            pPin = GetInPin(pFilter, 0);

            // Read this pin's major media type
            AM_MEDIA_TYPE type={0};
            hr = pPin->ConnectionMediaType(&type);
            if (FAILED(hr))
                break;

            // Is this pin's media type the requested type?
            // If so, then this is the renderer for which we are searching.
            // Copy the interface pointer and return.
            if (type.majortype == *mediatype)
            {
                // Found our filter
                *ppFilter = pFilter;
                bFound = TRUE;
            }
            // This is not the renderer, so release the interface.
            else
                pFilter->Release();

            // Delete memory allocated by ConnectionMediaType()
            UtilFreeMediaType(type);
        }
        else
        {
            // No match, so release the interface
            pFilter->Release();
        }
    }

    pEnum->Release();
    return hr;
}


HRESULT FindAudioRenderer(IGraphBuilder *pGB, IBaseFilter **ppFilter)
{
    return FindRenderer(pGB, &MEDIATYPE_Audio, ppFilter);
}

HRESULT FindVideoRenderer(IGraphBuilder *pGB, IBaseFilter **ppFilter)
{
    return FindRenderer(pGB, &MEDIATYPE_Video, ppFilter);
}


HRESULT CountFilterPins(IBaseFilter *pFilter, ULONG *pulInPins, ULONG *pulOutPins)
{
    HRESULT hr=S_OK;
    IEnumPins *pEnum=0;
    ULONG ulFound;
    IPin *pPin;

    // Verify input
    if (!pFilter || !pulInPins || !pulOutPins)
        return E_POINTER;

    // Clear number of pins found
    *pulInPins = 0;
    *pulOutPins = 0;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    pEnum->Reset();

    // Count every pin on the filter
    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION) 3;

        hr = pPin->QueryDirection(&pindir);

        if(pindir == PINDIR_INPUT)
            (*pulInPins)++;
        else
            (*pulOutPins)++;

        pPin->Release();
    } 

    pEnum->Release();
    return hr;
}


HRESULT CountTotalFilterPins(IBaseFilter *pFilter, ULONG *pulPins)
{
    HRESULT hr;
    IEnumPins *pEnum=0;
    ULONG ulFound;
    IPin *pPin;

    // Verify input
    if (!pFilter || !pulPins)
        return E_POINTER;

    // Clear number of pins found
    *pulPins = 0;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    // Count every pin on the filter, ignoring direction
    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        (*pulPins)++;
        pPin->Release();
    } 

    pEnum->Release();
    return hr;
}


HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

    if (!pFilter)
       return E_POINTER;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection(&pindir);
        if(pindir == dirrequired)
        {
            if(iNum == 0)
            {
                *ppPin = pPin;  // Return the pin's interface
                hr = S_OK;      // Found requested pin, so clear error
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    return hr;
}

//
// NOTE: The GetInPin and GetOutPin methods DO NOT increment the reference count
// of the returned pin.  Use CComPtr interface pointers in your code to prevent
// memory leaks caused by reference counting problems.  The SDK samples that use
// these methods all use CComPtr<IPin> interface pointers.
// 
//     For example:  CComPtr<IPin> pPin = GetInPin(pFilter,0);
//
IPin * GetInPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin;
    GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
    return pComPin;
}


IPin * GetOutPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin;
    GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
    return pComPin;
}


HRESULT FindOtherSplitterPin(IPin *pPinIn, GUID guid, int nStream, IPin **ppSplitPin)
{
    if (!ppSplitPin)
        return E_POINTER;

    CComPtr< IPin > pPinOut;
    pPinOut = pPinIn;

    while(pPinOut)
    {
        PIN_INFO ThisPinInfo;
        pPinOut->QueryPinInfo(&ThisPinInfo);
        if(ThisPinInfo.pFilter) ThisPinInfo.pFilter->Release();

        pPinOut = NULL;
        CComPtr< IEnumPins > pEnumPins;
        ThisPinInfo.pFilter->EnumPins(&pEnumPins);
        if(!pEnumPins)
        {
            return NULL;
        }

        // look at every pin on the current filter...
        //
        ULONG Fetched = 0;
        while(1)
        {
            CComPtr< IPin > pPin;
            Fetched = 0;
            assert(!pPin); // is it out of scope?
            pEnumPins->Next(1, &pPin, &Fetched);
            if(!Fetched)
            {
                break;
            }

            PIN_INFO pi;
            pPin->QueryPinInfo(&pi);
            if(pi.pFilter) pi.pFilter->Release();

            // if it's an input pin...
            //
            if(pi.dir == PINDIR_INPUT)
            {
                // continue searching upstream from this pin
                //
                pPin->ConnectedTo(&pPinOut);

                // a pin that supports the required media type is the
                // splitter pin we are looking for!  We are done
                //
            }
            else
            {
                CComPtr< IEnumMediaTypes > pMediaEnum;
                pPin->EnumMediaTypes(&pMediaEnum);
                if(pMediaEnum)
                {
                    Fetched = 0;
                    AM_MEDIA_TYPE *pMediaType;
                    pMediaEnum->Next(1, &pMediaType, &Fetched);
                    if(Fetched)
                    {
                        if(pMediaType->majortype == guid)
                        {
                            if(nStream-- == 0)
                            {
                                UtilDeleteMediaType(pMediaType);
                                *ppSplitPin = pPin;
                                (*ppSplitPin)->AddRef();
                                return S_OK;
                            }
                        }
                        UtilDeleteMediaType(pMediaType);
                    }
                }
            }

            // go try the next pin

        } // while
    }
    assert(FALSE);
    return E_FAIL;
}


HRESULT SeekNextFrame( IMediaSeeking * pSeeking, double FPS, long Frame )
{
    // try seeking by frames first
    //
    HRESULT hr = pSeeking->SetTimeFormat(&TIME_FORMAT_FRAME);
    REFERENCE_TIME Pos = 0;
    if(!FAILED(hr))
    {
        pSeeking->GetCurrentPosition(&Pos);
        Pos++;
    }
    else
    {
        // couldn't seek by frames, use Frame and FPS to calculate time
        //
        Pos = REFERENCE_TIME(double( Frame * UNITS ) / FPS);

        // add a half-frame to seek to middle of the frame
        //
        Pos += REFERENCE_TIME(double( UNITS ) * 0.5 / FPS);
    }

    hr = pSeeking->SetPositions(&Pos, AM_SEEKING_AbsolutePositioning, 
                                NULL, AM_SEEKING_NoPositioning);
    return hr;

}


// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
HRESULT AddGraphToROT(IUnknown *pUnkGraph, DWORD *pdwRegister) 
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;

    if (!pUnkGraph || !pdwRegister)
        return E_POINTER;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return E_FAIL;

    StringCchPrintfW(wsz,NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, 
              GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, 
                            pMoniker, pdwRegister);
        pMoniker->Release();
    }

    pROT->Release();
    return hr;
}

// Removes a filter graph from the Running Object Table
void RemoveGraphFromROT(DWORD pdwRegister)
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(pdwRegister);
        pROT->Release();
    }
}

/*
void ShowFilenameByCLSID(REFCLSID clsid, TCHAR *szFilename)
{
    HRESULT hr;
    LPOLESTR strCLSID;

    // Convert binary CLSID to a readable version
    hr = StringFromCLSID(clsid, &strCLSID);
    if(SUCCEEDED(hr))
    {
        TCHAR szKey[512];
        CString strQuery(strCLSID);

        // Create key name for reading filename registry
        wsprintf(szKey, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"),
                 (LPCTSTR) strQuery);

        // Free memory associated with strCLSID (allocated in StringFromCLSID)
        CoTaskMemFree(strCLSID);

        HKEY hkeyFilter=0;
        DWORD dwSize=MAX_PATH;
        BYTE szFile[MAX_PATH];
        int rc=0;

        // Open the CLSID key that contains information about the filter
        rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);
        if (rc == ERROR_SUCCESS)
        {
            rc = RegQueryValueEx(hkeyFilter, NULL,  // Read (Default) value
                                 NULL, NULL, szFile, &dwSize);

            if (rc == ERROR_SUCCESS)
                wsprintf(szFilename, TEXT("%s\0"), szFile);
            else
                _tcscpy(szFilename, TEXT("<Unknown>\0"));

            RegCloseKey(hkeyFilter);
        }
    }
}
*/

HRESULT GetFileDurationString(IMediaSeeking *pMS, TCHAR *szDuration)
{
    HRESULT hr;

    if (!pMS)
        return E_NOINTERFACE;
    if (!szDuration)
        return E_POINTER;

    // Initialize the display in case we can't read the duration
    StringCchCopy(szDuration, NUMELMS(szDuration), TEXT("<00:00.000>\0"));

    // Is media time supported for this file?
    if (S_OK != pMS->IsFormatSupported(&TIME_FORMAT_MEDIA_TIME))
        return E_NOINTERFACE;

    // Read the time format to restore later
    GUID guidOriginalFormat;
    hr = pMS->GetTimeFormat(&guidOriginalFormat);
    if (FAILED(hr))
        return hr;

    // Ensure media time format for easy display
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
    if (FAILED(hr))
        return hr;

    // Read the file's duration
    LONGLONG llDuration;
    hr = pMS->GetDuration(&llDuration);
    if (FAILED(hr))
        return hr;

    // Return to the original format
    if (guidOriginalFormat != TIME_FORMAT_MEDIA_TIME)
    {
        hr = pMS->SetTimeFormat(&guidOriginalFormat);
        if (FAILED(hr))
            return hr;
    }

    // Convert the LONGLONG duration into human-readable format
    unsigned long nTotalMS = (unsigned long) llDuration / 10000; // 100ns -> ms
    int nMS = nTotalMS % 1000;
    int nSeconds = nTotalMS / 1000;
    int nMinutes = nSeconds / 60;
    nSeconds %= 60;

    // Update the string
    StringCchPrintf(szDuration, NUMELMS(szDuration), _T("%02dm:%02d.%03ds\0"), nMinutes, nSeconds, nMS);

    return hr;
}


BOOL SupportsPropertyPage(IBaseFilter *pFilter) 
{
    HRESULT hr;
    ISpecifyPropertyPages *pSpecify;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        pSpecify->Release();
        return TRUE;
    }
    else
        return FALSE;
}


HRESULT ShowFilterPropertyPage(IBaseFilter *pFilter, HWND hwndParent)
{
    HRESULT hr;
    ISpecifyPropertyPages *pSpecify=0;

    if (!pFilter)
        return E_NOINTERFACE;

    // Discover if this filter contains a property page
    hr = pFilter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpecify);
    if (SUCCEEDED(hr)) 
    {
        do 
        {
            FILTER_INFO FilterInfo;
            hr = pFilter->QueryFilterInfo(&FilterInfo);
            if (FAILED(hr))
                break;

            CAUUID caGUID;
            hr = pSpecify->GetPages(&caGUID);
            if (FAILED(hr))
                break;
      
            // Display the filter's property page
            OleCreatePropertyFrame(
                hwndParent,             // Parent window
                0,                      // x (Reserved)
                0,                      // y (Reserved)
                FilterInfo.achName,     // Caption for the dialog box
                1,                      // Number of filters
                (IUnknown **)&pFilter,  // Pointer to the filter 
                caGUID.cElems,          // Number of property pages
                caGUID.pElems,          // Pointer to property page CLSIDs
                0,                      // Locale identifier
                0,                      // Reserved
                NULL                    // Reserved
            );

            CoTaskMemFree(caGUID.pElems);
            FilterInfo.pGraph->Release(); 

        } while(0);
    }

    // Release interfaces
    if (pSpecify)
        pSpecify->Release();
        
    pFilter->Release();
    return hr;
}


//
// Some hardware decoders and video renderers support stepping media
// frame by frame with the IVideoFrameStep interface.  See the interface
// documentation for more details on frame stepping.
//
BOOL CanFrameStep(IGraphBuilder *pGB)
{
    HRESULT hr;
    IVideoFrameStep* pFS;

    // Get frame step interface
    hr = pGB->QueryInterface(__uuidof(IVideoFrameStep), (PVOID *)&pFS);
    if (FAILED(hr))
        return FALSE;

    // Check if this decoder can step
    hr = pFS->CanStep(0L, NULL); 

    // Release frame step interface
    pFS->Release();

    if (hr == S_OK)
        return TRUE;
    else
        return FALSE;
}


//
// General purpose function to delete a heap allocated AM_MEDIA_TYPE structure.
// This is useful when calling IEnumMediaTypes::Next, as the interface
// implementation allocates the structures which you must later delete.
// The format block may also be a pointer to an interface to release.
//
// This code was exerpted from the BaseClasses' mtype.cpp by combining
// DeleteMediaType() and FreeMediaType().  Since some applications link with
// the base classes library (instead of simply linking with strmiids.lib)
// strictly because they need the DeleteMediaType() method, this utility
// method can be used instead to prevent unnecessary overhead.

void UtilDeleteMediaType(AM_MEDIA_TYPE *pmt)
{
    // Allow NULL pointers for coding simplicity
    if (pmt == NULL) {
        return;
    }

    // Free media type's format data
    if (pmt->cbFormat != 0) 
    {
        CoTaskMemFree((PVOID)pmt->pbFormat);

        // Strictly unnecessary but tidier
        pmt->cbFormat = 0;
        pmt->pbFormat = NULL;
    }

    // Release interface
    if (pmt->pUnk != NULL) 
    {
        pmt->pUnk->Release();
        pmt->pUnk = NULL;
    }

    // Free media type
    CoTaskMemFree((PVOID)pmt);
}


//  Free an existing media type (ie free resources it holds)

void UtilFreeMediaType(AM_MEDIA_TYPE& mt)
{
    if(mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);

        // Strictly unnecessary but tidier
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if(mt.pUnk != NULL)
    {
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}


HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
    HRESULT hr;
    
    IStorage *pStorage = NULL;
    hr = StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, &pStorage);
    if(FAILED(hr)) 
    {
        return hr;
    }

    IStream *pStream;
    hr = pStorage->CreateStream(
        wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, &pStream);
    if (FAILED(hr)) 
    {
        pStorage->Release();    
        return hr;
    }

    IPersistStream *pPersist = NULL;
    pGraph->QueryInterface(IID_IPersistStream, reinterpret_cast<void**>(&pPersist));
    hr = pPersist->Save(pStream, TRUE);
    pStream->Release();
    pPersist->Release();
    if (SUCCEEDED(hr)) 
    {
        hr = pStorage->Commit(STGC_DEFAULT);
    }
    pStorage->Release();
    return hr;
}


HRESULT LoadGraphFile(IGraphBuilder *pGraph, const WCHAR* wszName)
{
    IStorage *pStorage = 0;
    if (S_OK != StgIsStorageFile(wszName))
    {
        return E_FAIL;
    }
    HRESULT hr = StgOpenStorage(wszName, 0, 
        STGM_TRANSACTED | STGM_READ | STGM_SHARE_DENY_WRITE, 
        0, 0, &pStorage);
    if (FAILED(hr))
    {
        return hr;
    }
    IPersistStream *pPersistStream = 0;
    hr = pGraph->QueryInterface(IID_IPersistStream,
             reinterpret_cast<void**>(&pPersistStream));
    if (SUCCEEDED(hr))
    {
        IStream *pStream = 0;
        hr = pStorage->OpenStream(L"ActiveMovieGraph", 0, 
            STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pStream);
        if(SUCCEEDED(hr))
        {
            hr = pPersistStream->Load(pStream);
            pStream->Release();
        }
        pPersistStream->Release();
    }
    pStorage->Release();
    return hr;
}

