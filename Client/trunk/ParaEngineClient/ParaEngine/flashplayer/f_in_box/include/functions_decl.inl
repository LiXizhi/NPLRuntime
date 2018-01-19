HRESULT WINAPI FPC_SetZoomRect(/* in */ HWND hwndFlashPlayerControl, /* in */ long left, /* in */ long top, /* in */ long right, /* in */ long bottom);

HRESULT WINAPI FPC_Zoom(/* in */ HWND hwndFlashPlayerControl, /* in */ int factor);

HRESULT WINAPI FPC_Pan(/* in */ HWND hwndFlashPlayerControl, /* in */ long x, /* in */ long y, /* in */ int mode);

HRESULT WINAPI FPC_Play(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_Stop(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_Back(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_Forward(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_Rewind(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_StopPlay(/* in */ HWND hwndFlashPlayerControl);

HRESULT WINAPI FPC_GotoFrame(/* in */ HWND hwndFlashPlayerControl, /* in */ long FrameNum);

HRESULT WINAPI FPC_CurrentFrame(/* in */ HWND hwndFlashPlayerControl, /* out */ long* Result);

HRESULT WINAPI FPC_IsPlaying(/* in */ HWND hwndFlashPlayerControl, /* out */ VARIANT_BOOL* Result);

HRESULT WINAPI FPC_PercentLoaded(/* in */ HWND hwndFlashPlayerControl, /* out */ long* Result);

HRESULT WINAPI FPC_FrameLoaded(/* in */ HWND hwndFlashPlayerControl, /* in */ long FrameNum, /* out */ VARIANT_BOOL* Result);

HRESULT WINAPI FPC_FlashVersion(/* in */ HWND hwndFlashPlayerControl, /* out */ long* Result);

HRESULT WINAPI FPC_LoadMovieA(/* in */ HWND hwndFlashPlayerControl, /* in */ int layer, /* in */ LPCSTR url);
HRESULT WINAPI FPC_LoadMovieW(/* in */ HWND hwndFlashPlayerControl, /* in */ int layer, /* in */ LPCWSTR url);

#ifdef UNICODE
    #define FPC_LoadMovie		FPC_LoadMovieW
#else
    #define FPC_LoadMovie		FPC_LoadMovieA
#endif // UNICODE


HRESULT WINAPI FPC_TGotoFrameA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ long FrameNum);
HRESULT WINAPI FPC_TGotoFrameW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ long FrameNum);

#ifdef UNICODE
    #define FPC_TGotoFrame		FPC_TGotoFrameW
#else
    #define FPC_TGotoFrame		FPC_TGotoFrameA
#endif // UNICODE


HRESULT WINAPI FPC_TGotoLabelA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ LPCSTR label);
HRESULT WINAPI FPC_TGotoLabelW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ LPCWSTR label);

#ifdef UNICODE
    #define FPC_TGotoLabel		FPC_TGotoLabelW
#else
    #define FPC_TGotoLabel		FPC_TGotoLabelA
#endif // UNICODE


HRESULT WINAPI FPC_TCurrentFrameA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* out */ long* Result);
HRESULT WINAPI FPC_TCurrentFrameW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* out */ long* Result);

#ifdef UNICODE
    #define FPC_TCurrentFrame		FPC_TCurrentFrameW
#else
    #define FPC_TCurrentFrame		FPC_TCurrentFrameA
#endif // UNICODE


HRESULT WINAPI FPC_TCurrentLabelA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* out */ LPSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);
HRESULT WINAPI FPC_TCurrentLabelW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* out */ LPWSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_TCurrentLabel		FPC_TCurrentLabelW
#else
    #define FPC_TCurrentLabel		FPC_TCurrentLabelA
#endif // UNICODE


HRESULT WINAPI FPC_TPlayA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target);
HRESULT WINAPI FPC_TPlayW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target);

#ifdef UNICODE
    #define FPC_TPlay		FPC_TPlayW
#else
    #define FPC_TPlay		FPC_TPlayA
#endif // UNICODE


HRESULT WINAPI FPC_TStopPlayA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target);
HRESULT WINAPI FPC_TStopPlayW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target);

#ifdef UNICODE
    #define FPC_TStopPlay		FPC_TStopPlayW
#else
    #define FPC_TStopPlay		FPC_TStopPlayA
#endif // UNICODE


HRESULT WINAPI FPC_SetVariableA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR name, /* in */ LPCSTR value);
HRESULT WINAPI FPC_SetVariableW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR name, /* in */ LPCWSTR value);

#ifdef UNICODE
    #define FPC_SetVariable		FPC_SetVariableW
#else
    #define FPC_SetVariable		FPC_SetVariableA
#endif // UNICODE


HRESULT WINAPI FPC_GetVariableA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR name, /* out */ LPSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetVariableW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR name, /* out */ LPWSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetVariable		FPC_GetVariableW
#else
    #define FPC_GetVariable		FPC_GetVariableA
#endif // UNICODE


HRESULT WINAPI FPC_TSetPropertyA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int property, /* in */ LPCSTR value);
HRESULT WINAPI FPC_TSetPropertyW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int property, /* in */ LPCWSTR value);

#ifdef UNICODE
    #define FPC_TSetProperty		FPC_TSetPropertyW
#else
    #define FPC_TSetProperty		FPC_TSetPropertyA
#endif // UNICODE


HRESULT WINAPI FPC_TGetPropertyA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int property, /* out */ LPSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);
HRESULT WINAPI FPC_TGetPropertyW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int property, /* out */ LPWSTR lpszBuffer, /* in, out */ DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_TGetProperty		FPC_TGetPropertyW
#else
    #define FPC_TGetProperty		FPC_TGetPropertyA
#endif // UNICODE


HRESULT WINAPI FPC_TCallFrameA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int FrameNum);
HRESULT WINAPI FPC_TCallFrameW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int FrameNum);

#ifdef UNICODE
    #define FPC_TCallFrame		FPC_TCallFrameW
#else
    #define FPC_TCallFrame		FPC_TCallFrameA
#endif // UNICODE


HRESULT WINAPI FPC_TCallLabelA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ LPCSTR label);
HRESULT WINAPI FPC_TCallLabelW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ LPCWSTR label);

#ifdef UNICODE
    #define FPC_TCallLabel		FPC_TCallLabelW
#else
    #define FPC_TCallLabel		FPC_TCallLabelA
#endif // UNICODE


HRESULT WINAPI FPC_TSetPropertyNumA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int property, /* in */ double value);
HRESULT WINAPI FPC_TSetPropertyNumW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int property, /* in */ double value);

#ifdef UNICODE
    #define FPC_TSetPropertyNum		FPC_TSetPropertyNumW
#else
    #define FPC_TSetPropertyNum		FPC_TSetPropertyNumA
#endif // UNICODE


HRESULT WINAPI FPC_TGetPropertyNumA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int property, /* out */ double* Result);
HRESULT WINAPI FPC_TGetPropertyNumW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int property, /* out */ double* Result);

#ifdef UNICODE
    #define FPC_TGetPropertyNum		FPC_TGetPropertyNumW
#else
    #define FPC_TGetPropertyNum		FPC_TGetPropertyNumA
#endif // UNICODE


HRESULT WINAPI FPC_TGetPropertyAsNumberA(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCSTR target, /* in */ int property, /* out */ double* Result);
HRESULT WINAPI FPC_TGetPropertyAsNumberW(/* in */ HWND hwndFlashPlayerControl, /* in */ LPCWSTR target, /* in */ int property, /* out */ double* Result);

#ifdef UNICODE
    #define FPC_TGetPropertyAsNumber		FPC_TGetPropertyAsNumberW
#else
    #define FPC_TGetPropertyAsNumber		FPC_TGetPropertyAsNumberA
#endif // UNICODE


HRESULT WINAPI FPC_GetReadyState(/* in */ HWND hwndFlashPlayerControl, long* pReadyState);
HRESULT WINAPI FPC_GetTotalFrames(/* in */ HWND hwndFlashPlayerControl, long* pTotalFrames);
HRESULT WINAPI FPC_PutPlaying(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL Playing);
HRESULT WINAPI FPC_GetPlaying(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL* pPlaying);
HRESULT WINAPI FPC_PutQuality(/* in */ HWND hwndFlashPlayerControl, int Quality);
HRESULT WINAPI FPC_GetQuality(/* in */ HWND hwndFlashPlayerControl, int* pQuality);
HRESULT WINAPI FPC_PutScaleMode(/* in */ HWND hwndFlashPlayerControl, int ScaleMode);
HRESULT WINAPI FPC_GetScaleMode(/* in */ HWND hwndFlashPlayerControl, int* pScaleMode);
HRESULT WINAPI FPC_PutAlignMode(/* in */ HWND hwndFlashPlayerControl, int AlignMode);
HRESULT WINAPI FPC_GetAlignMode(/* in */ HWND hwndFlashPlayerControl, int* pAlignMode);
HRESULT WINAPI FPC_PutBackgroundColor(/* in */ HWND hwndFlashPlayerControl, long BackgroundColor);
HRESULT WINAPI FPC_GetBackgroundColor(/* in */ HWND hwndFlashPlayerControl, long* pBackgroundColor);
HRESULT WINAPI FPC_PutLoop(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL Loop);
HRESULT WINAPI FPC_GetLoop(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL* pLoop);
HRESULT WINAPI FPC_PutMovieA(/* in */ HWND hwndFlashPlayerControl, LPCSTR Movie);
HRESULT WINAPI FPC_PutMovieW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR Movie);

#ifdef UNICODE
    #define FPC_PutMovie		FPC_PutMovieW
#else
    #define FPC_PutMovie		FPC_PutMovieA
#endif // UNICODE


HRESULT WINAPI FPC_GetMovieA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetMovieW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetMovie		FPC_GetMovieW
#else
    #define FPC_GetMovie		FPC_GetMovieA
#endif // UNICODE


HRESULT WINAPI FPC_PutFrameNum(/* in */ HWND hwndFlashPlayerControl, long FrameNum);
HRESULT WINAPI FPC_GetFrameNum(/* in */ HWND hwndFlashPlayerControl, long* pFrameNum);
HRESULT WINAPI FPC_PutWModeA(/* in */ HWND hwndFlashPlayerControl, LPCSTR WMode);
HRESULT WINAPI FPC_PutWModeW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR WMode);

#ifdef UNICODE
    #define FPC_PutWMode		FPC_PutWModeW
#else
    #define FPC_PutWMode		FPC_PutWModeA
#endif // UNICODE


HRESULT WINAPI FPC_GetWModeA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetWModeW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetWMode		FPC_GetWModeW
#else
    #define FPC_GetWMode		FPC_GetWModeA
#endif // UNICODE


HRESULT WINAPI FPC_PutSAlignA(/* in */ HWND hwndFlashPlayerControl, LPCSTR SAlign);
HRESULT WINAPI FPC_PutSAlignW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR SAlign);

#ifdef UNICODE
    #define FPC_PutSAlign		FPC_PutSAlignW
#else
    #define FPC_PutSAlign		FPC_PutSAlignA
#endif // UNICODE


HRESULT WINAPI FPC_GetSAlignA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetSAlignW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetSAlign		FPC_GetSAlignW
#else
    #define FPC_GetSAlign		FPC_GetSAlignA
#endif // UNICODE


HRESULT WINAPI FPC_PutMenu(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL Menu);
HRESULT WINAPI FPC_GetMenu(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL* pMenu);
HRESULT WINAPI FPC_PutBaseA(/* in */ HWND hwndFlashPlayerControl, LPCSTR Base);
HRESULT WINAPI FPC_PutBaseW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR Base);

#ifdef UNICODE
    #define FPC_PutBase		FPC_PutBaseW
#else
    #define FPC_PutBase		FPC_PutBaseA
#endif // UNICODE


HRESULT WINAPI FPC_GetBaseA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetBaseW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetBase		FPC_GetBaseW
#else
    #define FPC_GetBase		FPC_GetBaseA
#endif // UNICODE


HRESULT WINAPI FPC_PutScaleA(/* in */ HWND hwndFlashPlayerControl, LPCSTR Scale);
HRESULT WINAPI FPC_PutScaleW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR Scale);

#ifdef UNICODE
    #define FPC_PutScale		FPC_PutScaleW
#else
    #define FPC_PutScale		FPC_PutScaleA
#endif // UNICODE


HRESULT WINAPI FPC_GetScaleA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetScaleW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetScale		FPC_GetScaleW
#else
    #define FPC_GetScale		FPC_GetScaleA
#endif // UNICODE


HRESULT WINAPI FPC_PutDeviceFont(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL DeviceFont);
HRESULT WINAPI FPC_GetDeviceFont(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL* pDeviceFont);
HRESULT WINAPI FPC_PutEmbedMovie(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL EmbedMovie);
HRESULT WINAPI FPC_GetEmbedMovie(/* in */ HWND hwndFlashPlayerControl, VARIANT_BOOL* pEmbedMovie);
HRESULT WINAPI FPC_PutBGColorA(/* in */ HWND hwndFlashPlayerControl, LPCSTR BGColor);
HRESULT WINAPI FPC_PutBGColorW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR BGColor);

#ifdef UNICODE
    #define FPC_PutBGColor		FPC_PutBGColorW
#else
    #define FPC_PutBGColor		FPC_PutBGColorA
#endif // UNICODE


HRESULT WINAPI FPC_GetBGColorA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetBGColorW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetBGColor		FPC_GetBGColorW
#else
    #define FPC_GetBGColor		FPC_GetBGColorA
#endif // UNICODE


HRESULT WINAPI FPC_PutQuality2A(/* in */ HWND hwndFlashPlayerControl, LPCSTR Quality2);
HRESULT WINAPI FPC_PutQuality2W(/* in */ HWND hwndFlashPlayerControl, LPCWSTR Quality2);

#ifdef UNICODE
    #define FPC_PutQuality2		FPC_PutQuality2W
#else
    #define FPC_PutQuality2		FPC_PutQuality2A
#endif // UNICODE


HRESULT WINAPI FPC_GetQuality2A(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetQuality2W(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetQuality2		FPC_GetQuality2W
#else
    #define FPC_GetQuality2		FPC_GetQuality2A
#endif // UNICODE


HRESULT WINAPI FPC_PutSWRemoteA(/* in */ HWND hwndFlashPlayerControl, LPCSTR SWRemote);
HRESULT WINAPI FPC_PutSWRemoteW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR SWRemote);

#ifdef UNICODE
    #define FPC_PutSWRemote		FPC_PutSWRemoteW
#else
    #define FPC_PutSWRemote		FPC_PutSWRemoteA
#endif // UNICODE


HRESULT WINAPI FPC_GetSWRemoteA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetSWRemoteW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetSWRemote		FPC_GetSWRemoteW
#else
    #define FPC_GetSWRemote		FPC_GetSWRemoteA
#endif // UNICODE


HRESULT WINAPI FPC_PutStackingA(/* in */ HWND hwndFlashPlayerControl, LPCSTR Stacking);
HRESULT WINAPI FPC_PutStackingW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR Stacking);

#ifdef UNICODE
    #define FPC_PutStacking		FPC_PutStackingW
#else
    #define FPC_PutStacking		FPC_PutStackingA
#endif // UNICODE


HRESULT WINAPI FPC_GetStackingA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetStackingW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetStacking		FPC_GetStackingW
#else
    #define FPC_GetStacking		FPC_GetStackingA
#endif // UNICODE


HRESULT WINAPI FPC_PutFlashVarsA(/* in */ HWND hwndFlashPlayerControl, LPCSTR FlashVars);
HRESULT WINAPI FPC_PutFlashVarsW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR FlashVars);

#ifdef UNICODE
    #define FPC_PutFlashVars		FPC_PutFlashVarsW
#else
    #define FPC_PutFlashVars		FPC_PutFlashVarsA
#endif // UNICODE


HRESULT WINAPI FPC_GetFlashVarsA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetFlashVarsW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetFlashVars		FPC_GetFlashVarsW
#else
    #define FPC_GetFlashVars		FPC_GetFlashVarsA
#endif // UNICODE


HRESULT WINAPI FPC_PutAllowScriptAccessA(/* in */ HWND hwndFlashPlayerControl, LPCSTR AllowScriptAccess);
HRESULT WINAPI FPC_PutAllowScriptAccessW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR AllowScriptAccess);

#ifdef UNICODE
    #define FPC_PutAllowScriptAccess		FPC_PutAllowScriptAccessW
#else
    #define FPC_PutAllowScriptAccess		FPC_PutAllowScriptAccessA
#endif // UNICODE


HRESULT WINAPI FPC_GetAllowScriptAccessA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetAllowScriptAccessW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetAllowScriptAccess		FPC_GetAllowScriptAccessW
#else
    #define FPC_GetAllowScriptAccess		FPC_GetAllowScriptAccessA
#endif // UNICODE


HRESULT WINAPI FPC_PutMovieDataA(/* in */ HWND hwndFlashPlayerControl, LPCSTR MovieData);
HRESULT WINAPI FPC_PutMovieDataW(/* in */ HWND hwndFlashPlayerControl, LPCWSTR MovieData);

#ifdef UNICODE
    #define FPC_PutMovieData		FPC_PutMovieDataW
#else
    #define FPC_PutMovieData		FPC_PutMovieDataA
#endif // UNICODE


HRESULT WINAPI FPC_GetMovieDataA(/* in */ HWND hwndFlashPlayerControl, LPSTR lpszBuffer, DWORD* pdwBufferSize);
HRESULT WINAPI FPC_GetMovieDataW(/* in */ HWND hwndFlashPlayerControl, LPWSTR lpszBuffer, DWORD* pdwBufferSize);

#ifdef UNICODE
    #define FPC_GetMovieData		FPC_GetMovieDataW
#else
    #define FPC_GetMovieData		FPC_GetMovieDataA
#endif // UNICODE


