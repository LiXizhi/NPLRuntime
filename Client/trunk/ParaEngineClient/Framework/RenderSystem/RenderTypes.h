#pragma  once
#include "IRenderWindow.h"
namespace ParaEngine
{
	/* Formats
	* Most of these names have the following convention:
	*      A = Alpha
	*      R = Red
	*      G = Green
	*      B = Blue
	*      X = Unused Bits
	*      P = Palette
	*      L = Luminance
	*      U = dU coordinate for BumpMap
	*      V = dV coordinate for BumpMap
	*      S = Stencil
	*      D = Depth (e.g. Z or W buffer)
	*      C = Computed from other channels (typically on certain read operations)
	*
	*      Further, the order of the pieces are from MSB first; hence
	*      A8L8 indicates that the high byte of this two byte
	*      format is alpha.
	*
	*      D16_LOCKABLE indicates:
	*           - An integer 16-bit value.
	*           - An app-lockable surface.
	*
	*      D32F_LOCKABLE indicates:
	*           - An IEEE 754 floating-point value.
	*           - An app-lockable surface.
	*
	*      All Depth/Stencil formats except D16_LOCKABLE and D32F_LOCKABLE indicate:
	*          - no particular bit ordering per pixel, and
	*          - are not app lockable, and
	*          - the driver is allowed to consume more than the indicated
	*            number of bits per Depth channel (but not Stencil channel).
	*/
	enum class PixelFormat
	{
		Unkonwn,
		R8G8B8,
		A8R8G8B8,
		X8R8G8B8,
		R5G6B5,
		X1R5G5B5,
		A1R5G5B5,
		A4R4G4B4,
		R3G3B2,
		A8,
		A8R3G3B2,
		X4R4G4B4,
		A2B10G10R10,
		A8B8G8R8,

		X8B8G8R8,
		G16R16,
		A2R10G10B10,
		A16B16G16R16,

		A8P8,
		P8,

		L8,
		A8L8,
		A4L4,

		V8U8,
		L6V5U5,
		X8L8V8U8,
		Q8W8V8U8,
		V16U16,
		A2W10V10U10,

		UYVY,
		R8G8_B8G8,
		YUY2,
		G8R8_G8B8,
		DXT1,
		DXT2,
		DXT3,
		DXT4,
		DXT5,


		D16_LOCKABLE,
		D32,
		D15S1,
		D24S8,
		D24X8,
		D24X4S4,
		D16,
		D32F_LOCKABLE,
		D24FS8,
		COUNT
	};

	enum class MultiSampleType
	{
		None = 0,
		NONMASKABLE = 1,
		Samples_2 = 2,
		Samples_3 = 3,
		Samples_4 = 4,
		Samples_5 = 5,
		Samples_6 = 6,
		Samples_7 = 7,
		Samples_8 = 8,
		Samples_9 = 9,
		Samples_10 = 10,
		Samples_11 = 11,
		Samples_12 = 12,
		Samples_13 = 13,
		Samples_14 = 14,
		Samples_15 = 15,
		Samples_16 = 16,
	};



	enum class ERenderState
	{
		ZENABLE = 7,
		FILLMODE = 8,
		SHADEMODE = 9,
		ZWRITEENABLE = 14,   /* TRUE to enable z writes */
		ALPHATESTENABLE = 15,   /* TRUE to enable alpha tests */
		LASTPIXEL = 16,   /* TRUE for last-pixel on lines */
		SRCBLEND = 19,   /* D3DBLEND */
		DESTBLEND = 20,   /* D3DBLEND */
		CULLMODE = 22,   /* D3DCULL */
		ZFUNC = 23,   /* D3DCMPFUNC */
		ALPHAREF = 24,   /* D3DFIXED */
		ALPHAFUNC = 25,   /* D3DCMPFUNC */
		DITHERENABLE = 26,   /* TRUE to enable dithering */
		ALPHABLENDENABLE = 27,   /* TRUE to enable alpha blending */
		FOGENABLE = 28,   /* TRUE to enable fog blending */
		SPECULARENABLE = 29,   /* TRUE to enable specular */
		FOGCOLOR = 34,   /* D3DCOLOR */
		FOGTABLEMODE = 35,   /* D3DFOGMODE */
		FOGSTART = 36,   /* Fog start (for both vertex and pixel fog) */
		FOGEND = 37,   /* Fog end      */
		FOGDENSITY = 38,   /* Fog density  */
		RANGEFOGENABLE = 48,   /* Enables range-based fog */
		STENCILENABLE = 52,   /* BOOL enable/disable stenciling */
		STENCILFAIL = 53,   /* D3DSTENCILOP to do if stencil test fails */
		STENCILZFAIL = 54,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		STENCILPASS = 55,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		STENCILFUNC = 56,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		STENCILREF = 57,   /* Reference value used in stencil test */
		STENCILMASK = 58,   /* Mask value used in stencil test */
		STENCILWRITEMASK = 59,   /* Write mask applied to values written to stencil buffer */
		TEXTUREFACTOR = 60,   /* D3DCOLOR used for multi-texture blend */
		WRAP0 = 128,  /* wrap for 1st texture coord. set */
		WRAP1 = 129,  /* wrap for 2nd texture coord. set */
		WRAP2 = 130,  /* wrap for 3rd texture coord. set */
		WRAP3 = 131,  /* wrap for 4th texture coord. set */
		WRAP4 = 132,  /* wrap for 5th texture coord. set */
		WRAP5 = 133,  /* wrap for 6th texture coord. set */
		WRAP6 = 134,  /* wrap for 7th texture coord. set */
		WRAP7 = 135,  /* wrap for 8th texture coord. set */
		CLIPPING = 136,
		LIGHTING = 137,
		AMBIENT = 139,
		FOGVERTEXMODE = 140,
		COLORVERTEX = 141,
		LOCALVIEWER = 142,
		NORMALIZENORMALS = 143,
		DIFFUSEMATERIALSOURCE = 145,
		SPECULARMATERIALSOURCE = 146,
		AMBIENTMATERIALSOURCE = 147,
		EMISSIVEMATERIALSOURCE = 148,
		VERTEXBLEND = 151,
		CLIPPLANEENABLE = 152,
		POINTSIZE = 154,   /* float point size */
		POINTSIZE_MIN = 155,   /* float point size min threshold */
		POINTSPRITEENABLE = 156,   /* BOOL point texture coord control */
		POINTSCALEENABLE = 157,   /* BOOL point size scale enable */
		POINTSCALE_A = 158,   /* float point attenuation A value */
		POINTSCALE_B = 159,   /* float point attenuation B value */
		POINTSCALE_C = 160,   /* float point attenuation C value */
		MULTISAMPLEANTIALIAS = 161,  // BOOL - set to do FSAA with multisample buffer
		MULTISAMPLEMASK = 162,  // DWORD - per-sample enable/disable
		PATCHEDGESTYLE = 163,  // Sets whether patch edges will use float style tessellation
		DEBUGMONITORTOKEN = 165,  // DEBUG ONLY - token to debug monitor
		POINTSIZE_MAX = 166,   /* float point size max threshold */
		INDEXEDVERTEXBLENDENABLE = 167,
		COLORWRITEENABLE = 168,  // per-channel write enable
		TWEENFACTOR = 170,   // float tween factor
		BLENDOP = 171,   // D3DBLENDOP setting
		POSITIONDEGREE = 172,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		NORMALDEGREE = 173,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		SCISSORTESTENABLE = 174,
		SLOPESCALEDEPTHBIAS = 175,
		ANTIALIASEDLINEENABLE = 176,
		MINTESSELLATIONLEVEL = 178,
		MAXTESSELLATIONLEVEL = 179,
		ADAPTIVETESS_X = 180,
		ADAPTIVETESS_Y = 181,
		ADAPTIVETESS_Z = 182,
		ADAPTIVETESS_W = 183,
		ENABLEADAPTIVETESSELLATION = 184,
		TWOSIDEDSTENCILMODE = 185,   /* BOOL enable/disable 2 sided stenciling */
		CCW_STENCILFAIL = 186,   /* D3DSTENCILOP to do if ccw stencil test fails */
		CCW_STENCILZFAIL = 187,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		CCW_STENCILPASS = 188,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		CCW_STENCILFUNC = 189,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		COLORWRITEENABLE1 = 190,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE2 = 191,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE3 = 192,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		BLENDFACTOR = 193,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		SRGBWRITEENABLE = 194,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		DEPTHBIAS = 195,
		WRAP8 = 198,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		WRAP9 = 199,
		WRAP10 = 200,
		WRAP11 = 201,
		WRAP12 = 202,
		WRAP13 = 203,
		WRAP14 = 204,
		WRAP15 = 205,
		SEPARATEALPHABLENDENABLE = 206,  /* TRUE to enable a separate blending function for the alpha channel */
		SRCBLENDALPHA = 207,  /* SRC blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		DESTBLENDALPHA = 208,  /* DST blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		BLENDOPALPHA = 209,  /* Blending operation for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
	};
}

