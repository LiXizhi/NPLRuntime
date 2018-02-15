//================================================================================
// Method: SetZoomRect
// Return type: void

typedef struct SFPCSetZoomRect
{
	// [in] left
	long left;

	// [in] top
	long top;

	// [in] right
	long right;

	// [in] bottom
	long bottom;

	// [out]
	HRESULT hr;

} SFPCSetZoomRect;

#define FPCM_SETZOOMRECT		(FPCM_FIRST + 256)

//================================================================================

//================================================================================
// Method: Zoom
// Return type: void

typedef struct SFPCZoom
{
	// [in] factor
	int factor;

	// [out]
	HRESULT hr;

} SFPCZoom;

#define FPCM_ZOOM		(FPCM_FIRST + 257)

//================================================================================

//================================================================================
// Method: Pan
// Return type: void

typedef struct SFPCPan
{
	// [in] x
	long x;

	// [in] y
	long y;

	// [in] mode
	int mode;

	// [out]
	HRESULT hr;

} SFPCPan;

#define FPCM_PAN		(FPCM_FIRST + 258)

//================================================================================

//================================================================================
// Method: Play
// Return type: void

typedef struct SFPCPlay
{
	// [out]
	HRESULT hr;

} SFPCPlay;

#define FPCM_PLAY		(FPCM_FIRST + 259)

//================================================================================

//================================================================================
// Method: Stop
// Return type: void

typedef struct SFPCStop
{
	// [out]
	HRESULT hr;

} SFPCStop;

#define FPCM_STOP		(FPCM_FIRST + 260)

//================================================================================

//================================================================================
// Method: Back
// Return type: void

typedef struct SFPCBack
{
	// [out]
	HRESULT hr;

} SFPCBack;

#define FPCM_BACK		(FPCM_FIRST + 261)

//================================================================================

//================================================================================
// Method: Forward
// Return type: void

typedef struct SFPCForward
{
	// [out]
	HRESULT hr;

} SFPCForward;

#define FPCM_FORWARD		(FPCM_FIRST + 262)

//================================================================================

//================================================================================
// Method: Rewind
// Return type: void

typedef struct SFPCRewind
{
	// [out]
	HRESULT hr;

} SFPCRewind;

#define FPCM_REWIND		(FPCM_FIRST + 263)

//================================================================================

//================================================================================
// Method: StopPlay
// Return type: void

typedef struct SFPCStopPlay
{
	// [out]
	HRESULT hr;

} SFPCStopPlay;

#define FPCM_STOPPLAY		(FPCM_FIRST + 264)

//================================================================================

//================================================================================
// Method: GotoFrame
// Return type: void

typedef struct SFPCGotoFrame
{
	// [in] FrameNum
	long FrameNum;

	// [out]
	HRESULT hr;

} SFPCGotoFrame;

#define FPCM_GOTOFRAME		(FPCM_FIRST + 265)

//================================================================================

//================================================================================
// Method: CurrentFrame
// Return type: long

typedef struct SFPCCurrentFrame
{
	// [out, retval]
	long Result;

	// [out]
	HRESULT hr;

} SFPCCurrentFrame;

#define FPCM_CURRENTFRAME		(FPCM_FIRST + 266)

//================================================================================

//================================================================================
// Method: IsPlaying
// Return type: VARIANT_BOOL

typedef struct SFPCIsPlaying
{
	// [out, retval]
	VARIANT_BOOL Result;

	// [out]
	HRESULT hr;

} SFPCIsPlaying;

#define FPCM_ISPLAYING		(FPCM_FIRST + 267)

//================================================================================

//================================================================================
// Method: PercentLoaded
// Return type: long

typedef struct SFPCPercentLoaded
{
	// [out, retval]
	long Result;

	// [out]
	HRESULT hr;

} SFPCPercentLoaded;

#define FPCM_PERCENTLOADED		(FPCM_FIRST + 268)

//================================================================================

//================================================================================
// Method: FrameLoaded
// Return type: VARIANT_BOOL

typedef struct SFPCFrameLoaded
{
	// [in] FrameNum
	long FrameNum;

	// [out, retval]
	VARIANT_BOOL Result;

	// [out]
	HRESULT hr;

} SFPCFrameLoaded;

#define FPCM_FRAMELOADED		(FPCM_FIRST + 269)

//================================================================================

//================================================================================
// Method: FlashVersion
// Return type: long

typedef struct SFPCFlashVersion
{
	// [out, retval]
	long Result;

	// [out]
	HRESULT hr;

} SFPCFlashVersion;

#define FPCM_FLASHVERSION		(FPCM_FIRST + 270)

//================================================================================

//================================================================================
// Method: LoadMovie
// Return type: void

typedef struct SFPCLoadMovieA
{
	// [in] layer
	int layer;

	// [in] url
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} url;

	// [out]
	HRESULT hr;

} SFPCLoadMovieA;

typedef struct SFPCLoadMovieW
{
	// [in] layer
	int layer;

	// [in] url
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} url;

	// [out]
	HRESULT hr;

} SFPCLoadMovieW;

#ifdef UNICODE
    #define SFPCLoadMovie		SFPCLoadMovieW
#else
    #define SFPCLoadMovie		SFPCLoadMovieA
#endif // UNICODE

#define FPCM_LOADMOVIEA		(FPCM_FIRST + 271)
#define FPCM_LOADMOVIEW		(FPCM_FIRST + 272)

#ifdef UNICODE
    #define FPCM_LOADMOVIE		FPCM_LOADMOVIEW
#else
    #define FPCM_LOADMOVIE		FPCM_LOADMOVIEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TGotoFrame
// Return type: void

typedef struct SFPCTGotoFrameA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] FrameNum
	long FrameNum;

	// [out]
	HRESULT hr;

} SFPCTGotoFrameA;

typedef struct SFPCTGotoFrameW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] FrameNum
	long FrameNum;

	// [out]
	HRESULT hr;

} SFPCTGotoFrameW;

#ifdef UNICODE
    #define SFPCTGotoFrame		SFPCTGotoFrameW
#else
    #define SFPCTGotoFrame		SFPCTGotoFrameA
#endif // UNICODE

#define FPCM_TGOTOFRAMEA		(FPCM_FIRST + 273)
#define FPCM_TGOTOFRAMEW		(FPCM_FIRST + 274)

#ifdef UNICODE
    #define FPCM_TGOTOFRAME		FPCM_TGOTOFRAMEW
#else
    #define FPCM_TGOTOFRAME		FPCM_TGOTOFRAMEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TGotoLabel
// Return type: void

typedef struct SFPCTGotoLabelA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] label
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} label;

	// [out]
	HRESULT hr;

} SFPCTGotoLabelA;

typedef struct SFPCTGotoLabelW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] label
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} label;

	// [out]
	HRESULT hr;

} SFPCTGotoLabelW;

#ifdef UNICODE
    #define SFPCTGotoLabel		SFPCTGotoLabelW
#else
    #define SFPCTGotoLabel		SFPCTGotoLabelA
#endif // UNICODE

#define FPCM_TGOTOLABELA		(FPCM_FIRST + 275)
#define FPCM_TGOTOLABELW		(FPCM_FIRST + 276)

#ifdef UNICODE
    #define FPCM_TGOTOLABEL		FPCM_TGOTOLABELW
#else
    #define FPCM_TGOTOLABEL		FPCM_TGOTOLABELA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TCurrentFrame
// Return type: long

typedef struct SFPCTCurrentFrameA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out, retval]
	long Result;

	// [out]
	HRESULT hr;

} SFPCTCurrentFrameA;

typedef struct SFPCTCurrentFrameW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out, retval]
	long Result;

	// [out]
	HRESULT hr;

} SFPCTCurrentFrameW;

#ifdef UNICODE
    #define SFPCTCurrentFrame		SFPCTCurrentFrameW
#else
    #define SFPCTCurrentFrame		SFPCTCurrentFrameA
#endif // UNICODE

#define FPCM_TCURRENTFRAMEA		(FPCM_FIRST + 277)
#define FPCM_TCURRENTFRAMEW		(FPCM_FIRST + 278)

#ifdef UNICODE
    #define FPCM_TCURRENTFRAME		FPCM_TCURRENTFRAMEW
#else
    #define FPCM_TCURRENTFRAME		FPCM_TCURRENTFRAMEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TCurrentLabel
// Return type: BSTR

typedef struct SFPCTCurrentLabelA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	struct
	{
		LPSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCTCurrentLabelA;

typedef struct SFPCTCurrentLabelW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	struct
	{
		LPWSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCTCurrentLabelW;

#ifdef UNICODE
    #define SFPCTCurrentLabel		SFPCTCurrentLabelW
#else
    #define SFPCTCurrentLabel		SFPCTCurrentLabelA
#endif // UNICODE

#define FPCM_TCURRENTLABELA		(FPCM_FIRST + 279)
#define FPCM_TCURRENTLABELW		(FPCM_FIRST + 280)

#ifdef UNICODE
    #define FPCM_TCURRENTLABEL		FPCM_TCURRENTLABELW
#else
    #define FPCM_TCURRENTLABEL		FPCM_TCURRENTLABELA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TPlay
// Return type: void

typedef struct SFPCTPlayA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	HRESULT hr;

} SFPCTPlayA;

typedef struct SFPCTPlayW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	HRESULT hr;

} SFPCTPlayW;

#ifdef UNICODE
    #define SFPCTPlay		SFPCTPlayW
#else
    #define SFPCTPlay		SFPCTPlayA
#endif // UNICODE

#define FPCM_TPLAYA		(FPCM_FIRST + 281)
#define FPCM_TPLAYW		(FPCM_FIRST + 282)

#ifdef UNICODE
    #define FPCM_TPLAY		FPCM_TPLAYW
#else
    #define FPCM_TPLAY		FPCM_TPLAYA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TStopPlay
// Return type: void

typedef struct SFPCTStopPlayA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	HRESULT hr;

} SFPCTStopPlayA;

typedef struct SFPCTStopPlayW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [out]
	HRESULT hr;

} SFPCTStopPlayW;

#ifdef UNICODE
    #define SFPCTStopPlay		SFPCTStopPlayW
#else
    #define SFPCTStopPlay		SFPCTStopPlayA
#endif // UNICODE

#define FPCM_TSTOPPLAYA		(FPCM_FIRST + 283)
#define FPCM_TSTOPPLAYW		(FPCM_FIRST + 284)

#ifdef UNICODE
    #define FPCM_TSTOPPLAY		FPCM_TSTOPPLAYW
#else
    #define FPCM_TSTOPPLAY		FPCM_TSTOPPLAYA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: SetVariable
// Return type: void

typedef struct SFPCSetVariableA
{
	// [in] name
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} name;

	// [in] value
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} value;

	// [out]
	HRESULT hr;

} SFPCSetVariableA;

typedef struct SFPCSetVariableW
{
	// [in] name
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} name;

	// [in] value
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} value;

	// [out]
	HRESULT hr;

} SFPCSetVariableW;

#ifdef UNICODE
    #define SFPCSetVariable		SFPCSetVariableW
#else
    #define SFPCSetVariable		SFPCSetVariableA
#endif // UNICODE

#define FPCM_SETVARIABLEA		(FPCM_FIRST + 285)
#define FPCM_SETVARIABLEW		(FPCM_FIRST + 286)

#ifdef UNICODE
    #define FPCM_SETVARIABLE		FPCM_SETVARIABLEW
#else
    #define FPCM_SETVARIABLE		FPCM_SETVARIABLEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: GetVariable
// Return type: BSTR

typedef struct SFPCGetVariableA
{
	// [in] name
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} name;

	// [out]
	struct
	{
		LPSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCGetVariableA;

typedef struct SFPCGetVariableW
{
	// [in] name
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} name;

	// [out]
	struct
	{
		LPWSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCGetVariableW;

#ifdef UNICODE
    #define SFPCGetVariable		SFPCGetVariableW
#else
    #define SFPCGetVariable		SFPCGetVariableA
#endif // UNICODE

#define FPCM_GETVARIABLEA		(FPCM_FIRST + 287)
#define FPCM_GETVARIABLEW		(FPCM_FIRST + 288)

#ifdef UNICODE
    #define FPCM_GETVARIABLE		FPCM_GETVARIABLEW
#else
    #define FPCM_GETVARIABLE		FPCM_GETVARIABLEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TSetProperty
// Return type: void

typedef struct SFPCTSetPropertyA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [in] value
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} value;

	// [out]
	HRESULT hr;

} SFPCTSetPropertyA;

typedef struct SFPCTSetPropertyW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [in] value
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} value;

	// [out]
	HRESULT hr;

} SFPCTSetPropertyW;

#ifdef UNICODE
    #define SFPCTSetProperty		SFPCTSetPropertyW
#else
    #define SFPCTSetProperty		SFPCTSetPropertyA
#endif // UNICODE

#define FPCM_TSETPROPERTYA		(FPCM_FIRST + 289)
#define FPCM_TSETPROPERTYW		(FPCM_FIRST + 290)

#ifdef UNICODE
    #define FPCM_TSETPROPERTY		FPCM_TSETPROPERTYW
#else
    #define FPCM_TSETPROPERTY		FPCM_TSETPROPERTYA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TGetProperty
// Return type: BSTR

typedef struct SFPCTGetPropertyA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out]
	struct
	{
		LPSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyA;

typedef struct SFPCTGetPropertyW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out]
	struct
	{
		LPWSTR lpszBuffer;
		DWORD dwBufferSize;

	} Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyW;

#ifdef UNICODE
    #define SFPCTGetProperty		SFPCTGetPropertyW
#else
    #define SFPCTGetProperty		SFPCTGetPropertyA
#endif // UNICODE

#define FPCM_TGETPROPERTYA		(FPCM_FIRST + 291)
#define FPCM_TGETPROPERTYW		(FPCM_FIRST + 292)

#ifdef UNICODE
    #define FPCM_TGETPROPERTY		FPCM_TGETPROPERTYW
#else
    #define FPCM_TGETPROPERTY		FPCM_TGETPROPERTYA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TCallFrame
// Return type: void

typedef struct SFPCTCallFrameA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] FrameNum
	int FrameNum;

	// [out]
	HRESULT hr;

} SFPCTCallFrameA;

typedef struct SFPCTCallFrameW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] FrameNum
	int FrameNum;

	// [out]
	HRESULT hr;

} SFPCTCallFrameW;

#ifdef UNICODE
    #define SFPCTCallFrame		SFPCTCallFrameW
#else
    #define SFPCTCallFrame		SFPCTCallFrameA
#endif // UNICODE

#define FPCM_TCALLFRAMEA		(FPCM_FIRST + 293)
#define FPCM_TCALLFRAMEW		(FPCM_FIRST + 294)

#ifdef UNICODE
    #define FPCM_TCALLFRAME		FPCM_TCALLFRAMEW
#else
    #define FPCM_TCALLFRAME		FPCM_TCALLFRAMEA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TCallLabel
// Return type: void

typedef struct SFPCTCallLabelA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] label
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} label;

	// [out]
	HRESULT hr;

} SFPCTCallLabelA;

typedef struct SFPCTCallLabelW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] label
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} label;

	// [out]
	HRESULT hr;

} SFPCTCallLabelW;

#ifdef UNICODE
    #define SFPCTCallLabel		SFPCTCallLabelW
#else
    #define SFPCTCallLabel		SFPCTCallLabelA
#endif // UNICODE

#define FPCM_TCALLLABELA		(FPCM_FIRST + 295)
#define FPCM_TCALLLABELW		(FPCM_FIRST + 296)

#ifdef UNICODE
    #define FPCM_TCALLLABEL		FPCM_TCALLLABELW
#else
    #define FPCM_TCALLLABEL		FPCM_TCALLLABELA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TSetPropertyNum
// Return type: void

typedef struct SFPCTSetPropertyNumA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [in] value
	double value;

	// [out]
	HRESULT hr;

} SFPCTSetPropertyNumA;

typedef struct SFPCTSetPropertyNumW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [in] value
	double value;

	// [out]
	HRESULT hr;

} SFPCTSetPropertyNumW;

#ifdef UNICODE
    #define SFPCTSetPropertyNum		SFPCTSetPropertyNumW
#else
    #define SFPCTSetPropertyNum		SFPCTSetPropertyNumA
#endif // UNICODE

#define FPCM_TSETPROPERTYNUMA		(FPCM_FIRST + 297)
#define FPCM_TSETPROPERTYNUMW		(FPCM_FIRST + 298)

#ifdef UNICODE
    #define FPCM_TSETPROPERTYNUM		FPCM_TSETPROPERTYNUMW
#else
    #define FPCM_TSETPROPERTYNUM		FPCM_TSETPROPERTYNUMA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TGetPropertyNum
// Return type: double

typedef struct SFPCTGetPropertyNumA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out, retval]
	double Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyNumA;

typedef struct SFPCTGetPropertyNumW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out, retval]
	double Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyNumW;

#ifdef UNICODE
    #define SFPCTGetPropertyNum		SFPCTGetPropertyNumW
#else
    #define SFPCTGetPropertyNum		SFPCTGetPropertyNumA
#endif // UNICODE

#define FPCM_TGETPROPERTYNUMA		(FPCM_FIRST + 299)
#define FPCM_TGETPROPERTYNUMW		(FPCM_FIRST + 300)

#ifdef UNICODE
    #define FPCM_TGETPROPERTYNUM		FPCM_TGETPROPERTYNUMW
#else
    #define FPCM_TGETPROPERTYNUM		FPCM_TGETPROPERTYNUMA
#endif // UNICODE


//================================================================================

//================================================================================
// Method: TGetPropertyAsNumber
// Return type: double

typedef struct SFPCTGetPropertyAsNumberA
{
	// [in] target
	struct
	{
		LPCSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out, retval]
	double Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyAsNumberA;

typedef struct SFPCTGetPropertyAsNumberW
{
	// [in] target
	struct
	{
		LPCWSTR lpszBuffer;
		DWORD dwBufferSize;

	} target;

	// [in] property
	int property;

	// [out, retval]
	double Result;

	// [out]
	HRESULT hr;

} SFPCTGetPropertyAsNumberW;

#ifdef UNICODE
    #define SFPCTGetPropertyAsNumber		SFPCTGetPropertyAsNumberW
#else
    #define SFPCTGetPropertyAsNumber		SFPCTGetPropertyAsNumberA
#endif // UNICODE

#define FPCM_TGETPROPERTYASNUMBERA		(FPCM_FIRST + 301)
#define FPCM_TGETPROPERTYASNUMBERW		(FPCM_FIRST + 302)

#ifdef UNICODE
    #define FPCM_TGETPROPERTYASNUMBER		FPCM_TGETPROPERTYASNUMBERW
#else
    #define FPCM_TGETPROPERTYASNUMBER		FPCM_TGETPROPERTYASNUMBERA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: ReadyState
// Type: long

typedef struct SFPCGetReadyState
{
	// [out]
	long ReadyState;
	// [out]
	HRESULT hr;

} SFPCGetReadyState;

#define FPCM_GET_READYSTATE		(FPCM_FIRST + 303)

//================================================================================

//================================================================================
// Property: TotalFrames
// Type: long

typedef struct SFPCGetTotalFrames
{
	// [out]
	long TotalFrames;
	// [out]
	HRESULT hr;

} SFPCGetTotalFrames;

#define FPCM_GET_TOTALFRAMES		(FPCM_FIRST + 304)

//================================================================================

//================================================================================
// Property: Playing
// Type: VARIANT_BOOL

typedef struct SFPCPutPlaying
{
	// [in]
	VARIANT_BOOL Playing;
	// [out]
	HRESULT hr;

} SFPCPutPlaying;

#define FPCM_PUT_PLAYING		(FPCM_FIRST + 305)

typedef struct SFPCGetPlaying
{
	// [out]
	VARIANT_BOOL Playing;
	// [out]
	HRESULT hr;

} SFPCGetPlaying;

#define FPCM_GET_PLAYING		(FPCM_FIRST + 306)

//================================================================================

//================================================================================
// Property: Quality
// Type: int

typedef struct SFPCPutQuality
{
	// [in]
	int Quality;
	// [out]
	HRESULT hr;

} SFPCPutQuality;

#define FPCM_PUT_QUALITY		(FPCM_FIRST + 307)

typedef struct SFPCGetQuality
{
	// [out]
	int Quality;
	// [out]
	HRESULT hr;

} SFPCGetQuality;

#define FPCM_GET_QUALITY		(FPCM_FIRST + 308)

//================================================================================

//================================================================================
// Property: ScaleMode
// Type: int

typedef struct SFPCPutScaleMode
{
	// [in]
	int ScaleMode;
	// [out]
	HRESULT hr;

} SFPCPutScaleMode;

#define FPCM_PUT_SCALEMODE		(FPCM_FIRST + 309)

typedef struct SFPCGetScaleMode
{
	// [out]
	int ScaleMode;
	// [out]
	HRESULT hr;

} SFPCGetScaleMode;

#define FPCM_GET_SCALEMODE		(FPCM_FIRST + 310)

//================================================================================

//================================================================================
// Property: AlignMode
// Type: int

typedef struct SFPCPutAlignMode
{
	// [in]
	int AlignMode;
	// [out]
	HRESULT hr;

} SFPCPutAlignMode;

#define FPCM_PUT_ALIGNMODE		(FPCM_FIRST + 311)

typedef struct SFPCGetAlignMode
{
	// [out]
	int AlignMode;
	// [out]
	HRESULT hr;

} SFPCGetAlignMode;

#define FPCM_GET_ALIGNMODE		(FPCM_FIRST + 312)

//================================================================================

//================================================================================
// Property: BackgroundColor
// Type: long

typedef struct SFPCPutBackgroundColor
{
	// [in]
	long BackgroundColor;
	// [out]
	HRESULT hr;

} SFPCPutBackgroundColor;

#define FPCM_PUT_BACKGROUNDCOLOR		(FPCM_FIRST + 313)

typedef struct SFPCGetBackgroundColor
{
	// [out]
	long BackgroundColor;
	// [out]
	HRESULT hr;

} SFPCGetBackgroundColor;

#define FPCM_GET_BACKGROUNDCOLOR		(FPCM_FIRST + 314)

//================================================================================

//================================================================================
// Property: Loop
// Type: VARIANT_BOOL

typedef struct SFPCPutLoop
{
	// [in]
	VARIANT_BOOL Loop;
	// [out]
	HRESULT hr;

} SFPCPutLoop;

#define FPCM_PUT_LOOP		(FPCM_FIRST + 315)

typedef struct SFPCGetLoop
{
	// [out]
	VARIANT_BOOL Loop;
	// [out]
	HRESULT hr;

} SFPCGetLoop;

#define FPCM_GET_LOOP		(FPCM_FIRST + 316)

//================================================================================

//================================================================================
// Property: Movie
// Type: BSTR

typedef struct SFPCPutMovieA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutMovieA;

typedef struct SFPCPutMovieW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutMovieW;

#ifdef UNICODE
    #define SFPCPutMovie		SFPCPutMovieW
#else
    #define SFPCPutMovie		SFPCPutMovieA
#endif // UNICODE

#define FPCM_PUT_MOVIEA		(FPCM_FIRST + 317)
#define FPCM_PUT_MOVIEW		(FPCM_FIRST + 318)

#ifdef UNICODE
    #define FPCM_PUT_MOVIE		FPCM_PUT_MOVIEW
#else
    #define FPCM_PUT_MOVIE		FPCM_PUT_MOVIEA
#endif // UNICODE

typedef struct SFPCGetMovieA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetMovieA;

typedef struct SFPCGetMovieW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetMovieW;

#ifdef UNICODE
    #define SFPCGetMovie		SFPCGetMovieW
#else
    #define SFPCGetMovie		SFPCGetMovieA
#endif // UNICODE

#define FPCM_GET_MOVIEA		(FPCM_FIRST + 319)
#define FPCM_GET_MOVIEW		(FPCM_FIRST + 320)

#ifdef UNICODE
    #define FPCM_GET_MOVIE		FPCM_GET_MOVIEW
#else
    #define FPCM_GET_MOVIE		FPCM_GET_MOVIEA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: FrameNum
// Type: long

typedef struct SFPCPutFrameNum
{
	// [in]
	long FrameNum;
	// [out]
	HRESULT hr;

} SFPCPutFrameNum;

#define FPCM_PUT_FRAMENUM		(FPCM_FIRST + 321)

typedef struct SFPCGetFrameNum
{
	// [out]
	long FrameNum;
	// [out]
	HRESULT hr;

} SFPCGetFrameNum;

#define FPCM_GET_FRAMENUM		(FPCM_FIRST + 322)

//================================================================================

//================================================================================
// Property: WMode
// Type: BSTR

typedef struct SFPCPutWModeA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutWModeA;

typedef struct SFPCPutWModeW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutWModeW;

#ifdef UNICODE
    #define SFPCPutWMode		SFPCPutWModeW
#else
    #define SFPCPutWMode		SFPCPutWModeA
#endif // UNICODE

#define FPCM_PUT_WMODEA		(FPCM_FIRST + 323)
#define FPCM_PUT_WMODEW		(FPCM_FIRST + 324)

#ifdef UNICODE
    #define FPCM_PUT_WMODE		FPCM_PUT_WMODEW
#else
    #define FPCM_PUT_WMODE		FPCM_PUT_WMODEA
#endif // UNICODE

typedef struct SFPCGetWModeA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetWModeA;

typedef struct SFPCGetWModeW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetWModeW;

#ifdef UNICODE
    #define SFPCGetWMode		SFPCGetWModeW
#else
    #define SFPCGetWMode		SFPCGetWModeA
#endif // UNICODE

#define FPCM_GET_WMODEA		(FPCM_FIRST + 325)
#define FPCM_GET_WMODEW		(FPCM_FIRST + 326)

#ifdef UNICODE
    #define FPCM_GET_WMODE		FPCM_GET_WMODEW
#else
    #define FPCM_GET_WMODE		FPCM_GET_WMODEA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: SAlign
// Type: BSTR

typedef struct SFPCPutSAlignA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutSAlignA;

typedef struct SFPCPutSAlignW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutSAlignW;

#ifdef UNICODE
    #define SFPCPutSAlign		SFPCPutSAlignW
#else
    #define SFPCPutSAlign		SFPCPutSAlignA
#endif // UNICODE

#define FPCM_PUT_SALIGNA		(FPCM_FIRST + 327)
#define FPCM_PUT_SALIGNW		(FPCM_FIRST + 328)

#ifdef UNICODE
    #define FPCM_PUT_SALIGN		FPCM_PUT_SALIGNW
#else
    #define FPCM_PUT_SALIGN		FPCM_PUT_SALIGNA
#endif // UNICODE

typedef struct SFPCGetSAlignA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetSAlignA;

typedef struct SFPCGetSAlignW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetSAlignW;

#ifdef UNICODE
    #define SFPCGetSAlign		SFPCGetSAlignW
#else
    #define SFPCGetSAlign		SFPCGetSAlignA
#endif // UNICODE

#define FPCM_GET_SALIGNA		(FPCM_FIRST + 329)
#define FPCM_GET_SALIGNW		(FPCM_FIRST + 330)

#ifdef UNICODE
    #define FPCM_GET_SALIGN		FPCM_GET_SALIGNW
#else
    #define FPCM_GET_SALIGN		FPCM_GET_SALIGNA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: Menu
// Type: VARIANT_BOOL

typedef struct SFPCPutMenu
{
	// [in]
	VARIANT_BOOL Menu;
	// [out]
	HRESULT hr;

} SFPCPutMenu;

#define FPCM_PUT_MENU		(FPCM_FIRST + 331)

typedef struct SFPCGetMenu
{
	// [out]
	VARIANT_BOOL Menu;
	// [out]
	HRESULT hr;

} SFPCGetMenu;

#define FPCM_GET_MENU		(FPCM_FIRST + 332)

//================================================================================

//================================================================================
// Property: Base
// Type: BSTR

typedef struct SFPCPutBaseA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutBaseA;

typedef struct SFPCPutBaseW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutBaseW;

#ifdef UNICODE
    #define SFPCPutBase		SFPCPutBaseW
#else
    #define SFPCPutBase		SFPCPutBaseA
#endif // UNICODE

#define FPCM_PUT_BASEA		(FPCM_FIRST + 333)
#define FPCM_PUT_BASEW		(FPCM_FIRST + 334)

#ifdef UNICODE
    #define FPCM_PUT_BASE		FPCM_PUT_BASEW
#else
    #define FPCM_PUT_BASE		FPCM_PUT_BASEA
#endif // UNICODE

typedef struct SFPCGetBaseA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetBaseA;

typedef struct SFPCGetBaseW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetBaseW;

#ifdef UNICODE
    #define SFPCGetBase		SFPCGetBaseW
#else
    #define SFPCGetBase		SFPCGetBaseA
#endif // UNICODE

#define FPCM_GET_BASEA		(FPCM_FIRST + 335)
#define FPCM_GET_BASEW		(FPCM_FIRST + 336)

#ifdef UNICODE
    #define FPCM_GET_BASE		FPCM_GET_BASEW
#else
    #define FPCM_GET_BASE		FPCM_GET_BASEA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: Scale
// Type: BSTR

typedef struct SFPCPutScaleA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutScaleA;

typedef struct SFPCPutScaleW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutScaleW;

#ifdef UNICODE
    #define SFPCPutScale		SFPCPutScaleW
#else
    #define SFPCPutScale		SFPCPutScaleA
#endif // UNICODE

#define FPCM_PUT_SCALEA		(FPCM_FIRST + 337)
#define FPCM_PUT_SCALEW		(FPCM_FIRST + 338)

#ifdef UNICODE
    #define FPCM_PUT_SCALE		FPCM_PUT_SCALEW
#else
    #define FPCM_PUT_SCALE		FPCM_PUT_SCALEA
#endif // UNICODE

typedef struct SFPCGetScaleA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetScaleA;

typedef struct SFPCGetScaleW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetScaleW;

#ifdef UNICODE
    #define SFPCGetScale		SFPCGetScaleW
#else
    #define SFPCGetScale		SFPCGetScaleA
#endif // UNICODE

#define FPCM_GET_SCALEA		(FPCM_FIRST + 339)
#define FPCM_GET_SCALEW		(FPCM_FIRST + 340)

#ifdef UNICODE
    #define FPCM_GET_SCALE		FPCM_GET_SCALEW
#else
    #define FPCM_GET_SCALE		FPCM_GET_SCALEA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: DeviceFont
// Type: VARIANT_BOOL

typedef struct SFPCPutDeviceFont
{
	// [in]
	VARIANT_BOOL DeviceFont;
	// [out]
	HRESULT hr;

} SFPCPutDeviceFont;

#define FPCM_PUT_DEVICEFONT		(FPCM_FIRST + 341)

typedef struct SFPCGetDeviceFont
{
	// [out]
	VARIANT_BOOL DeviceFont;
	// [out]
	HRESULT hr;

} SFPCGetDeviceFont;

#define FPCM_GET_DEVICEFONT		(FPCM_FIRST + 342)

//================================================================================

//================================================================================
// Property: EmbedMovie
// Type: VARIANT_BOOL

typedef struct SFPCPutEmbedMovie
{
	// [in]
	VARIANT_BOOL EmbedMovie;
	// [out]
	HRESULT hr;

} SFPCPutEmbedMovie;

#define FPCM_PUT_EMBEDMOVIE		(FPCM_FIRST + 343)

typedef struct SFPCGetEmbedMovie
{
	// [out]
	VARIANT_BOOL EmbedMovie;
	// [out]
	HRESULT hr;

} SFPCGetEmbedMovie;

#define FPCM_GET_EMBEDMOVIE		(FPCM_FIRST + 344)

//================================================================================

//================================================================================
// Property: BGColor
// Type: BSTR

typedef struct SFPCPutBGColorA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutBGColorA;

typedef struct SFPCPutBGColorW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutBGColorW;

#ifdef UNICODE
    #define SFPCPutBGColor		SFPCPutBGColorW
#else
    #define SFPCPutBGColor		SFPCPutBGColorA
#endif // UNICODE

#define FPCM_PUT_BGCOLORA		(FPCM_FIRST + 345)
#define FPCM_PUT_BGCOLORW		(FPCM_FIRST + 346)

#ifdef UNICODE
    #define FPCM_PUT_BGCOLOR		FPCM_PUT_BGCOLORW
#else
    #define FPCM_PUT_BGCOLOR		FPCM_PUT_BGCOLORA
#endif // UNICODE

typedef struct SFPCGetBGColorA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetBGColorA;

typedef struct SFPCGetBGColorW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetBGColorW;

#ifdef UNICODE
    #define SFPCGetBGColor		SFPCGetBGColorW
#else
    #define SFPCGetBGColor		SFPCGetBGColorA
#endif // UNICODE

#define FPCM_GET_BGCOLORA		(FPCM_FIRST + 347)
#define FPCM_GET_BGCOLORW		(FPCM_FIRST + 348)

#ifdef UNICODE
    #define FPCM_GET_BGCOLOR		FPCM_GET_BGCOLORW
#else
    #define FPCM_GET_BGCOLOR		FPCM_GET_BGCOLORA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: Quality2
// Type: BSTR

typedef struct SFPCPutQuality2A
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutQuality2A;

typedef struct SFPCPutQuality2W
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutQuality2W;

#ifdef UNICODE
    #define SFPCPutQuality2		SFPCPutQuality2W
#else
    #define SFPCPutQuality2		SFPCPutQuality2A
#endif // UNICODE

#define FPCM_PUT_QUALITY2A		(FPCM_FIRST + 349)
#define FPCM_PUT_QUALITY2W		(FPCM_FIRST + 350)

#ifdef UNICODE
    #define FPCM_PUT_QUALITY2		FPCM_PUT_QUALITY2W
#else
    #define FPCM_PUT_QUALITY2		FPCM_PUT_QUALITY2A
#endif // UNICODE

typedef struct SFPCGetQuality2A
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetQuality2A;

typedef struct SFPCGetQuality2W
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetQuality2W;

#ifdef UNICODE
    #define SFPCGetQuality2		SFPCGetQuality2W
#else
    #define SFPCGetQuality2		SFPCGetQuality2A
#endif // UNICODE

#define FPCM_GET_QUALITY2A		(FPCM_FIRST + 351)
#define FPCM_GET_QUALITY2W		(FPCM_FIRST + 352)

#ifdef UNICODE
    #define FPCM_GET_QUALITY2		FPCM_GET_QUALITY2W
#else
    #define FPCM_GET_QUALITY2		FPCM_GET_QUALITY2A
#endif // UNICODE


//================================================================================

//================================================================================
// Property: SWRemote
// Type: BSTR

typedef struct SFPCPutSWRemoteA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutSWRemoteA;

typedef struct SFPCPutSWRemoteW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutSWRemoteW;

#ifdef UNICODE
    #define SFPCPutSWRemote		SFPCPutSWRemoteW
#else
    #define SFPCPutSWRemote		SFPCPutSWRemoteA
#endif // UNICODE

#define FPCM_PUT_SWREMOTEA		(FPCM_FIRST + 353)
#define FPCM_PUT_SWREMOTEW		(FPCM_FIRST + 354)

#ifdef UNICODE
    #define FPCM_PUT_SWREMOTE		FPCM_PUT_SWREMOTEW
#else
    #define FPCM_PUT_SWREMOTE		FPCM_PUT_SWREMOTEA
#endif // UNICODE

typedef struct SFPCGetSWRemoteA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetSWRemoteA;

typedef struct SFPCGetSWRemoteW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetSWRemoteW;

#ifdef UNICODE
    #define SFPCGetSWRemote		SFPCGetSWRemoteW
#else
    #define SFPCGetSWRemote		SFPCGetSWRemoteA
#endif // UNICODE

#define FPCM_GET_SWREMOTEA		(FPCM_FIRST + 355)
#define FPCM_GET_SWREMOTEW		(FPCM_FIRST + 356)

#ifdef UNICODE
    #define FPCM_GET_SWREMOTE		FPCM_GET_SWREMOTEW
#else
    #define FPCM_GET_SWREMOTE		FPCM_GET_SWREMOTEA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: Stacking
// Type: BSTR

typedef struct SFPCPutStackingA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutStackingA;

typedef struct SFPCPutStackingW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutStackingW;

#ifdef UNICODE
    #define SFPCPutStacking		SFPCPutStackingW
#else
    #define SFPCPutStacking		SFPCPutStackingA
#endif // UNICODE

#define FPCM_PUT_STACKINGA		(FPCM_FIRST + 357)
#define FPCM_PUT_STACKINGW		(FPCM_FIRST + 358)

#ifdef UNICODE
    #define FPCM_PUT_STACKING		FPCM_PUT_STACKINGW
#else
    #define FPCM_PUT_STACKING		FPCM_PUT_STACKINGA
#endif // UNICODE

typedef struct SFPCGetStackingA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetStackingA;

typedef struct SFPCGetStackingW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetStackingW;

#ifdef UNICODE
    #define SFPCGetStacking		SFPCGetStackingW
#else
    #define SFPCGetStacking		SFPCGetStackingA
#endif // UNICODE

#define FPCM_GET_STACKINGA		(FPCM_FIRST + 359)
#define FPCM_GET_STACKINGW		(FPCM_FIRST + 360)

#ifdef UNICODE
    #define FPCM_GET_STACKING		FPCM_GET_STACKINGW
#else
    #define FPCM_GET_STACKING		FPCM_GET_STACKINGA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: FlashVars
// Type: BSTR

typedef struct SFPCPutFlashVarsA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutFlashVarsA;

typedef struct SFPCPutFlashVarsW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutFlashVarsW;

#ifdef UNICODE
    #define SFPCPutFlashVars		SFPCPutFlashVarsW
#else
    #define SFPCPutFlashVars		SFPCPutFlashVarsA
#endif // UNICODE

#define FPCM_PUT_FLASHVARSA		(FPCM_FIRST + 361)
#define FPCM_PUT_FLASHVARSW		(FPCM_FIRST + 362)

#ifdef UNICODE
    #define FPCM_PUT_FLASHVARS		FPCM_PUT_FLASHVARSW
#else
    #define FPCM_PUT_FLASHVARS		FPCM_PUT_FLASHVARSA
#endif // UNICODE

typedef struct SFPCGetFlashVarsA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetFlashVarsA;

typedef struct SFPCGetFlashVarsW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetFlashVarsW;

#ifdef UNICODE
    #define SFPCGetFlashVars		SFPCGetFlashVarsW
#else
    #define SFPCGetFlashVars		SFPCGetFlashVarsA
#endif // UNICODE

#define FPCM_GET_FLASHVARSA		(FPCM_FIRST + 363)
#define FPCM_GET_FLASHVARSW		(FPCM_FIRST + 364)

#ifdef UNICODE
    #define FPCM_GET_FLASHVARS		FPCM_GET_FLASHVARSW
#else
    #define FPCM_GET_FLASHVARS		FPCM_GET_FLASHVARSA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: AllowScriptAccess
// Type: BSTR

typedef struct SFPCPutAllowScriptAccessA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutAllowScriptAccessA;

typedef struct SFPCPutAllowScriptAccessW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutAllowScriptAccessW;

#ifdef UNICODE
    #define SFPCPutAllowScriptAccess		SFPCPutAllowScriptAccessW
#else
    #define SFPCPutAllowScriptAccess		SFPCPutAllowScriptAccessA
#endif // UNICODE

#define FPCM_PUT_ALLOWSCRIPTACCESSA		(FPCM_FIRST + 365)
#define FPCM_PUT_ALLOWSCRIPTACCESSW		(FPCM_FIRST + 366)

#ifdef UNICODE
    #define FPCM_PUT_ALLOWSCRIPTACCESS		FPCM_PUT_ALLOWSCRIPTACCESSW
#else
    #define FPCM_PUT_ALLOWSCRIPTACCESS		FPCM_PUT_ALLOWSCRIPTACCESSA
#endif // UNICODE

typedef struct SFPCGetAllowScriptAccessA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetAllowScriptAccessA;

typedef struct SFPCGetAllowScriptAccessW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetAllowScriptAccessW;

#ifdef UNICODE
    #define SFPCGetAllowScriptAccess		SFPCGetAllowScriptAccessW
#else
    #define SFPCGetAllowScriptAccess		SFPCGetAllowScriptAccessA
#endif // UNICODE

#define FPCM_GET_ALLOWSCRIPTACCESSA		(FPCM_FIRST + 367)
#define FPCM_GET_ALLOWSCRIPTACCESSW		(FPCM_FIRST + 368)

#ifdef UNICODE
    #define FPCM_GET_ALLOWSCRIPTACCESS		FPCM_GET_ALLOWSCRIPTACCESSW
#else
    #define FPCM_GET_ALLOWSCRIPTACCESS		FPCM_GET_ALLOWSCRIPTACCESSA
#endif // UNICODE


//================================================================================

//================================================================================
// Property: MovieData
// Type: BSTR

typedef struct SFPCPutMovieDataA
{
	// [in]
	LPCSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutMovieDataA;

typedef struct SFPCPutMovieDataW
{
	// [in]
	LPCWSTR lpszBuffer;
	// [out]
	HRESULT hr;

} SFPCPutMovieDataW;

#ifdef UNICODE
    #define SFPCPutMovieData		SFPCPutMovieDataW
#else
    #define SFPCPutMovieData		SFPCPutMovieDataA
#endif // UNICODE

#define FPCM_PUT_MOVIEDATAA		(FPCM_FIRST + 369)
#define FPCM_PUT_MOVIEDATAW		(FPCM_FIRST + 370)

#ifdef UNICODE
    #define FPCM_PUT_MOVIEDATA		FPCM_PUT_MOVIEDATAW
#else
    #define FPCM_PUT_MOVIEDATA		FPCM_PUT_MOVIEDATAA
#endif // UNICODE

typedef struct SFPCGetMovieDataA
{
	// [in, out]
	LPSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetMovieDataA;

typedef struct SFPCGetMovieDataW
{
	// [in, out]
	LPWSTR lpszBuffer;
	DWORD dwBufferSize;

	// [out]
	HRESULT hr;

} SFPCGetMovieDataW;

#ifdef UNICODE
    #define SFPCGetMovieData		SFPCGetMovieDataW
#else
    #define SFPCGetMovieData		SFPCGetMovieDataA
#endif // UNICODE

#define FPCM_GET_MOVIEDATAA		(FPCM_FIRST + 371)
#define FPCM_GET_MOVIEDATAW		(FPCM_FIRST + 372)

#ifdef UNICODE
    #define FPCM_GET_MOVIEDATA		FPCM_GET_MOVIEDATAW
#else
    #define FPCM_GET_MOVIEDATA		FPCM_GET_MOVIEDATAA
#endif // UNICODE


//================================================================================

