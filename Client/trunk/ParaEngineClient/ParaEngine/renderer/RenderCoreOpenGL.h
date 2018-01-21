#pragma  once
#include "RenderDeviceOpenGL.h"

//#pragma once
//
//#ifndef DIRECTINPUT_VERSION
//namespace ParaEngine
//{
//#ifndef FIELD_OFFSET
//#define FIELD_OFFSET(type, field)    ((uint32_t)offsetof(type, field))
//#endif
//
//
//	typedef struct _DIMOUSESTATE {
//		uint32    lX;
//		uint32    lY;
//		uint32    lZ;
//		BYTE    rgbButtons[4];
//	} DIMOUSESTATE, *LPDIMOUSESTATE;
//
//	// emulate direct mouse
//	typedef struct _DIMOUSESTATE2 {
//		uint32    lX;
//		uint32    lY;
//		uint32    lZ;
//		BYTE    rgbButtons[8];
//	} DIMOUSESTATE2, *LPDIMOUSESTATE2;
//
//	enum DIMOFS_ENUM
//	{
//		DIMOFS_X = FIELD_OFFSET(DIMOUSESTATE, lX),
//		DIMOFS_Y = FIELD_OFFSET(DIMOUSESTATE, lY),
//		DIMOFS_Z = FIELD_OFFSET(DIMOUSESTATE, lZ),
//		DIMOFS_BUTTON0 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 0),
//		DIMOFS_BUTTON1 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 1),
//		DIMOFS_BUTTON2 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 2),
//		DIMOFS_BUTTON3 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 3)
//	};
//
//	// emulate direct input 8 structure
//	typedef struct DIDEVICEOBJECTDATA {
//		DWORD       dwOfs;
//		DWORD       dwData;
//		DWORD       dwTimeStamp;
//		DWORD       dwSequence;
//		DWORD		uAppData;
//	} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;
//
//
//
//
//}
//#endif
//
//namespace ParaEngine
//{
//#ifndef MAKEFOURCC
//#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
//                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
//                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
//#endif /* defined(MAKEFOURCC) */
//
//	typedef enum _D3DLIGHTTYPE {
//		D3DLIGHT_POINT = 1,
//		D3DLIGHT_SPOT = 2,
//		D3DLIGHT_DIRECTIONAL = 3,
//		D3DLIGHT_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DLIGHTTYPE;
//
//	typedef enum _D3DCULL {
//		RSV_CULL_NONE = 1,
//		RSV_CULL_CW = 2,
//		RSV_CULL_CCW = 3,
//		D3DCULL_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DCULL;
//
//
//
//
//	typedef enum _D3DFORMAT
//	{
//		D3DFMT_UNKNOWN = 0,
//
//		D3DFMT_R8G8B8 = 20,
//		D3DFMT_A8R8G8B8 = 21,
//		D3DFMT_X8R8G8B8 = 22,
//		D3DFMT_A8 = 28,
//		D3DFMT_A8B8G8R8 = 32,
//		D3DFMT_X8B8G8R8 = 33,
//
//		D3DFMT_DXT1 = MAKEFOURCC('D', 'X', 'T', '1'),
//		D3DFMT_DXT2 = MAKEFOURCC('D', 'X', 'T', '2'),
//		D3DFMT_DXT3 = MAKEFOURCC('D', 'X', 'T', '3'),
//		D3DFMT_DXT4 = MAKEFOURCC('D', 'X', 'T', '4'),
//		D3DFMT_DXT5 = MAKEFOURCC('D', 'X', 'T', '5'),
//
//		D3DFMT_D32 = 71,
//		D3DFMT_D15S1 = 73,
//		D3DFMT_D24S8 = 75,
//		D3DFMT_D24X8 = 77,
//		D3DFMT_D24X4S4 = 79,
//		D3DFMT_D16 = 80,
//
//		D3DFMT_VERTEXDATA = 100,
//		D3DFMT_INDEX16 = 101,
//		D3DFMT_INDEX32 = 102,
//
//		
//	} D3DFORMAT;
//
//	// Primitives supported by draw-primitive API
//	typedef enum _D3DPRIMITIVETYPE {
//		D3DPT_POINTLIST             = 1,
//		D3DPT_LINELIST              = 2,
//		D3DPT_LINESTRIP             = 3,
//		D3DPT_TRIANGLELIST          = 4,
//		D3DPT_TRIANGLESTRIP         = 5,
//		D3DPT_TRIANGLEFAN           = 6,
//		D3DPT_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
//	} D3DPRIMITIVETYPE;
//
//	typedef enum _D3DTEXTUREADDRESS {
//		D3DTADDRESS_WRAP = 1,
//		D3DTADDRESS_MIRROR = 2,
//		D3DTADDRESS_CLAMP = 3,
//		D3DTADDRESS_BORDER = 4,
//		D3DTADDRESS_MIRRORONCE = 5,
//		D3DTADDRESS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DTEXTUREADDRESS;
//
//	typedef enum _D3DTEXTUREFILTERTYPE
//	{
//		D3DTEXF_NONE = 0,    // filtering disabled (valid for mip filter only)
//		D3DTEXF_POINT = 1,    // nearest
//		D3DTEXF_LINEAR = 2,    // linear interpolation
//		D3DTEXF_ANISOTROPIC = 3,    // anisotropic
//		D3DTEXF_PYRAMIDALQUAD = 6,    // 4-sample tent
//		D3DTEXF_GAUSSIANQUAD = 7,    // 4-sample gaussian
//		/* -- D3D9Ex only */
//		D3DTEXF_FORCE_DWORD = 0x7fffffff,   // force 32-bit size enum
//	} D3DTEXTUREFILTERTYPE;
//
//	typedef struct _D3DVIEWPORT9 {
//		DWORD       X;
//		DWORD       Y;            /* Viewport Top left */
//		DWORD       Width;
//		DWORD       Height;       /* Viewport Dimensions */
//		float       MinZ;         /* Min/max of clip Volume */
//		float       MaxZ;
//	} D3DVIEWPORT9;
//
//	/*
//	* State enumerants for per-stage processing of fixed function pixel processing
//	* Two of these affect fixed function vertex processing as well: TEXTURETRANSFORMFLAGS and TEXCOORDINDEX.
//	*/
//	typedef enum _D3DTEXTURESTAGESTATETYPE
//	{
//		D3DTSS_COLOROP = 1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
//		D3DTSS_COLORARG1 = 2, /* D3DTA_* (texture arg) */
//		D3DTSS_COLORARG2 = 3, /* D3DTA_* (texture arg) */
//		D3DTSS_ALPHAOP = 4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
//		D3DTSS_ALPHAARG1 = 5, /* D3DTA_* (texture arg) */
//		D3DTSS_ALPHAARG2 = 6, /* D3DTA_* (texture arg) */
//		D3DTSS_BUMPENVMAT00 = 7, /* float (bump mapping matrix) */
//		D3DTSS_BUMPENVMAT01 = 8, /* float (bump mapping matrix) */
//		D3DTSS_BUMPENVMAT10 = 9, /* float (bump mapping matrix) */
//		D3DTSS_BUMPENVMAT11 = 10, /* float (bump mapping matrix) */
//		D3DTSS_TEXCOORDINDEX = 11, /* identifies which set of texture coordinates index this texture */
//		D3DTSS_BUMPENVLSCALE = 22, /* float scale for bump map luminance */
//		D3DTSS_BUMPENVLOFFSET = 23, /* float offset for bump map luminance */
//		D3DTSS_TEXTURETRANSFORMFLAGS = 24, /* D3DTEXTURETRANSFORMFLAGS controls texture transform */
//		D3DTSS_COLORARG0 = 26, /* D3DTA_* third arg for triadic ops */
//		D3DTSS_ALPHAARG0 = 27, /* D3DTA_* third arg for triadic ops */
//		D3DTSS_RESULTARG = 28, /* D3DTA_* arg for result (CURRENT or TEMP) */
//		D3DTSS_CONSTANT = 32, /* Per-stage constant D3DTA_CONSTANT */
//
//
//		D3DTSS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DTEXTURESTAGESTATETYPE;
//
//	typedef enum _D3DCMPFUNC {
//		D3DCMP_NEVER = 1,
//		D3DCMP_LESS = 2,
//		D3DCMP_EQUAL = 3,
//		D3DCMP_LESSEQUAL = 4,
//		D3DCMP_GREATER = 5,
//		D3DCMP_NOTEQUAL = 6,
//		D3DCMP_GREATEREQUAL = 7,
//		D3DCMP_ALWAYS = 8,
//		D3DCMP_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DCMPFUNC;
//
//	typedef enum _D3DSTENCILOP {
//		D3DSTENCILOP_KEEP = 1,
//		D3DSTENCILOP_ZERO = 2,
//		D3DSTENCILOP_REPLACE = 3,
//		D3DSTENCILOP_INCRSAT = 4,
//		D3DSTENCILOP_DECRSAT = 5,
//		D3DSTENCILOP_INVERT = 6,
//		D3DSTENCILOP_INCR = 7,
//		D3DSTENCILOP_DECR = 8,
//		D3DSTENCILOP_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DSTENCILOP;
//
//	/*
//	* Enumerations for COLOROP and ALPHAOP texture blending operations set in
//	* texture processing stage controls in D3DTSS.
//	*/
//	typedef enum _D3DTEXTUREOP
//	{
//		// Control
//		D3DTOP_DISABLE = 1,      // disables stage
//		D3DTOP_SELECTARG1 = 2,      // the default
//		D3DTOP_SELECTARG2 = 3,
//
//		// Modulate
//		D3DTOP_MODULATE = 4,      // multiply args together
//		D3DTOP_MODULATE2X = 5,      // multiply and  1 bit
//		D3DTOP_MODULATE4X = 6,      // multiply and  2 bits
//
//		// Add
//		D3DTOP_ADD = 7,   // add arguments together
//		D3DTOP_ADDSIGNED = 8,   // add with -0.5 bias
//		D3DTOP_ADDSIGNED2X = 9,   // as above but left  1 bit
//		D3DTOP_SUBTRACT = 10,   // Arg1 - Arg2, with no saturation
//		D3DTOP_ADDSMOOTH = 11,   // add 2 args, subtract product
//		// Arg1 + Arg2 - Arg1*Arg2
//		// = Arg1 + (1-Arg1)*Arg2
//
//		// Linear alpha blend: Arg1*(Alpha) + Arg2*(1-Alpha)
//		D3DTOP_BLENDDIFFUSEALPHA = 12, // iterated alpha
//		D3DTOP_BLENDTEXTUREALPHA = 13, // texture alpha
//		D3DTOP_BLENDFACTORALPHA = 14, // alpha from ERenderState::TEXTUREFACTOR
//
//		// Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
//		D3DTOP_BLENDTEXTUREALPHAPM = 15, // texture alpha
//		D3DTOP_BLENDCURRENTALPHA = 16, // by alpha of current color
//
//		// Specular mapping
//		D3DTOP_PREMODULATE = 17,     // modulate with next texture before use
//		D3DTOP_MODULATEALPHA_ADDCOLOR = 18,     // Arg1.RGB + Arg1.A*Arg2.RGB
//		// COLOROP only
//		D3DTOP_MODULATECOLOR_ADDALPHA = 19,     // Arg1.RGB*Arg2.RGB + Arg1.A
//		// COLOROP only
//		D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20,  // (1-Arg1.A)*Arg2.RGB + Arg1.RGB
//		// COLOROP only
//		D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21,  // (1-Arg1.RGB)*Arg2.RGB + Arg1.A
//		// COLOROP only
//
//		// Bump mapping
//		D3DTOP_BUMPENVMAP = 22, // per pixel env map perturbation
//		D3DTOP_BUMPENVMAPLUMINANCE = 23, // with luminance channel
//
//		// This can do either diffuse or specular bump mapping with correct input.
//		// Performs the function (Arg1.R*Arg2.R + Arg1.G*Arg2.G + Arg1.B*Arg2.B)
//		// where each component has been scaled and offset to make it signed.
//		// The result is replicated into all four (including alpha) channels.
//		// This is a valid COLOROP only.
//		D3DTOP_DOTPRODUCT3 = 24,
//
//		// Triadic ops
//		D3DTOP_MULTIPLYADD = 25, // Arg0 + Arg1*Arg2
//		D3DTOP_LERP = 26, // (Arg0)*Arg1 + (1-Arg0)*Arg2
//
//		D3DTOP_FORCE_DWORD = 0x7fffffff,
//	} D3DTEXTUREOP;
//
//	typedef enum _D3DTRANSFORMSTATETYPE {
//		D3DTS_VIEW = 2,
//		D3DTS_PROJECTION = 3,
//		D3DTS_TEXTURE0 = 16,
//		D3DTS_TEXTURE1 = 17,
//		D3DTS_TEXTURE2 = 18,
//		D3DTS_TEXTURE3 = 19,
//		D3DTS_TEXTURE4 = 20,
//		D3DTS_TEXTURE5 = 21,
//		D3DTS_TEXTURE6 = 22,
//		D3DTS_TEXTURE7 = 23,
//		D3DTS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DTRANSFORMSTATETYPE;
//
//	typedef enum _D3DDECLTYPE
//	{
//		D3DDECLTYPE_FLOAT1 = 0,  // 1D float expanded to (value, 0., 0., 1.)
//		D3DDECLTYPE_FLOAT2 = 1,  // 2D float expanded to (value, value, 0., 1.)
//		D3DDECLTYPE_FLOAT3 = 2,  // 3D float expanded to (value, value, value, 1.)
//		D3DDECLTYPE_FLOAT4 = 3,  // 4D float
//		D3DDECLTYPE_D3DCOLOR = 4,  // 4D packed unsigned bytes mapped to 0. to 1. range
//		// Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
//		D3DDECLTYPE_UBYTE4 = 5,  // 4D unsigned byte
//		D3DDECLTYPE_SHORT2 = 6,  // 2D signed short expanded to (value, value, 0., 1.)
//		D3DDECLTYPE_SHORT4 = 7,  // 4D signed short
//
//		// The following types are valid only with vertex shaders >= 2.0
//
//
//		D3DDECLTYPE_UBYTE4N = 8,  // Each of 4 bytes is normalized by dividing to 255.0
//		D3DDECLTYPE_SHORT2N = 9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
//		D3DDECLTYPE_SHORT4N = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
//		D3DDECLTYPE_USHORT2N = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
//		D3DDECLTYPE_USHORT4N = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
//		D3DDECLTYPE_UDEC3 = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
//		D3DDECLTYPE_DEC3N = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
//		D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
//		D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values
//		D3DDECLTYPE_UNUSED = 17,  // When the type field in a decl is unused.
//	} D3DDECLTYPE;
//
//	typedef enum _D3DDECLUSAGE
//	{
//		D3DDECLUSAGE_POSITION = 0,
//		D3DDECLUSAGE_BLENDWEIGHT,   // 1
//		D3DDECLUSAGE_BLENDINDICES,  // 2
//		D3DDECLUSAGE_NORMAL,        // 3
//		D3DDECLUSAGE_PSIZE,         // 4
//		D3DDECLUSAGE_TEXCOORD,      // 5
//		D3DDECLUSAGE_TANGENT,       // 6
//		D3DDECLUSAGE_BINORMAL,      // 7
//		D3DDECLUSAGE_TESSFACTOR,    // 8
//		D3DDECLUSAGE_POSITIONT,     // 9
//		D3DDECLUSAGE_COLOR,         // 10
//		D3DDECLUSAGE_FOG,           // 11
//		D3DDECLUSAGE_DEPTH,         // 12
//		D3DDECLUSAGE_SAMPLE,        // 13
//	} D3DDECLUSAGE;
//
//	typedef enum _D3DBLEND {
//		D3DBLEND_ZERO = 1,
//		D3DBLEND_ONE = 2,
//		D3DBLEND_SRCCOLOR = 3,
//		D3DBLEND_INVSRCCOLOR = 4,
//		D3DBLEND_SRCALPHA = 5,
//		D3DBLEND_INVSRCALPHA = 6,
//		D3DBLEND_DESTALPHA = 7,
//		D3DBLEND_INVDESTALPHA = 8,
//		D3DBLEND_DESTCOLOR = 9,
//		D3DBLEND_INVDESTCOLOR = 10,
//		D3DBLEND_SRCALPHASAT = 11,
//		D3DBLEND_BOTHSRCALPHA = 12,
//		D3DBLEND_BOTHINVSRCALPHA = 13,
//		D3DBLEND_BLENDFACTOR = 14, /* Only supported if D3DPBLENDCAPS_BLENDFACTOR is on */
//		D3DBLEND_INVBLENDFACTOR = 15, /* Only supported if D3DPBLENDCAPS_BLENDFACTOR is on */
//		/* -- D3D9Ex only */
//		D3DBLEND_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DBLEND;
//
//	struct VertexElement;
//
//	// This is used to initialize the last vertex element in a vertex declaration
//	// array
//	//
//#define D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}
//
//#define D3DTS_WORLDMATRIX(index) (D3DTRANSFORMSTATETYPE)(index + 256)
//#define D3DTS_WORLD  D3DTS_WORLDMATRIX(0)
//#define D3DTS_WORLD1 D3DTS_WORLDMATRIX(1)
//#define D3DTS_WORLD2 D3DTS_WORLDMATRIX(2)
//#define D3DTS_WORLD3 D3DTS_WORLDMATRIX(3)
//
//	typedef enum _D3DRENDERSTATETYPE {
//		ERenderState::ZENABLE = 7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
//		ERenderState::FILLMODE = 8,    /* D3DFILLMODE */
//		ERenderState::SHADEMODE = 9,    /* D3DSHADEMODE */
//		ERenderState::ZWRITEENABLE = 14,   /* TRUE to enable z writes */
//		ERenderState::ALPHATESTENABLE = 15,   /* TRUE to enable alpha tests */
//		ERenderState::LASTPIXEL = 16,   /* TRUE for last-pixel on lines */
//		ERenderState::SRCBLEND = 19,   /* D3DBLEND */
//		ERenderState::DESTBLEND = 20,   /* D3DBLEND */
//		ERenderState::CULLMODE = 22,   /* D3DCULL */
//		ERenderState::ZFUNC = 23,   /* D3DCMPFUNC */
//		ERenderState::ALPHAREF = 24,   /* D3DFIXED */
//		ERenderState::ALPHAFUNC = 25,   /* D3DCMPFUNC */
//		ERenderState::DITHERENABLE = 26,   /* TRUE to enable dithering */
//		ERenderState::ALPHABLENDENABLE = 27,   /* TRUE to enable alpha blending */
//		ERenderState::FOGENABLE = 28,   /* TRUE to enable fog blending */
//		ERenderState::SPECULARENABLE = 29,   /* TRUE to enable specular */
//		ERenderState::FOGCOLOR = 34,   /* D3DCOLOR */
//		ERenderState::FOGTABLEMODE = 35,   /* D3DFOGMODE */
//		ERenderState::FOGSTART = 36,   /* Fog start (for both vertex and pixel fog) */
//		ERenderState::FOGEND = 37,   /* Fog end      */
//		ERenderState::FOGDENSITY = 38,   /* Fog density  */
//		ERenderState::RANGEFOGENABLE = 48,   /* Enables range-based fog */
//		ERenderState::STENCILENABLE = 52,   /* BOOL enable/disable stenciling */
//		ERenderState::STENCILFAIL = 53,   /* D3DSTENCILOP to do if stencil test fails */
//		ERenderState::STENCILZFAIL = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
//		ERenderState::STENCILPASS = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
//		ERenderState::STENCILFUNC = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
//		ERenderState::STENCILREF = 57,   /* Reference value used in stencil test */
//		ERenderState::STENCILMASK = 58,   /* Mask value used in stencil test */
//		ERenderState::STENCILWRITEMASK = 59,   /* Write mask applied to values written to stencil buffer */
//		ERenderState::TEXTUREFACTOR = 60,   /* D3DCOLOR used for multi-texture blend */
//		ERenderState::WRAP0 = 128,  /* wrap for 1st texture coord. set */
//		ERenderState::WRAP1 = 129,  /* wrap for 2nd texture coord. set */
//		ERenderState::WRAP2 = 130,  /* wrap for 3rd texture coord. set */
//		ERenderState::WRAP3 = 131,  /* wrap for 4th texture coord. set */
//		ERenderState::WRAP4 = 132,  /* wrap for 5th texture coord. set */
//		ERenderState::WRAP5 = 133,  /* wrap for 6th texture coord. set */
//		ERenderState::WRAP6 = 134,  /* wrap for 7th texture coord. set */
//		ERenderState::WRAP7 = 135,  /* wrap for 8th texture coord. set */
//		ERenderState::CLIPPING = 136,
//		ERenderState::LIGHTING = 137,
//		ERenderState::AMBIENT = 139,
//		ERenderState::FOGVERTEXMODE = 140,
//		ERenderState::COLORVERTEX = 141,
//		ERenderState::LOCALVIEWER = 142,
//		ERenderState::NORMALIZENORMALS = 143,
//		ERenderState::DIFFUSEMATERIALSOURCE = 145,
//		ERenderState::SPECULARMATERIALSOURCE = 146,
//		ERenderState::AMBIENTMATERIALSOURCE = 147,
//		ERenderState::EMISSIVEMATERIALSOURCE = 148,
//		ERenderState::VERTEXBLEND = 151,
//		ERenderState::CLIPPLANEENABLE = 152,
//		ERenderState::POINTSIZE = 154,   /* float point size */
//		ERenderState::POINTSIZE_MIN = 155,   /* float point size min threshold */
//		ERenderState::POINTSPRITEENABLE = 156,   /* BOOL point texture coord control */
//		ERenderState::POINTSCALEENABLE = 157,   /* BOOL point size scale enable */
//		ERenderState::POINTSCALE_A = 158,   /* float point attenuation A value */
//		ERenderState::POINTSCALE_B = 159,   /* float point attenuation B value */
//		ERenderState::POINTSCALE_C = 160,   /* float point attenuation C value */
//		ERenderState::MULTISAMPLEANTIALIAS = 161,  // BOOL - set to do FSAA with multisample buffer
//		ERenderState::MULTISAMPLEMASK = 162,  // DWORD - per-sample enable/disable
//		ERenderState::PATCHEDGESTYLE = 163,  // Sets whether patch edges will use float style tessellation
//		ERenderState::DEBUGMONITORTOKEN = 165,  // DEBUG ONLY - token to debug monitor
//		ERenderState::POINTSIZE_MAX = 166,   /* float point size max threshold */
//		ERenderState::INDEXEDVERTEXBLENDENABLE = 167,
//		ERenderState::COLORWRITEENABLE = 168,  // per-channel write enable
//		ERenderState::TWEENFACTOR = 170,   // float tween factor
//		ERenderState::BLENDOP = 171,   // D3DBLENDOP setting
//		ERenderState::POSITIONDEGREE = 172,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
//		ERenderState::NORMALDEGREE = 173,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
//		ERenderState::SCISSORTESTENABLE = 174,
//		ERenderState::SLOPESCALEDEPTHBIAS = 175,
//		ERenderState::ANTIALIASEDLINEENABLE = 176,
//		ERenderState::MINTESSELLATIONLEVEL = 178,
//		ERenderState::MAXTESSELLATIONLEVEL = 179,
//		ERenderState::ADAPTIVETESS_X = 180,
//		ERenderState::ADAPTIVETESS_Y = 181,
//		ERenderState::ADAPTIVETESS_Z = 182,
//		ERenderState::ADAPTIVETESS_W = 183,
//		ERenderState::ENABLEADAPTIVETESSELLATION = 184,
//		ERenderState::TWOSIDEDSTENCILMODE = 185,   /* BOOL enable/disable 2 sided stenciling */
//		ERenderState::CCW_STENCILFAIL = 186,   /* D3DSTENCILOP to do if ccw stencil test fails */
//		ERenderState::CCW_STENCILZFAIL = 187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
//		ERenderState::CCW_STENCILPASS = 188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
//		ERenderState::CCW_STENCILFUNC = 189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
//		ERenderState::COLORWRITEENABLE1 = 190,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
//		ERenderState::COLORWRITEENABLE2 = 191,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
//		ERenderState::COLORWRITEENABLE3 = 192,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
//		ERenderState::BLENDFACTOR = 193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
//		ERenderState::SRGBWRITEENABLE = 194,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
//		ERenderState::DEPTHBIAS = 195,
//		ERenderState::WRAP8 = 198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
//		ERenderState::WRAP9 = 199,
//		ERenderState::WRAP10 = 200,
//		ERenderState::WRAP11 = 201,
//		ERenderState::WRAP12 = 202,
//		ERenderState::WRAP13 = 203,
//		ERenderState::WRAP14 = 204,
//		ERenderState::WRAP15 = 205,
//		ERenderState::SEPARATEALPHABLENDENABLE = 206,  /* TRUE to enable a separate blending function for the alpha channel */
//		ERenderState::SRCBLENDALPHA = 207,  /* SRC blend factor for the alpha channel when ERenderState::SEPARATEDESTALPHAENABLE is TRUE */
//		ERenderState::DESTBLENDALPHA = 208,  /* DST blend factor for the alpha channel when ERenderState::SEPARATEDESTALPHAENABLE is TRUE */
//		ERenderState::BLENDOPALPHA = 209,  /* Blending operation for the alpha channel when ERenderState::SEPARATEDESTALPHAENABLE is TRUE */
//
//
//		ERenderState::FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
//	} D3DRENDERSTATETYPE;
//
//	/*
//	 * State enumerants for per-sampler texture processing.
//	 */
//	typedef enum _D3DSAMPLERSTATETYPE
//	{
//		D3DSAMP_ADDRESSU       = 1,  /* D3DTEXTUREADDRESS for U coordinate */
//		D3DSAMP_ADDRESSV       = 2,  /* D3DTEXTUREADDRESS for V coordinate */
//		D3DSAMP_ADDRESSW       = 3,  /* D3DTEXTUREADDRESS for W coordinate */
//		D3DSAMP_BORDERCOLOR    = 4,  /* D3DCOLOR */
//		D3DSAMP_MAGFILTER      = 5,  /* D3DTEXTUREFILTER filter to use for magnification */
//		D3DSAMP_MINFILTER      = 6,  /* D3DTEXTUREFILTER filter to use for minification */
//		D3DSAMP_MIPFILTER      = 7,  /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
//		D3DSAMP_MIPMAPLODBIAS  = 8,  /* float Mipmap LOD bias */
//		D3DSAMP_MAXMIPLEVEL    = 9,  /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
//		D3DSAMP_MAXANISOTROPY  = 10, /* DWORD maximum anisotropy */
//		D3DSAMP_SRGBTEXTURE    = 11, /* Default = 0 (which means Gamma 1.0,
//									   no correction required.) else correct for
//									   Gamma = 2.2 */
//		D3DSAMP_ELEMENTINDEX   = 12, /* When multi-element texture is assigned to sampler, this
//										indicates which element index to use.  Default = 0.  */
//		D3DSAMP_DMAPOFFSET     = 13, /* Offset in vertices in the pre-sampled displacement map.
//										Only valid for D3DDMAPSAMPLER sampler  */
//		D3DSAMP_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */
//	} D3DSAMPLERSTATETYPE;
//
//	typedef enum _D3DDECLMETHOD
//	{
//		D3DDECLMETHOD_DEFAULT = 0,
//		D3DDECLMETHOD_PARTIALU,
//		D3DDECLMETHOD_PARTIALV,
//		D3DDECLMETHOD_CROSSUV,    // Normal
//		D3DDECLMETHOD_UV,
//		D3DDECLMETHOD_LOOKUP,               // Lookup a displacement map
//		D3DDECLMETHOD_LOOKUPPRESAMPLED,     // Lookup a pre-sampled displacement map
//	} D3DDECLMETHOD;
//
//	/* Pool types */
//	typedef enum _D3DPOOL {
//		D3DPOOL_DEFAULT = 0,
//		D3DPOOL_MANAGED = 1,
//		D3DPOOL_SYSTEMMEM = 2,
//		D3DPOOL_SCRATCH = 3,
//		D3DPOOL_FORCE_DWORD = 0x7fffffff
//	} D3DPOOL;
//}
//
//#include "VirtualKeyDef.h"
//
//// Flexible vertex format bits
////
//#define D3DFVF_RESERVED0        0x001
//#define D3DFVF_POSITION_MASK    0x400E
//#define D3DFVF_XYZ              0x002
//#define D3DFVF_XYZRHW           0x004
//#define D3DFVF_XYZB1            0x006
//#define D3DFVF_XYZB2            0x008
//#define D3DFVF_XYZB3            0x00a
//#define D3DFVF_XYZB4            0x00c
//#define D3DFVF_XYZB5            0x00e
//#define D3DFVF_XYZW             0x4002
//
//#define D3DFVF_NORMAL           0x010
//#define D3DFVF_PSIZE            0x020
//#define D3DFVF_DIFFUSE          0x040
//#define D3DFVF_SPECULAR         0x080
//
//#define D3DFVF_TEXCOUNT_MASK    0xf00
//#define D3DFVF_TEXCOUNT_SHIFT   8
//#define D3DFVF_TEX0             0x000
//#define D3DFVF_TEX1             0x100
//#define D3DFVF_TEX2             0x200
//#define D3DFVF_TEX3             0x300
//#define D3DFVF_TEX4             0x400
//#define D3DFVF_TEX5             0x500
//#define D3DFVF_TEX6             0x600
//#define D3DFVF_TEX7             0x700
//#define D3DFVF_TEX8             0x800
//
///* Usages for Vertex/Index buffers */
//#define D3DUSAGE_WRITEONLY          (0x00000008L)
//#define D3DUSAGE_SOFTWAREPROCESSING (0x00000010L)
//#define D3DUSAGE_DONOTCLIP          (0x00000020L)
//#define D3DUSAGE_POINTS             (0x00000040L)
//#define D3DUSAGE_RTPATCHES          (0x00000080L)
//#define D3DUSAGE_NPATCHES           (0x00000100L)
//
//#define D3DUSAGE_RENDERTARGET       (0x00000001L)
//#define D3DUSAGE_DEPTHSTENCIL       (0x00000002L)
//#define D3DUSAGE_DYNAMIC            (0x00000200L)
//
//#define D3DX_DEFAULT            ((UINT) -1)
//
///*
//* Options for clearing
//*/
//#define D3DCLEAR_TARGET            0x00000001l  /* Clear target surface */
//#define D3DCLEAR_ZBUFFER           0x00000002l  /* Clear target z buffer */
//#define D3DCLEAR_STENCIL           0x00000004l  /* Clear stencil planes */
//
//#define D3DTA_SELECTMASK        0x0000000f  // mask for arg selector
//#define D3DTA_DIFFUSE           0x00000000  // select diffuse color (read only)
//#define D3DTA_CURRENT           0x00000001  // select stage destination register (read/write)
//#define D3DTA_TEXTURE           0x00000002  // select texture color (read only)
//
//
//#define D3DXSPRITE_DONOTSAVESTATE               (1 << 0)
//#define D3DXSPRITE_DONOTMODIFY_RENDERSTATE      (1 << 1)
//#define D3DXSPRITE_OBJECTSPACE                  (1 << 2)
//#define D3DXSPRITE_BILLBOARD                    (1 << 3)
//#define D3DXSPRITE_ALPHABLEND                   (1 << 4)
//#define D3DXSPRITE_SORT_TEXTURE                 (1 << 5)
//#define D3DXSPRITE_SORT_DEPTH_FRONTTOBACK       (1 << 6)
//#define D3DXSPRITE_SORT_DEPTH_BACKTOFRONT       (1 << 7)
//#define D3DXSPRITE_DO_NOT_ADDREF_TEXTURE        (1 << 8)
//
//#define D3DLOCK_READONLY           0x00000010L
//#define D3DLOCK_DISCARD            0x00002000L
//#define D3DLOCK_NOOVERWRITE        0x00001000L
//#define D3DLOCK_NOSYSLOCK          0x00000800L
//#define D3DLOCK_DONOTWAIT          0x00004000L                  
//#define D3DLOCK_NO_DIRTY_UPDATE     0x00008000L
//
//#define D3DCURSOR_IMMEDIATE_UPDATE             0x00000001L
//
//#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0
//#define PE_CHECK_GL_ERROR_DEBUG()
//#else
//#define PE_CHECK_GL_ERROR_DEBUG() RenderDevice::CheckRenderError(__FILE__, __FUNCTION__, __LINE__);
//#endif
