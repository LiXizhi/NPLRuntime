#pragma  once
#include <cstdint>
#include "Framework/Interface/Render/IRenderWindow.h"

#if defined(USE_DIRECTX_RENDERER)
#include <d3d9.h>
#endif

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
		VERTEXDATA,
		INDEX16,
		INDEX32,
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


#define CLEAR_TARGET            0x00000001l  /* Clear target surface */
#define CLEAR_ZBUFFER           0x00000002l  /* Clear target z buffer */
#define CLEAR_STENCIL           0x00000004l  /* Clear stencil planes */


	enum class ERenderState
	{
		Unkonwn = 0,
		ZENABLE,
		FILLMODE,
		SHADEMODE,
		ZWRITEENABLE,   /* TRUE to enable z writes */
		ALPHATESTENABLE,   /* TRUE to enable alpha tests */
		LASTPIXEL,   /* TRUE for last-pixel on lines */
		SRCBLEND ,   /* D3DBLEND */
		DESTBLEND,   /* D3DBLEND */
		CULLMODE,   /* D3DCULL */
		ZFUNC,   /* D3DCMPFUNC */
		ALPHAREF,   /* D3DFIXED */
		ALPHAFUNC,   /* D3DCMPFUNC */
		DITHERENABLE,   /* TRUE to enable dithering */
		ALPHABLENDENABLE,   /* TRUE to enable alpha blending */
		FOGENABLE,   /* TRUE to enable fog blending */
		SPECULARENABLE,   /* TRUE to enable specular */
		FOGCOLOR,   /* D3DCOLOR */
		FOGTABLEMODE,   /* D3DFOGMODE */
		FOGSTART,   /* Fog start (for both vertex and pixel fog) */
		FOGEND,   /* Fog end      */
		FOGDENSITY,   /* Fog density  */
		RANGEFOGENABLE,   /* Enables range-based fog */
		STENCILENABLE,   /* BOOL enable/disable stenciling */
		STENCILFAIL,   /* D3DSTENCILOP to do if stencil test fails */
		STENCILZFAIL,   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		STENCILPASS,   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		STENCILFUNC,   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		STENCILREF,   /* Reference value used in stencil test */
		STENCILMASK,   /* Mask value used in stencil test */
		STENCILWRITEMASK,   /* Write mask applied to values written to stencil buffer */
		TEXTUREFACTOR,   /* D3DCOLOR used for multi-texture blend */
		WRAP0,  /* wrap for 1st texture coord. set */
		WRAP1,  /* wrap for 2nd texture coord. set */
		WRAP2,  /* wrap for 3rd texture coord. set */
		WRAP3,  /* wrap for 4th texture coord. set */
		WRAP4,  /* wrap for 5th texture coord. set */
		WRAP5,  /* wrap for 6th texture coord. set */
		WRAP6,  /* wrap for 7th texture coord. set */
		WRAP7,  /* wrap for 8th texture coord. set */
		CLIPPING,
		LIGHTING,
		AMBIENT,
		FOGVERTEXMODE,
		COLORVERTEX,
		LOCALVIEWER,
		NORMALIZENORMALS,
		DIFFUSEMATERIALSOURCE,
		SPECULARMATERIALSOURCE,
		AMBIENTMATERIALSOURCE,
		EMISSIVEMATERIALSOURCE,
		VERTEXBLEND,
		CLIPPLANEENABLE,
		POINTSIZE,   /* float point size */
		POINTSIZE_MIN,   /* float point size min threshold */
		POINTSPRITEENABLE,   /* BOOL point texture coord control */
		POINTSCALEENABLE,   /* BOOL point size scale enable */
		POINTSCALE_A,   /* float point attenuation A value */
		POINTSCALE_B,   /* float point attenuation B value */
		POINTSCALE_C,   /* float point attenuation C value */
		MULTISAMPLEANTIALIAS,  // BOOL - set to do FSAA with multisample buffer
		MULTISAMPLEMASK,  // DWORD - per-sample enable/disable
		PATCHEDGESTYLE,  // Sets whether patch edges will use float style tessellation
		DEBUGMONITORTOKEN,  // DEBUG ONLY - token to debug monitor
		POINTSIZE_MAX,   /* float point size max threshold */
		INDEXEDVERTEXBLENDENABLE,
		COLORWRITEENABLE,  // per-channel write enable
		TWEENFACTOR,   // float tween factor
		BLENDOP,   // D3DBLENDOP setting
		POSITIONDEGREE,   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		NORMALDEGREE,   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		SCISSORTESTENABLE,
		SLOPESCALEDEPTHBIAS,
		ANTIALIASEDLINEENABLE,
		MINTESSELLATIONLEVEL,
		MAXTESSELLATIONLEVEL,
		ADAPTIVETESS_X,
		ADAPTIVETESS_Y,
		ADAPTIVETESS_Z,
		ADAPTIVETESS_W,
		ENABLEADAPTIVETESSELLATION,
		TWOSIDEDSTENCILMODE,   /* BOOL enable/disable 2 sided stenciling */
		CCW_STENCILFAIL,   /* D3DSTENCILOP to do if ccw stencil test fails */
		CCW_STENCILZFAIL,   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		CCW_STENCILPASS,   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		CCW_STENCILFUNC,   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		COLORWRITEENABLE1,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE2,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		COLORWRITEENABLE3,   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		BLENDFACTOR,   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		SRGBWRITEENABLE,   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		DEPTHBIAS,
		WRAP8,   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		WRAP9,
		WRAP10,
		WRAP11,
		WRAP12,
		WRAP13,
		WRAP14,
		WRAP15,
		SEPARATEALPHABLENDENABLE,  /* TRUE to enable a separate blending function for the alpha channel */
		SRCBLENDALPHA,  /* SRC blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		DESTBLENDALPHA,  /* DST blend factor for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */
		BLENDOPALPHA,  /* Blending operation for the alpha channel when SEPARATEDESTALPHAENABLE is TRUE */

		COUNT // END
	};

	// These bits could be ORed together to use with ERenderState::CLIPPLANEENABLE
	//
	const uint32_t RSV_CLIPPLANE_NONE(0);
	const uint32_t RSV_CLIPPLANE0(1 << 0);
	const uint32_t RSV_CLIPPLANE1(1 << 1);
	const uint32_t RSV_CLIPPLANE2(1 << 2);
	const uint32_t RSV_CLIPPLANE3(1 << 3);
	const uint32_t RSV_CLIPPLANE4(1 << 4);
	const uint32_t RSV_CLIPPLANE5(1 << 5);

	const uint32_t RSV_TRUE(1);
	const uint32_t RSV_FALSE(0);

	enum ERSVCULL
	{
		RSV_CULL_NONE,
		RSV_CULL_CW,
		RSV_CULL_CCW
	};

	enum ERSVCMP
	{
		RSV_CMP_NEVER = 1,
		RSV_CMP_LESS = 2,
		RSV_CMP_EQUAL = 3,
		RSV_CMP_LESSEQUAL = 4,
		RSV_CMP_GREATER = 5,
		RSV_CMP_NOTEQUAL = 6,
		RSV_CMP_GREATEREQUAL = 7,
		RSV_CMP_ALWAYS = 8,
	};

	enum ERSVBLEND {
		RSV_BLEND_ZERO = 1,
		RSV_BLEND_ONE = 2,
		RSV_BLEND_SRCCOLOR = 3,
		RSV_BLEND_INVSRCCOLOR = 4,
		RSV_BLEND_SRCALPHA = 5,
		RSV_BLEND_INVSRCALPHA = 6,
		RSV_BLEND_DESTALPHA = 7,
		RSV_BLEND_INVDESTALPHA = 8,
		RSV_BLEND_DESTCOLOR = 9,
		RSV_BLEND_INVDESTCOLOR = 10,
		RSV_BLEND_SRCALPHASAT = 11,
		RSV_BLEND_BOTHSRCALPHA = 12,
		RSV_BLEND_BOTHINVSRCALPHA = 13,
		RSV_BLEND_BLENDFACTOR = 14, /* Only supported if RSV_PBLENDCAPS_BLENDFACTOR is on */
		RSV_BLEND_INVBLENDFACTOR = 15, /* Only supported if RSV_PBLENDCAPS_BLENDFACTOR is on */
	};

	enum ERSVSTENCILOP {
		RSV_STENCILOP_KEEP = 1,
		RSV_STENCILOP_ZERO = 2,
		RSV_STENCILOP_REPLACE = 3,
		RSV_STENCILOP_INCRSAT = 4,
		RSV_STENCILOP_DECRSAT = 5,
		RSV_STENCILOP_INVERT = 6,
		RSV_STENCILOP_INCR = 7,
		RSV_STENCILOP_DECR = 8,
		RSV_STENCILOP_FORCE_DWORD = 0x7fffffff, /* force 32-bit size enum */
	} ;



	enum class ELightType {
		Point = 1,
		Spot = 2,
		Directional = 3
	};

	enum class EPoolType {
		Default = 0,
		Managed = 1,
		SystemMem = 2,
		Scratch = 3,
	};

	// Primitives supported by draw-primitive API
	enum class EPrimitiveType {
		POINTLIST = 1,
		LINELIST = 2,
		LINESTRIP = 3,
		TRIANGLELIST = 4,
		TRIANGLESTRIP = 5,
		TRIANGLEFAN = 6,
	};

	enum class ESamplerStateType
	{
		ADDRESSU = 1,
		ADDRESSV = 2,
		ADDRESSW = 3,
		BORDERCOLOR = 4, 
		MAGFILTER = 5,  
		MINFILTER = 6,  
		MIPFILTER = 7,  
		MIPMAPLODBIAS = 8, 
		MAXMIPLEVEL = 9,  
		MAXANISOTROPY = 10, 
		SRGBTEXTURE = 11,											
		ELEMENTINDEX = 12,						
		DMAPOFFSET = 13,	
	};
	
	enum class ETransformsStateType
	{
		WORLD,
		VIEW,
		PROJECTION,
		TEXTURE0,
		TEXTURE1,
		TEXTURE2,
		TEXTURE3,
		TEXTURE4,
		TEXTURE5,
		TEXTURE6,
		TEXTURE7,
	};
}



namespace ParaEngine
{
#if defined(USE_DIRECTX_RENDERER)
	using DeviceTexturePtr_type = IDirect3DTexture9 * ;
	using VertexBufferDevicePtr_type = IDirect3DVertexBuffer9 * ;
	using IndexBufferDevicePtr_type = IDirect3DIndexBuffer9 * ;
	using VertexDeclarationPtr = IDirect3DVertexDeclaration9 * ;
	using VertexElement = D3DVERTEXELEMENT9;
#elif defined(USE_OPENGL_RENDERER)

	struct VertexElement;
	class CVertexDeclaration; typedef CVertexDeclaration* VertexDeclarationPtr;
	class GLTexture2D;

	typedef GLTexture2D* DeviceTexturePtr_type;
	typedef uint32_t VertexBufferDevicePtr_type;
	typedef uint32_t IndexBufferDevicePtr_type;
#else
	struct VertexElement;
	class CVertexDeclaration; typedef CVertexDeclaration* VertexDeclarationPtr;

	typedef uint32_t DeviceTexturePtr_type;
	typedef uint32_t VertexBufferDevicePtr_type;
	typedef uint32_t IndexBufferDevicePtr_type;
#endif
}

