#pragma once

namespace ParaEngine
{
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

	typedef enum _D3DLIGHTTYPE {
		D3DLIGHT_POINT = 1,
		D3DLIGHT_SPOT = 2,
		D3DLIGHT_DIRECTIONAL = 3,
		D3DLIGHT_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DLIGHTTYPE;

	typedef enum _D3DCULL {
		D3DCULL_NONE = 1,
		D3DCULL_CW = 2,
		D3DCULL_CCW = 3,
		D3DCULL_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DCULL;

	// emulate direct input 8 structure
	typedef struct DIDEVICEOBJECTDATA {
		DWORD       dwOfs;
		DWORD       dwData;
		DWORD       dwTimeStamp;
		DWORD       dwSequence;
		DWORD		uAppData;
	} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

	typedef struct _DIMOUSESTATE {
		uint32    lX;
		uint32    lY;
		uint32    lZ;
		BYTE    rgbButtons[4];
	} DIMOUSESTATE, *LPDIMOUSESTATE;

	// emulate direct mouse
	typedef struct _DIMOUSESTATE2 {
		uint32    lX;
		uint32    lY;
		uint32    lZ;
		BYTE    rgbButtons[8];
	} DIMOUSESTATE2, *LPDIMOUSESTATE2;

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((uint32_t)offsetof(type, field))
#endif
	
	enum DIMOFS_ENUM
	{
		DIMOFS_X = FIELD_OFFSET(DIMOUSESTATE, lX),
		DIMOFS_Y = FIELD_OFFSET(DIMOUSESTATE, lY),
		DIMOFS_Z = FIELD_OFFSET(DIMOUSESTATE, lZ),
		DIMOFS_BUTTON0 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 0),
		DIMOFS_BUTTON1 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 1),
		DIMOFS_BUTTON2 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 2),
		DIMOFS_BUTTON3 = (FIELD_OFFSET(DIMOUSESTATE, rgbButtons) + 3)
	};
	

	typedef enum _D3DFORMAT
	{
		D3DFMT_UNKNOWN = 0,

		D3DFMT_R8G8B8 = 20,
		D3DFMT_A8R8G8B8 = 21,
		D3DFMT_X8R8G8B8 = 22,
		D3DFMT_A8 = 28,
		D3DFMT_A8B8G8R8 = 32,
		D3DFMT_X8B8G8R8 = 33,

		D3DFMT_DXT1 = MAKEFOURCC('D', 'X', 'T', '1'),
		D3DFMT_DXT2 = MAKEFOURCC('D', 'X', 'T', '2'),
		D3DFMT_DXT3 = MAKEFOURCC('D', 'X', 'T', '3'),
		D3DFMT_DXT4 = MAKEFOURCC('D', 'X', 'T', '4'),
		D3DFMT_DXT5 = MAKEFOURCC('D', 'X', 'T', '5'),

		D3DFMT_D32 = 71,
		D3DFMT_D15S1 = 73,
		D3DFMT_D24S8 = 75,
		D3DFMT_D24X8 = 77,
		D3DFMT_D24X4S4 = 79,
		D3DFMT_D16 = 80,

		D3DFMT_VERTEXDATA = 100,
		D3DFMT_INDEX16 = 101,
		D3DFMT_INDEX32 = 102,

		
	} D3DFORMAT;

	// Primitives supported by draw-primitive API
	typedef enum _D3DPRIMITIVETYPE {
		D3DPT_POINTLIST             = 1,
		D3DPT_LINELIST              = 2,
		D3DPT_LINESTRIP             = 3,
		D3DPT_TRIANGLELIST          = 4,
		D3DPT_TRIANGLESTRIP         = 5,
		D3DPT_TRIANGLEFAN           = 6,
		D3DPT_FORCE_DWORD           = 0x7fffffff, /* force 32-bit size enum */
	} D3DPRIMITIVETYPE;

	typedef enum _D3DTEXTUREADDRESS {
		D3DTADDRESS_WRAP = 1,
		D3DTADDRESS_MIRROR = 2,
		D3DTADDRESS_CLAMP = 3,
		D3DTADDRESS_BORDER = 4,
		D3DTADDRESS_MIRRORONCE = 5,
		D3DTADDRESS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DTEXTUREADDRESS;

	typedef enum _D3DTEXTUREFILTERTYPE
	{
		D3DTEXF_NONE = 0,    // filtering disabled (valid for mip filter only)
		D3DTEXF_POINT = 1,    // nearest
		D3DTEXF_LINEAR = 2,    // linear interpolation
		D3DTEXF_ANISOTROPIC = 3,    // anisotropic
		D3DTEXF_PYRAMIDALQUAD = 6,    // 4-sample tent
		D3DTEXF_GAUSSIANQUAD = 7,    // 4-sample gaussian
		/* -- D3D9Ex only */
		D3DTEXF_FORCE_DWORD = 0x7fffffff,   // force 32-bit size enum
	} D3DTEXTUREFILTERTYPE;

	typedef struct _D3DVIEWPORT9 {
		DWORD       X;
		DWORD       Y;            /* Viewport Top left */
		DWORD       Width;
		DWORD       Height;       /* Viewport Dimensions */
		float       MinZ;         /* Min/max of clip Volume */
		float       MaxZ;
	} D3DVIEWPORT9;

	/*
	* State enumerants for per-stage processing of fixed function pixel processing
	* Two of these affect fixed function vertex processing as well: TEXTURETRANSFORMFLAGS and TEXCOORDINDEX.
	*/
	typedef enum _D3DTEXTURESTAGESTATETYPE
	{
		D3DTSS_COLOROP = 1, /* D3DTEXTUREOP - per-stage blending controls for color channels */
		D3DTSS_COLORARG1 = 2, /* D3DTA_* (texture arg) */
		D3DTSS_COLORARG2 = 3, /* D3DTA_* (texture arg) */
		D3DTSS_ALPHAOP = 4, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
		D3DTSS_ALPHAARG1 = 5, /* D3DTA_* (texture arg) */
		D3DTSS_ALPHAARG2 = 6, /* D3DTA_* (texture arg) */
		D3DTSS_BUMPENVMAT00 = 7, /* float (bump mapping matrix) */
		D3DTSS_BUMPENVMAT01 = 8, /* float (bump mapping matrix) */
		D3DTSS_BUMPENVMAT10 = 9, /* float (bump mapping matrix) */
		D3DTSS_BUMPENVMAT11 = 10, /* float (bump mapping matrix) */
		D3DTSS_TEXCOORDINDEX = 11, /* identifies which set of texture coordinates index this texture */
		D3DTSS_BUMPENVLSCALE = 22, /* float scale for bump map luminance */
		D3DTSS_BUMPENVLOFFSET = 23, /* float offset for bump map luminance */
		D3DTSS_TEXTURETRANSFORMFLAGS = 24, /* D3DTEXTURETRANSFORMFLAGS controls texture transform */
		D3DTSS_COLORARG0 = 26, /* D3DTA_* third arg for triadic ops */
		D3DTSS_ALPHAARG0 = 27, /* D3DTA_* third arg for triadic ops */
		D3DTSS_RESULTARG = 28, /* D3DTA_* arg for result (CURRENT or TEMP) */
		D3DTSS_CONSTANT = 32, /* Per-stage constant D3DTA_CONSTANT */


		D3DTSS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DTEXTURESTAGESTATETYPE;

	typedef enum _D3DCMPFUNC {
		D3DCMP_NEVER = 1,
		D3DCMP_LESS = 2,
		D3DCMP_EQUAL = 3,
		D3DCMP_LESSEQUAL = 4,
		D3DCMP_GREATER = 5,
		D3DCMP_NOTEQUAL = 6,
		D3DCMP_GREATEREQUAL = 7,
		D3DCMP_ALWAYS = 8,
		D3DCMP_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DCMPFUNC;

	typedef enum _D3DSTENCILOP {
		D3DSTENCILOP_KEEP = 1,
		D3DSTENCILOP_ZERO = 2,
		D3DSTENCILOP_REPLACE = 3,
		D3DSTENCILOP_INCRSAT = 4,
		D3DSTENCILOP_DECRSAT = 5,
		D3DSTENCILOP_INVERT = 6,
		D3DSTENCILOP_INCR = 7,
		D3DSTENCILOP_DECR = 8,
		D3DSTENCILOP_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DSTENCILOP;

	/*
	* Enumerations for COLOROP and ALPHAOP texture blending operations set in
	* texture processing stage controls in D3DTSS.
	*/
	typedef enum _D3DTEXTUREOP
	{
		// Control
		D3DTOP_DISABLE = 1,      // disables stage
		D3DTOP_SELECTARG1 = 2,      // the default
		D3DTOP_SELECTARG2 = 3,

		// Modulate
		D3DTOP_MODULATE = 4,      // multiply args together
		D3DTOP_MODULATE2X = 5,      // multiply and  1 bit
		D3DTOP_MODULATE4X = 6,      // multiply and  2 bits

		// Add
		D3DTOP_ADD = 7,   // add arguments together
		D3DTOP_ADDSIGNED = 8,   // add with -0.5 bias
		D3DTOP_ADDSIGNED2X = 9,   // as above but left  1 bit
		D3DTOP_SUBTRACT = 10,   // Arg1 - Arg2, with no saturation
		D3DTOP_ADDSMOOTH = 11,   // add 2 args, subtract product
		// Arg1 + Arg2 - Arg1*Arg2
		// = Arg1 + (1-Arg1)*Arg2

		// Linear alpha blend: Arg1*(Alpha) + Arg2*(1-Alpha)
		D3DTOP_BLENDDIFFUSEALPHA = 12, // iterated alpha
		D3DTOP_BLENDTEXTUREALPHA = 13, // texture alpha
		D3DTOP_BLENDFACTORALPHA = 14, // alpha from D3DRS_TEXTUREFACTOR

		// Linear alpha blend with pre-multiplied arg1 input: Arg1 + Arg2*(1-Alpha)
		D3DTOP_BLENDTEXTUREALPHAPM = 15, // texture alpha
		D3DTOP_BLENDCURRENTALPHA = 16, // by alpha of current color

		// Specular mapping
		D3DTOP_PREMODULATE = 17,     // modulate with next texture before use
		D3DTOP_MODULATEALPHA_ADDCOLOR = 18,     // Arg1.RGB + Arg1.A*Arg2.RGB
		// COLOROP only
		D3DTOP_MODULATECOLOR_ADDALPHA = 19,     // Arg1.RGB*Arg2.RGB + Arg1.A
		// COLOROP only
		D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20,  // (1-Arg1.A)*Arg2.RGB + Arg1.RGB
		// COLOROP only
		D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21,  // (1-Arg1.RGB)*Arg2.RGB + Arg1.A
		// COLOROP only

		// Bump mapping
		D3DTOP_BUMPENVMAP = 22, // per pixel env map perturbation
		D3DTOP_BUMPENVMAPLUMINANCE = 23, // with luminance channel

		// This can do either diffuse or specular bump mapping with correct input.
		// Performs the function (Arg1.R*Arg2.R + Arg1.G*Arg2.G + Arg1.B*Arg2.B)
		// where each component has been scaled and offset to make it signed.
		// The result is replicated into all four (including alpha) channels.
		// This is a valid COLOROP only.
		D3DTOP_DOTPRODUCT3 = 24,

		// Triadic ops
		D3DTOP_MULTIPLYADD = 25, // Arg0 + Arg1*Arg2
		D3DTOP_LERP = 26, // (Arg0)*Arg1 + (1-Arg0)*Arg2

		D3DTOP_FORCE_DWORD = 0x7fffffff,
	} D3DTEXTUREOP;

	typedef enum _D3DTRANSFORMSTATETYPE {
		D3DTS_VIEW = 2,
		D3DTS_PROJECTION = 3,
		D3DTS_TEXTURE0 = 16,
		D3DTS_TEXTURE1 = 17,
		D3DTS_TEXTURE2 = 18,
		D3DTS_TEXTURE3 = 19,
		D3DTS_TEXTURE4 = 20,
		D3DTS_TEXTURE5 = 21,
		D3DTS_TEXTURE6 = 22,
		D3DTS_TEXTURE7 = 23,
		D3DTS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DTRANSFORMSTATETYPE;

	typedef enum _D3DDECLTYPE
	{
		D3DDECLTYPE_FLOAT1 = 0,  // 1D float expanded to (value, 0., 0., 1.)
		D3DDECLTYPE_FLOAT2 = 1,  // 2D float expanded to (value, value, 0., 1.)
		D3DDECLTYPE_FLOAT3 = 2,  // 3D float expanded to (value, value, value, 1.)
		D3DDECLTYPE_FLOAT4 = 3,  // 4D float
		D3DDECLTYPE_D3DCOLOR = 4,  // 4D packed unsigned bytes mapped to 0. to 1. range
		// Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
		D3DDECLTYPE_UBYTE4 = 5,  // 4D unsigned byte
		D3DDECLTYPE_SHORT2 = 6,  // 2D signed short expanded to (value, value, 0., 1.)
		D3DDECLTYPE_SHORT4 = 7,  // 4D signed short

		// The following types are valid only with vertex shaders >= 2.0


		D3DDECLTYPE_UBYTE4N = 8,  // Each of 4 bytes is normalized by dividing to 255.0
		D3DDECLTYPE_SHORT2N = 9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
		D3DDECLTYPE_SHORT4N = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
		D3DDECLTYPE_USHORT2N = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
		D3DDECLTYPE_USHORT4N = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
		D3DDECLTYPE_UDEC3 = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
		D3DDECLTYPE_DEC3N = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
		D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
		D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values
		D3DDECLTYPE_UNUSED = 17,  // When the type field in a decl is unused.
	} D3DDECLTYPE;

	typedef enum _D3DDECLUSAGE
	{
		D3DDECLUSAGE_POSITION = 0,
		D3DDECLUSAGE_BLENDWEIGHT,   // 1
		D3DDECLUSAGE_BLENDINDICES,  // 2
		D3DDECLUSAGE_NORMAL,        // 3
		D3DDECLUSAGE_PSIZE,         // 4
		D3DDECLUSAGE_TEXCOORD,      // 5
		D3DDECLUSAGE_TANGENT,       // 6
		D3DDECLUSAGE_BINORMAL,      // 7
		D3DDECLUSAGE_TESSFACTOR,    // 8
		D3DDECLUSAGE_POSITIONT,     // 9
		D3DDECLUSAGE_COLOR,         // 10
		D3DDECLUSAGE_FOG,           // 11
		D3DDECLUSAGE_DEPTH,         // 12
		D3DDECLUSAGE_SAMPLE,        // 13
	} D3DDECLUSAGE;

	typedef enum _D3DBLEND {
		D3DBLEND_ZERO = 1,
		D3DBLEND_ONE = 2,
		D3DBLEND_SRCCOLOR = 3,
		D3DBLEND_INVSRCCOLOR = 4,
		D3DBLEND_SRCALPHA = 5,
		D3DBLEND_INVSRCALPHA = 6,
		D3DBLEND_DESTALPHA = 7,
		D3DBLEND_INVDESTALPHA = 8,
		D3DBLEND_DESTCOLOR = 9,
		D3DBLEND_INVDESTCOLOR = 10,
		D3DBLEND_SRCALPHASAT = 11,
		D3DBLEND_BOTHSRCALPHA = 12,
		D3DBLEND_BOTHINVSRCALPHA = 13,
		D3DBLEND_BLENDFACTOR = 14, /* Only supported if D3DPBLENDCAPS_BLENDFACTOR is on */
		D3DBLEND_INVBLENDFACTOR = 15, /* Only supported if D3DPBLENDCAPS_BLENDFACTOR is on */
		/* -- D3D9Ex only */
		D3DBLEND_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DBLEND;

	struct VertexElement;

	// This is used to initialize the last vertex element in a vertex declaration
	// array
	//
#define D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED,0,0,0}

#define D3DTS_WORLDMATRIX(index) (D3DTRANSFORMSTATETYPE)(index + 256)
#define D3DTS_WORLD  D3DTS_WORLDMATRIX(0)
#define D3DTS_WORLD1 D3DTS_WORLDMATRIX(1)
#define D3DTS_WORLD2 D3DTS_WORLDMATRIX(2)
#define D3DTS_WORLD3 D3DTS_WORLDMATRIX(3)

	typedef enum _D3DRENDERSTATETYPE {
		D3DRS_ZENABLE = 7,    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
		D3DRS_FILLMODE = 8,    /* D3DFILLMODE */
		D3DRS_SHADEMODE = 9,    /* D3DSHADEMODE */
		D3DRS_ZWRITEENABLE = 14,   /* TRUE to enable z writes */
		D3DRS_ALPHATESTENABLE = 15,   /* TRUE to enable alpha tests */
		D3DRS_LASTPIXEL = 16,   /* TRUE for last-pixel on lines */
		D3DRS_SRCBLEND = 19,   /* D3DBLEND */
		D3DRS_DESTBLEND = 20,   /* D3DBLEND */
		D3DRS_CULLMODE = 22,   /* D3DCULL */
		D3DRS_ZFUNC = 23,   /* D3DCMPFUNC */
		D3DRS_ALPHAREF = 24,   /* D3DFIXED */
		D3DRS_ALPHAFUNC = 25,   /* D3DCMPFUNC */
		D3DRS_DITHERENABLE = 26,   /* TRUE to enable dithering */
		D3DRS_ALPHABLENDENABLE = 27,   /* TRUE to enable alpha blending */
		D3DRS_FOGENABLE = 28,   /* TRUE to enable fog blending */
		D3DRS_SPECULARENABLE = 29,   /* TRUE to enable specular */
		D3DRS_FOGCOLOR = 34,   /* D3DCOLOR */
		D3DRS_FOGTABLEMODE = 35,   /* D3DFOGMODE */
		D3DRS_FOGSTART = 36,   /* Fog start (for both vertex and pixel fog) */
		D3DRS_FOGEND = 37,   /* Fog end      */
		D3DRS_FOGDENSITY = 38,   /* Fog density  */
		D3DRS_RANGEFOGENABLE = 48,   /* Enables range-based fog */
		D3DRS_STENCILENABLE = 52,   /* BOOL enable/disable stenciling */
		D3DRS_STENCILFAIL = 53,   /* D3DSTENCILOP to do if stencil test fails */
		D3DRS_STENCILZFAIL = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		D3DRS_STENCILPASS = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		D3DRS_STENCILFUNC = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		D3DRS_STENCILREF = 57,   /* Reference value used in stencil test */
		D3DRS_STENCILMASK = 58,   /* Mask value used in stencil test */
		D3DRS_STENCILWRITEMASK = 59,   /* Write mask applied to values written to stencil buffer */
		D3DRS_TEXTUREFACTOR = 60,   /* D3DCOLOR used for multi-texture blend */
		D3DRS_WRAP0 = 128,  /* wrap for 1st texture coord. set */
		D3DRS_WRAP1 = 129,  /* wrap for 2nd texture coord. set */
		D3DRS_WRAP2 = 130,  /* wrap for 3rd texture coord. set */
		D3DRS_WRAP3 = 131,  /* wrap for 4th texture coord. set */
		D3DRS_WRAP4 = 132,  /* wrap for 5th texture coord. set */
		D3DRS_WRAP5 = 133,  /* wrap for 6th texture coord. set */
		D3DRS_WRAP6 = 134,  /* wrap for 7th texture coord. set */
		D3DRS_WRAP7 = 135,  /* wrap for 8th texture coord. set */
		D3DRS_CLIPPING = 136,
		D3DRS_LIGHTING = 137,
		D3DRS_AMBIENT = 139,
		D3DRS_FOGVERTEXMODE = 140,
		D3DRS_COLORVERTEX = 141,
		D3DRS_LOCALVIEWER = 142,
		D3DRS_NORMALIZENORMALS = 143,
		D3DRS_DIFFUSEMATERIALSOURCE = 145,
		D3DRS_SPECULARMATERIALSOURCE = 146,
		D3DRS_AMBIENTMATERIALSOURCE = 147,
		D3DRS_EMISSIVEMATERIALSOURCE = 148,
		D3DRS_VERTEXBLEND = 151,
		D3DRS_CLIPPLANEENABLE = 152,
		D3DRS_POINTSIZE = 154,   /* float point size */
		D3DRS_POINTSIZE_MIN = 155,   /* float point size min threshold */
		D3DRS_POINTSPRITEENABLE = 156,   /* BOOL point texture coord control */
		D3DRS_POINTSCALEENABLE = 157,   /* BOOL point size scale enable */
		D3DRS_POINTSCALE_A = 158,   /* float point attenuation A value */
		D3DRS_POINTSCALE_B = 159,   /* float point attenuation B value */
		D3DRS_POINTSCALE_C = 160,   /* float point attenuation C value */
		D3DRS_MULTISAMPLEANTIALIAS = 161,  // BOOL - set to do FSAA with multisample buffer
		D3DRS_MULTISAMPLEMASK = 162,  // DWORD - per-sample enable/disable
		D3DRS_PATCHEDGESTYLE = 163,  // Sets whether patch edges will use float style tessellation
		D3DRS_DEBUGMONITORTOKEN = 165,  // DEBUG ONLY - token to debug monitor
		D3DRS_POINTSIZE_MAX = 166,   /* float point size max threshold */
		D3DRS_INDEXEDVERTEXBLENDENABLE = 167,
		D3DRS_COLORWRITEENABLE = 168,  // per-channel write enable
		D3DRS_TWEENFACTOR = 170,   // float tween factor
		D3DRS_BLENDOP = 171,   // D3DBLENDOP setting
		D3DRS_POSITIONDEGREE = 172,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		D3DRS_NORMALDEGREE = 173,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		D3DRS_SCISSORTESTENABLE = 174,
		D3DRS_SLOPESCALEDEPTHBIAS = 175,
		D3DRS_ANTIALIASEDLINEENABLE = 176,
		D3DRS_MINTESSELLATIONLEVEL = 178,
		D3DRS_MAXTESSELLATIONLEVEL = 179,
		D3DRS_ADAPTIVETESS_X = 180,
		D3DRS_ADAPTIVETESS_Y = 181,
		D3DRS_ADAPTIVETESS_Z = 182,
		D3DRS_ADAPTIVETESS_W = 183,
		D3DRS_ENABLEADAPTIVETESSELLATION = 184,
		D3DRS_TWOSIDEDSTENCILMODE = 185,   /* BOOL enable/disable 2 sided stenciling */
		D3DRS_CCW_STENCILFAIL = 186,   /* D3DSTENCILOP to do if ccw stencil test fails */
		D3DRS_CCW_STENCILZFAIL = 187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		D3DRS_CCW_STENCILPASS = 188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		D3DRS_CCW_STENCILFUNC = 189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		D3DRS_COLORWRITEENABLE1 = 190,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_COLORWRITEENABLE2 = 191,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_COLORWRITEENABLE3 = 192,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DRS_BLENDFACTOR = 193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		D3DRS_SRGBWRITEENABLE = 194,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		D3DRS_DEPTHBIAS = 195,
		D3DRS_WRAP8 = 198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		D3DRS_WRAP9 = 199,
		D3DRS_WRAP10 = 200,
		D3DRS_WRAP11 = 201,
		D3DRS_WRAP12 = 202,
		D3DRS_WRAP13 = 203,
		D3DRS_WRAP14 = 204,
		D3DRS_WRAP15 = 205,
		D3DRS_SEPARATEALPHABLENDENABLE = 206,  /* TRUE to enable a separate blending function for the alpha channel */
		D3DRS_SRCBLENDALPHA = 207,  /* SRC blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DRS_DESTBLENDALPHA = 208,  /* DST blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DRS_BLENDOPALPHA = 209,  /* Blending operation for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */


		D3DRS_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} D3DRENDERSTATETYPE;

	/*
	 * State enumerants for per-sampler texture processing.
	 */
	typedef enum _D3DSAMPLERSTATETYPE
	{
		D3DSAMP_ADDRESSU       = 1,  /* D3DTEXTUREADDRESS for U coordinate */
		D3DSAMP_ADDRESSV       = 2,  /* D3DTEXTUREADDRESS for V coordinate */
		D3DSAMP_ADDRESSW       = 3,  /* D3DTEXTUREADDRESS for W coordinate */
		D3DSAMP_BORDERCOLOR    = 4,  /* D3DCOLOR */
		D3DSAMP_MAGFILTER      = 5,  /* D3DTEXTUREFILTER filter to use for magnification */
		D3DSAMP_MINFILTER      = 6,  /* D3DTEXTUREFILTER filter to use for minification */
		D3DSAMP_MIPFILTER      = 7,  /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
		D3DSAMP_MIPMAPLODBIAS  = 8,  /* float Mipmap LOD bias */
		D3DSAMP_MAXMIPLEVEL    = 9,  /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
		D3DSAMP_MAXANISOTROPY  = 10, /* DWORD maximum anisotropy */
		D3DSAMP_SRGBTEXTURE    = 11, /* Default = 0 (which means Gamma 1.0,
									   no correction required.) else correct for
									   Gamma = 2.2 */
		D3DSAMP_ELEMENTINDEX   = 12, /* When multi-element texture is assigned to sampler, this
										indicates which element index to use.  Default = 0.  */
		D3DSAMP_DMAPOFFSET     = 13, /* Offset in vertices in the pre-sampled displacement map.
										Only valid for D3DDMAPSAMPLER sampler  */
		D3DSAMP_FORCE_DWORD   = 0x7fffffff, /* force 32-bit size enum */
	} D3DSAMPLERSTATETYPE;

	typedef enum _D3DDECLMETHOD
	{
		D3DDECLMETHOD_DEFAULT = 0,
		D3DDECLMETHOD_PARTIALU,
		D3DDECLMETHOD_PARTIALV,
		D3DDECLMETHOD_CROSSUV,    // Normal
		D3DDECLMETHOD_UV,
		D3DDECLMETHOD_LOOKUP,               // Lookup a displacement map
		D3DDECLMETHOD_LOOKUPPRESAMPLED,     // Lookup a pre-sampled displacement map
	} D3DDECLMETHOD;

	/* Pool types */
	typedef enum _D3DPOOL {
		D3DPOOL_DEFAULT = 0,
		D3DPOOL_MANAGED = 1,
		D3DPOOL_SYSTEMMEM = 2,
		D3DPOOL_SCRATCH = 3,
		D3DPOOL_FORCE_DWORD = 0x7fffffff
	} D3DPOOL;
}

#include "VirtualKeyDef.h"

// Flexible vertex format bits
//
#define D3DFVF_RESERVED0        0x001
#define D3DFVF_POSITION_MASK    0x400E
#define D3DFVF_XYZ              0x002
#define D3DFVF_XYZRHW           0x004
#define D3DFVF_XYZB1            0x006
#define D3DFVF_XYZB2            0x008
#define D3DFVF_XYZB3            0x00a
#define D3DFVF_XYZB4            0x00c
#define D3DFVF_XYZB5            0x00e
#define D3DFVF_XYZW             0x4002

#define D3DFVF_NORMAL           0x010
#define D3DFVF_PSIZE            0x020
#define D3DFVF_DIFFUSE          0x040
#define D3DFVF_SPECULAR         0x080

#define D3DFVF_TEXCOUNT_MASK    0xf00
#define D3DFVF_TEXCOUNT_SHIFT   8
#define D3DFVF_TEX0             0x000
#define D3DFVF_TEX1             0x100
#define D3DFVF_TEX2             0x200
#define D3DFVF_TEX3             0x300
#define D3DFVF_TEX4             0x400
#define D3DFVF_TEX5             0x500
#define D3DFVF_TEX6             0x600
#define D3DFVF_TEX7             0x700
#define D3DFVF_TEX8             0x800

/* Usages for Vertex/Index buffers */
#define D3DUSAGE_WRITEONLY          (0x00000008L)
#define D3DUSAGE_SOFTWAREPROCESSING (0x00000010L)
#define D3DUSAGE_DONOTCLIP          (0x00000020L)
#define D3DUSAGE_POINTS             (0x00000040L)
#define D3DUSAGE_RTPATCHES          (0x00000080L)
#define D3DUSAGE_NPATCHES           (0x00000100L)

#define D3DUSAGE_RENDERTARGET       (0x00000001L)
#define D3DUSAGE_DEPTHSTENCIL       (0x00000002L)
#define D3DUSAGE_DYNAMIC            (0x00000200L)

#define D3DX_DEFAULT            ((UINT) -1)

/*
* Options for clearing
*/
#define D3DCLEAR_TARGET            0x00000001l  /* Clear target surface */
#define D3DCLEAR_ZBUFFER           0x00000002l  /* Clear target z buffer */
#define D3DCLEAR_STENCIL           0x00000004l  /* Clear stencil planes */

#define D3DTA_SELECTMASK        0x0000000f  // mask for arg selector
#define D3DTA_DIFFUSE           0x00000000  // select diffuse color (read only)
#define D3DTA_CURRENT           0x00000001  // select stage destination register (read/write)
#define D3DTA_TEXTURE           0x00000002  // select texture color (read only)


#define D3DXSPRITE_DONOTSAVESTATE               (1 << 0)
#define D3DXSPRITE_DONOTMODIFY_RENDERSTATE      (1 << 1)
#define D3DXSPRITE_OBJECTSPACE                  (1 << 2)
#define D3DXSPRITE_BILLBOARD                    (1 << 3)
#define D3DXSPRITE_ALPHABLEND                   (1 << 4)
#define D3DXSPRITE_SORT_TEXTURE                 (1 << 5)
#define D3DXSPRITE_SORT_DEPTH_FRONTTOBACK       (1 << 6)
#define D3DXSPRITE_SORT_DEPTH_BACKTOFRONT       (1 << 7)
#define D3DXSPRITE_DO_NOT_ADDREF_TEXTURE        (1 << 8)

#define D3DLOCK_READONLY           0x00000010L
#define D3DLOCK_DISCARD            0x00002000L
#define D3DLOCK_NOOVERWRITE        0x00001000L
#define D3DLOCK_NOSYSLOCK          0x00000800L
#define D3DLOCK_DONOTWAIT          0x00004000L                  
#define D3DLOCK_NO_DIRTY_UPDATE     0x00008000L

#define D3DCURSOR_IMMEDIATE_UPDATE             0x00000001L

#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0
#define PE_CHECK_GL_ERROR_DEBUG()
#else
#define PE_CHECK_GL_ERROR_DEBUG() RenderDevice::CheckRenderError(__FILE__, __FUNCTION__, __LINE__);
#endif
