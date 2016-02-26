#pragma once

#ifndef VIDEO_FLAGS_H
#define VIDEO_FLAGS_H

#define DEBUG_RESET_DEVICE 0

#define USE_VMR9           0
#define MORE_VIDEO_FILTERS 0
#define ENABLE_YUV2RGB     1

#define REF_NEEDED 0
#define HAL_NEEDED !REF_NEEDED

// This is used for Debugging.  If the Debug Version of D3D Runtime is used, 
// turning this on, will disable buffer clears, and eliminate the tearing that
// is normally seen
#define NO_CLEARS          0

#if NO_CLEARS
#define D3DCLEAR(buffer, color) S_OK
#else
#define D3DCLEAR(buffer, color) pd3dDevice->Clear(0, NULL, buffer, color, 1.0, 0)
#endif


#if 1 
    #define D3D_RELEASE(object) \
        do { \
            if (object) \
                if (object->Release() == 0) \
                    object = NULL; \
        } while (object != NULL)

#else
    #define D3D_RELEASE(object) \
        g_startcount = 0; \
        do { \
            if (object) { \
                g_refcount = object->Release(); \
                g_startcount = max(g_refcount+1, g_startcount); \
                if (g_refcount == 0) { object = NULL; } \
            } else { \
                g_refcount = 0; \
            } \
        } while (g_refcount > 0);
#endif

#endif
