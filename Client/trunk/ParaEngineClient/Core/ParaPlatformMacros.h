#pragma once

#ifndef PE_ASSERT
#if defined(_DEBUG) || (defined(COCOS2D_DEBUG) && COCOS2D_DEBUG > 0)
#define PE_ASSERT(cond) if (!(cond)) { ParaEngine::CLogger::GetSingleton().WriteFormated("assertion failed at %s:%d\n", __FILE__,__LINE__); }
#else
#define PE_ASSERT(cond)
#endif

#ifndef WIN32
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#define LOWORD(_dw)     ((WORD)(((DWORD)(_dw)) & 0xffff))
#define HIWORD(_dw)     ((WORD)((((DWORD)(_dw)) >> 16) & 0xffff))
#define MAKEWPARAM(l, h)      ((WPARAM)(DWORD)MAKELONG(l, h))
#define MAKELPARAM(l, h)      ((LPARAM)(DWORD)MAKELONG(l, h))
#define MAKELRESULT(l, h)     ((LRESULT)(DWORD)MAKELONG(l, h))
/* Value for rolling one detent */
#define WHEEL_DELTA                     120
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

#endif

