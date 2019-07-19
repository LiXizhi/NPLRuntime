#include "ParaEngine.h"
#include "DDSLoader.h"
#include "BC.h"
#include "png.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

namespace ParaEngine
{
	const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

	struct DDS_PIXELFORMAT
	{
		uint32_t    size;
		uint32_t    flags;
		uint32_t    fourCC;
		uint32_t    RGBBitCount;
		uint32_t    RBitMask;
		uint32_t    GBitMask;
		uint32_t    BBitMask;
		uint32_t    ABitMask;
	};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHAPIXELS 0x00000001  // DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8
#define DDS_PAL8A       0x00000021  // DDPF_PALETTEINDEXED8 | DDPF_ALPHAPIXELS
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
				(static_cast<uint32_t>(static_cast<uint8_t>(ch0)) \
				| (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8) \
				| (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16) \
				| (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24))
#endif /* defined(MAKEFOURCC) */

	const DDS_PIXELFORMAT DDSPF_DXT1 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_DXT2 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_DXT3 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_DXT4 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_DXT5 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_BC4_UNORM =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','U'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_BC4_SNORM =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','4','S'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_BC5_UNORM =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','U'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_BC5_SNORM =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('B','C','5','S'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('R','G','B','G'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('G','R','G','B'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_YUY2 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0 };

	const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

	const DDS_PIXELFORMAT DDSPF_X8R8G8B8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

	const DDS_PIXELFORMAT DDSPF_A8B8G8R8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

	const DDS_PIXELFORMAT DDSPF_X8B8G8R8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };

	const DDS_PIXELFORMAT DDSPF_G16R16 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGB,  0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

	const DDS_PIXELFORMAT DDSPF_R5G6B5 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

	const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

	const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

	const DDS_PIXELFORMAT DDSPF_R8G8B8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

	const DDS_PIXELFORMAT DDSPF_L8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0,  8, 0xff, 0x00, 0x00, 0x00 };

	const DDS_PIXELFORMAT DDSPF_L16 =
	{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };

	const DDS_PIXELFORMAT DDSPF_A8L8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };

	const DDS_PIXELFORMAT DDSPF_A8L8_ALT =
	{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA, 0, 8, 0x00ff, 0x0000, 0x0000, 0xff00 };

	const DDS_PIXELFORMAT DDSPF_A8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

	const DDS_PIXELFORMAT DDSPF_V8U8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000 };

	const DDS_PIXELFORMAT DDSPF_Q8W8V8U8 =
	{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

	const DDS_PIXELFORMAT DDSPF_V16U16 =
	{ sizeof(DDS_PIXELFORMAT), DDS_BUMPDUDV, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };

	// D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue

	// This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
	const DDS_PIXELFORMAT DDSPF_DX10 =
	{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','1','0'), 0, 0, 0, 0, 0 };

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
							   DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
							   DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

	// Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
	enum DDS_RESOURCE_DIMENSION
	{
		DDS_DIMENSION_TEXTURE1D = 2,
		DDS_DIMENSION_TEXTURE2D = 3,
		DDS_DIMENSION_TEXTURE3D = 4,
	};

	// Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
	enum DDS_RESOURCE_MISC_FLAG
	{
		DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
	};

	enum DDS_MISC_FLAGS2
	{
		DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
	};

	enum DDS_ALPHA_MODE
	{
		DDS_ALPHA_MODE_UNKNOWN = 0,
		DDS_ALPHA_MODE_STRAIGHT = 1,
		DDS_ALPHA_MODE_PREMULTIPLIED = 2,
		DDS_ALPHA_MODE_OPAQUE = 3,
		DDS_ALPHA_MODE_CUSTOM = 4,
	};

	struct DDS_HEADER
	{
		uint32_t        size;
		uint32_t        flags;
		uint32_t        height;
		uint32_t        width;
		uint32_t        pitchOrLinearSize;
		uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
		uint32_t        mipMapCount;
		uint32_t        reserved1[11];
		DDS_PIXELFORMAT ddspf;
		uint32_t        caps;
		uint32_t        caps2;
		uint32_t        caps3;
		uint32_t        caps4;
		uint32_t        reserved2;
	};

	struct DDS_HEADER_DXT10
	{
		DXGI_FORMAT     dxgiFormat;
		uint32_t        resourceDimension;
		uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
		uint32_t        arraySize;
		uint32_t        miscFlags2;
	};

	enum CONVERSION_FLAGS
	{
		CONV_FLAGS_NONE = 0x0,
		CONV_FLAGS_EXPAND = 0x1,      // Conversion requires expanded pixel size
		CONV_FLAGS_NOALPHA = 0x2,      // Conversion requires setting alpha to known value
		CONV_FLAGS_SWIZZLE = 0x4,      // BGR/RGB order swizzling required
		CONV_FLAGS_PAL8 = 0x8,      // Has an 8-bit palette
		CONV_FLAGS_888 = 0x10,     // Source is an 8:8:8 (24bpp) format
		CONV_FLAGS_565 = 0x20,     // Source is a 5:6:5 (16bpp) format
		CONV_FLAGS_5551 = 0x40,     // Source is a 5:5:5:1 (16bpp) format
		CONV_FLAGS_4444 = 0x80,     // Source is a 4:4:4:4 (16bpp) format
		CONV_FLAGS_44 = 0x100,    // Source is a 4:4 (8bpp) format
		CONV_FLAGS_332 = 0x200,    // Source is a 3:3:2 (8bpp) format
		CONV_FLAGS_8332 = 0x400,    // Source is a 8:3:3:2 (16bpp) format
		CONV_FLAGS_A8P8 = 0x800,    // Has an 8-bit palette with an alpha channel
		CONV_FLAGS_DX10 = 0x10000,  // Has the 'DX10' extension header
		CONV_FLAGS_PMALPHA = 0x20000,  // Contains premultiplied alpha data
		CONV_FLAGS_L8 = 0x40000,  // Source is a 8 luminance format 
		CONV_FLAGS_L16 = 0x80000,  // Source is a 16 luminance format 
		CONV_FLAGS_A8L8 = 0x100000, // Source is a 8:8 luminance format 
	};

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

	struct LegacyDDS
	{
		DXGI_FORMAT     format;
		DWORD           convFlags;
		DDS_PIXELFORMAT ddpf;
	};

	const LegacyDDS g_LegacyDDSMap[] =
	{
		{ DXGI_FORMAT_BC1_UNORM,          CONV_FLAGS_NONE,        DDSPF_DXT1 }, // D3DFMT_DXT1
		{ DXGI_FORMAT_BC2_UNORM,          CONV_FLAGS_NONE,        DDSPF_DXT3 }, // D3DFMT_DXT3
		{ DXGI_FORMAT_BC3_UNORM,          CONV_FLAGS_NONE,        DDSPF_DXT5 }, // D3DFMT_DXT5

		{ DXGI_FORMAT_BC2_UNORM,          CONV_FLAGS_PMALPHA,     DDSPF_DXT2 }, // D3DFMT_DXT2
		{ DXGI_FORMAT_BC3_UNORM,          CONV_FLAGS_PMALPHA,     DDSPF_DXT4 }, // D3DFMT_DXT4

		{ DXGI_FORMAT_BC4_UNORM,          CONV_FLAGS_NONE,        DDSPF_BC4_UNORM },
		{ DXGI_FORMAT_BC4_SNORM,          CONV_FLAGS_NONE,        DDSPF_BC4_SNORM },
		{ DXGI_FORMAT_BC5_UNORM,          CONV_FLAGS_NONE,        DDSPF_BC5_UNORM },
		{ DXGI_FORMAT_BC5_SNORM,          CONV_FLAGS_NONE,        DDSPF_BC5_SNORM },

		{ DXGI_FORMAT_BC4_UNORM,          CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('A', 'T', 'I', '1'), 0, 0, 0, 0, 0 } },
		{ DXGI_FORMAT_BC5_UNORM,          CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('A', 'T', 'I', '2'), 0, 0, 0, 0, 0 } },

		{ DXGI_FORMAT_R8G8_B8G8_UNORM,    CONV_FLAGS_NONE,        DDSPF_R8G8_B8G8 }, // D3DFMT_R8G8_B8G8
		{ DXGI_FORMAT_G8R8_G8B8_UNORM,    CONV_FLAGS_NONE,        DDSPF_G8R8_G8B8 }, // D3DFMT_G8R8_G8B8

		{ DXGI_FORMAT_B8G8R8A8_UNORM,     CONV_FLAGS_NONE,        DDSPF_A8R8G8B8 }, // D3DFMT_A8R8G8B8 (uses DXGI 1.1 format)
		{ DXGI_FORMAT_B8G8R8X8_UNORM,     CONV_FLAGS_NONE,        DDSPF_X8R8G8B8 }, // D3DFMT_X8R8G8B8 (uses DXGI 1.1 format)
		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_NONE,        DDSPF_A8B8G8R8 }, // D3DFMT_A8B8G8R8
		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_NOALPHA,     DDSPF_X8B8G8R8 }, // D3DFMT_X8B8G8R8
		{ DXGI_FORMAT_R16G16_UNORM,       CONV_FLAGS_NONE,        DDSPF_G16R16 }, // D3DFMT_G16R16

		{ DXGI_FORMAT_R10G10B10A2_UNORM,  CONV_FLAGS_SWIZZLE,{ sizeof(DDS_PIXELFORMAT), DDS_RGBA,      0, 32, 0x000003ff, 0x000ffc00, 0x3ff00000, 0xc0000000 } }, // D3DFMT_A2R10G10B10 (D3DX reversal issue workaround)
		{ DXGI_FORMAT_R10G10B10A2_UNORM,  CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_RGBA,      0, 32, 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 } }, // D3DFMT_A2B10G10R10 (D3DX reversal issue workaround)

		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_EXPAND
		| CONV_FLAGS_NOALPHA
		| CONV_FLAGS_888,       DDSPF_R8G8B8 }, // D3DFMT_R8G8B8

		{ DXGI_FORMAT_B5G6R5_UNORM,       CONV_FLAGS_565,         DDSPF_R5G6B5 }, // D3DFMT_R5G6B5
		{ DXGI_FORMAT_B5G5R5A1_UNORM,     CONV_FLAGS_5551,        DDSPF_A1R5G5B5 }, // D3DFMT_A1R5G5B5
		{ DXGI_FORMAT_B5G5R5A1_UNORM,     CONV_FLAGS_5551
		| CONV_FLAGS_NOALPHA,{ sizeof(DDS_PIXELFORMAT), DDS_RGB,       0, 16, 0x7c00,     0x03e0,     0x001f,     0x0000 } }, // D3DFMT_X1R5G5B5

		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_EXPAND
		| CONV_FLAGS_8332,{ sizeof(DDS_PIXELFORMAT), DDS_RGBA,      0, 16, 0x00e0,     0x001c,     0x0003,     0xff00 } }, // D3DFMT_A8R3G3B2
		{ DXGI_FORMAT_B5G6R5_UNORM,       CONV_FLAGS_EXPAND
		| CONV_FLAGS_332,{ sizeof(DDS_PIXELFORMAT), DDS_RGB,       0,  8, 0xe0,       0x1c,       0x03,       0x00 } }, // D3DFMT_R3G3B2

		{ DXGI_FORMAT_R8_UNORM,           CONV_FLAGS_NONE,        DDSPF_L8 }, // D3DFMT_L8
		{ DXGI_FORMAT_R16_UNORM,          CONV_FLAGS_NONE,        DDSPF_L16 }, // D3DFMT_L16
		{ DXGI_FORMAT_R8G8_UNORM,         CONV_FLAGS_NONE,        DDSPF_A8L8 }, // D3DFMT_A8L8
		{ DXGI_FORMAT_R8G8_UNORM,         CONV_FLAGS_NONE,        DDSPF_A8L8_ALT }, // D3DFMT_A8L8 (alternative bitcount)

		{ DXGI_FORMAT_A8_UNORM,           CONV_FLAGS_NONE,        DDSPF_A8 }, // D3DFMT_A8

		{ DXGI_FORMAT_R16G16B16A16_UNORM, CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,   36,  0, 0,          0,          0,          0 } }, // D3DFMT_A16B16G16R16
		{ DXGI_FORMAT_R16G16B16A16_SNORM, CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  110,  0, 0,          0,          0,          0 } }, // D3DFMT_Q16W16V16U16
		{ DXGI_FORMAT_R16_FLOAT,          CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  111,  0, 0,          0,          0,          0 } }, // D3DFMT_R16F
		{ DXGI_FORMAT_R16G16_FLOAT,       CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  112,  0, 0,          0,          0,          0 } }, // D3DFMT_G16R16F
		{ DXGI_FORMAT_R16G16B16A16_FLOAT, CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  113,  0, 0,          0,          0,          0 } }, // D3DFMT_A16B16G16R16F
		{ DXGI_FORMAT_R32_FLOAT,          CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  114,  0, 0,          0,          0,          0 } }, // D3DFMT_R32F
		{ DXGI_FORMAT_R32G32_FLOAT,       CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  115,  0, 0,          0,          0,          0 } }, // D3DFMT_G32R32F
		{ DXGI_FORMAT_R32G32B32A32_FLOAT, CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,  116,  0, 0,          0,          0,          0 } }, // D3DFMT_A32B32G32R32F

		{ DXGI_FORMAT_R32_FLOAT,          CONV_FLAGS_NONE,{ sizeof(DDS_PIXELFORMAT), DDS_RGB,       0, 32, 0xffffffff, 0x00000000, 0x00000000, 0x00000000 } }, // D3DFMT_R32F (D3DX uses FourCC 114 instead)

		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_EXPAND
		| CONV_FLAGS_PAL8
		| CONV_FLAGS_A8P8,{ sizeof(DDS_PIXELFORMAT), DDS_PAL8A,     0, 16, 0,          0,          0,          0 } }, // D3DFMT_A8P8
		{ DXGI_FORMAT_R8G8B8A8_UNORM,     CONV_FLAGS_EXPAND
		| CONV_FLAGS_PAL8,{ sizeof(DDS_PIXELFORMAT), DDS_PAL8,      0,  8, 0,          0,          0,          0 } }, // D3DFMT_P8

		{ DXGI_FORMAT_B4G4R4A4_UNORM,     CONV_FLAGS_4444,        DDSPF_A4R4G4B4 }, // D3DFMT_A4R4G4B4 (uses DXGI 1.2 format)
		{ DXGI_FORMAT_B4G4R4A4_UNORM,     CONV_FLAGS_NOALPHA
		| CONV_FLAGS_4444,{ sizeof(DDS_PIXELFORMAT), DDS_RGB,       0, 16, 0x0f00,     0x00f0,     0x000f,     0x0000 } }, // D3DFMT_X4R4G4B4 (uses DXGI 1.2 format)
		{ DXGI_FORMAT_B4G4R4A4_UNORM,     CONV_FLAGS_EXPAND
		| CONV_FLAGS_44,{ sizeof(DDS_PIXELFORMAT), DDS_LUMINANCEA,0,  8, 0x0f,       0x00,       0x00,       0xf0 } }, // D3DFMT_A4L4 (uses DXGI 1.2 format)

		{ DXGI_FORMAT_YUY2,               CONV_FLAGS_NONE,        DDSPF_YUY2 }, // D3DFMT_YUY2 (uses DXGI 1.2 format)
		{ DXGI_FORMAT_YUY2,               CONV_FLAGS_SWIZZLE,{ sizeof(DDS_PIXELFORMAT), DDS_FOURCC,    MAKEFOURCC('U','Y','V','Y'), 0, 0, 0, 0, 0 } }, // D3DFMT_UYVY (uses DXGI 1.2 format)

		{ DXGI_FORMAT_R8G8_SNORM,         CONV_FLAGS_NONE,        DDSPF_V8U8 },     // D3DFMT_V8U8
		{ DXGI_FORMAT_R8G8B8A8_SNORM,     CONV_FLAGS_NONE,        DDSPF_Q8W8V8U8 }, // D3DFMT_Q8W8V8U8
		{ DXGI_FORMAT_R16G16_SNORM,       CONV_FLAGS_NONE,        DDSPF_V16U16 },   // D3DFMT_V16U16
	};

	enum CONVERT_FLAGS
	{
		CONVF_FLOAT = 0x1,
		CONVF_UNORM = 0x2,
		CONVF_UINT = 0x4,
		CONVF_SNORM = 0x8,
		CONVF_SINT = 0x10,
		CONVF_DEPTH = 0x20,
		CONVF_STENCIL = 0x40,
		CONVF_SHAREDEXP = 0x80,
		CONVF_BGR = 0x100,
		CONVF_XR = 0x200,
		CONVF_PACKED = 0x400,
		CONVF_BC = 0x800,
		CONVF_YUV = 0x1000,
		CONVF_POS_ONLY = 0x2000,
		CONVF_R = 0x10000,
		CONVF_G = 0x20000,
		CONVF_B = 0x40000,
		CONVF_A = 0x80000,
		CONVF_RGB_MASK = 0x70000,
		CONVF_RGBA_MASK = 0xF0000,
	};

#if DIRECTX_MATH_VERSION >= 310
#define StoreFloat3SE XMStoreFloat3SE
#else
	inline void XM_CALLCONV StoreFloat3SE(_Out_ XMFLOAT3SE* pDestination, DirectX::FXMVECTOR V)
	{
		assert(pDestination);

		DirectX::XMFLOAT3A tmp;
		DirectX::XMStoreFloat3A(&tmp, V);

		static const float maxf9 = float(0x1FF << 7);
		static const float minf9 = float(1.f / (1 << 16));

		float x = (tmp.x >= 0.f) ? ((tmp.x > maxf9) ? maxf9 : tmp.x) : 0.f;
		float y = (tmp.y >= 0.f) ? ((tmp.y > maxf9) ? maxf9 : tmp.y) : 0.f;
		float z = (tmp.z >= 0.f) ? ((tmp.z > maxf9) ? maxf9 : tmp.z) : 0.f;

		const float max_xy = (x > y) ? x : y;
		const float max_xyz = (max_xy > z) ? max_xy : z;

		const float maxColor = (max_xyz > minf9) ? max_xyz : minf9;

		union { float f; int32_t i; } fi;
		fi.f = maxColor;
		fi.i += 0x00004000; // round up leaving 9 bits in fraction (including assumed 1)

							// Fix applied from DirectXMath 3.10
		uint32_t exp = fi.i >> 23;
		pDestination->e = exp - 0x6f;

		fi.i = 0x83000000 - (exp << 23);
		float ScaleR = fi.f;

		pDestination->xm = static_cast<uint32_t>(lroundf(x * ScaleR));
		pDestination->ym = static_cast<uint32_t>(lroundf(y * ScaleR));
		pDestination->zm = static_cast<uint32_t>(lroundf(z * ScaleR));
	}
#endif
	const XMVECTORF32 g_Grayscale = { { { 0.2125f, 0.7154f, 0.0721f, 0.0f } } };
	const XMVECTORF32 g_HalfMin = { { { -65504.f, -65504.f, -65504.f, -65504.f } } };
	const XMVECTORF32 g_HalfMax = { { { 65504.f, 65504.f, 65504.f, 65504.f } } };
	const XMVECTORF32 g_8BitBias = { { { 0.5f / 255.f, 0.5f / 255.f, 0.5f / 255.f, 0.5f / 255.f } } };

	//-------------------------------------------------------------------------------------
	// Stores an image row from standard RGBA XMVECTOR (aligned) array
	//-------------------------------------------------------------------------------------
#define STORE_SCANLINE( type, func )\
		if (size >= sizeof(type))\
		{\
			type * __restrict dPtr = reinterpret_cast<type*>(pDestination);\
			for(uint32_t icount = 0; icount < (size - sizeof(type) + 1); icount += sizeof(type))\
			{\
				if (sPtr >= ePtr) break;\
				func(dPtr++, *sPtr++);\
			}\
			return true; \
		}\
		return false;

	_Use_decl_annotations_
		bool _StoreScanline(
			void* pDestination,
			uint32_t size,
			DXGI_FORMAT format,
			const XMVECTOR* pSource,
			uint32_t count,
			float threshold = 0)
	{
		assert(pDestination && size > 0);
		assert(pSource && count > 0 && ((reinterpret_cast<uintptr_t>(pSource) & 0xF) == 0));

		const XMVECTOR* __restrict sPtr = pSource;
		if (!sPtr)
			return false;

		const XMVECTOR* ePtr = pSource + count;

		switch (static_cast<int>(format))
		{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			STORE_SCANLINE(XMFLOAT4, XMStoreFloat4)

		case DXGI_FORMAT_R32G32B32A32_UINT:
			STORE_SCANLINE(XMUINT4, XMStoreUInt4)

		case DXGI_FORMAT_R32G32B32A32_SINT:
			STORE_SCANLINE(XMINT4, XMStoreSInt4)

		case DXGI_FORMAT_R32G32B32_FLOAT:
			STORE_SCANLINE(XMFLOAT3, XMStoreFloat3)

		case DXGI_FORMAT_R32G32B32_UINT:
			STORE_SCANLINE(XMUINT3, XMStoreUInt3)

		case DXGI_FORMAT_R32G32B32_SINT:
			STORE_SCANLINE(XMINT3, XMStoreSInt3)

		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			if (size >= sizeof(XMHALF4))
			{
				XMHALF4* __restrict dPtr = static_cast<XMHALF4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMHALF4) + 1); icount += sizeof(XMHALF4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = *sPtr++;
					v = XMVectorClamp(v, g_HalfMin, g_HalfMax);
					XMStoreHalf4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R16G16B16A16_UNORM:
			STORE_SCANLINE(XMUSHORTN4, XMStoreUShortN4)

		case DXGI_FORMAT_R16G16B16A16_UINT:
			STORE_SCANLINE(XMUSHORT4, XMStoreUShort4)

		case DXGI_FORMAT_R16G16B16A16_SNORM:
			STORE_SCANLINE(XMSHORTN4, XMStoreShortN4)

		case DXGI_FORMAT_R16G16B16A16_SINT:
			STORE_SCANLINE(XMSHORT4, XMStoreShort4)

		case DXGI_FORMAT_R32G32_FLOAT:
			STORE_SCANLINE(XMFLOAT2, XMStoreFloat2)

		case DXGI_FORMAT_R32G32_UINT:
			STORE_SCANLINE(XMUINT2, XMStoreUInt2)

		case DXGI_FORMAT_R32G32_SINT:
			STORE_SCANLINE(XMINT2, XMStoreSInt2)

		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			{
				const uint32_t psize = sizeof(float) + sizeof(uint32_t);
				if (size >= psize)
				{
					auto dPtr = static_cast<float*>(pDestination);
					for (uint32_t icount = 0; icount < (size - psize + 1); icount += psize)
					{
						if (sPtr >= ePtr) break;
						XMFLOAT4 f;
						XMStoreFloat4(&f, *sPtr++);
						dPtr[0] = f.x;
						auto ps8 = reinterpret_cast<uint8_t*>(&dPtr[1]);
						ps8[0] = static_cast<uint8_t>(std::min<float>(255.f, std::max<float>(0.f, f.y)));
						ps8[1] = ps8[2] = ps8[3] = 0;
						dPtr += 2;
					}
					return true;
				}
			}
			return false;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			STORE_SCANLINE(XMUDECN4, XMStoreUDecN4)

		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			STORE_SCANLINE(XMUDECN4, XMStoreUDecN4_XR)

		case DXGI_FORMAT_R10G10B10A2_UINT:
			STORE_SCANLINE(XMUDEC4, XMStoreUDec4)

		case DXGI_FORMAT_R11G11B10_FLOAT:
			STORE_SCANLINE(XMFLOAT3PK, XMStoreFloat3PK)

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorAdd(*sPtr++, g_8BitBias);
					XMStoreUByteN4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8G8B8A8_UINT:
			STORE_SCANLINE(XMUBYTE4, XMStoreUByte4)

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			STORE_SCANLINE(XMBYTEN4, XMStoreByteN4)

		case DXGI_FORMAT_R8G8B8A8_SINT:
			STORE_SCANLINE(XMBYTE4, XMStoreByte4)

		case DXGI_FORMAT_R16G16_FLOAT:
			if (size >= sizeof(XMHALF2))
			{
				XMHALF2* __restrict dPtr = static_cast<XMHALF2*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMHALF2) + 1); icount += sizeof(XMHALF2))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = *sPtr++;
					v = XMVectorClamp(v, g_HalfMin, g_HalfMax);
					XMStoreHalf2(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R16G16_UNORM:
			STORE_SCANLINE(XMUSHORTN2, XMStoreUShortN2)

		case DXGI_FORMAT_R16G16_UINT:
			STORE_SCANLINE(XMUSHORT2, XMStoreUShort2)

		case DXGI_FORMAT_R16G16_SNORM:
			STORE_SCANLINE(XMSHORTN2, XMStoreShortN2)

		case DXGI_FORMAT_R16G16_SINT:
			STORE_SCANLINE(XMSHORT2, XMStoreShort2)

		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
			if (size >= sizeof(float))
			{
				float * __restrict dPtr = static_cast<float*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(float) + 1); icount += sizeof(float))
				{
					if (sPtr >= ePtr) break;
					XMStoreFloat(dPtr++, *(sPtr++));
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R32_UINT:
			if (size >= sizeof(uint32_t))
			{
				uint32_t * __restrict dPtr = static_cast<uint32_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(uint32_t) + 1); icount += sizeof(uint32_t))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMConvertVectorFloatToUInt(*(sPtr++), 0);
					XMStoreInt(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R32_SINT:
			if (size >= sizeof(int32_t))
			{
				uint32_t * __restrict dPtr = static_cast<uint32_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(int32_t) + 1); icount += sizeof(int32_t))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMConvertVectorFloatToInt(*(sPtr++), 0);
					XMStoreInt(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			if (size >= sizeof(uint32_t))
			{
				static const XMVECTORF32 clamp = { { { 1.f, 255.f, 0.f, 0.f } } };
				XMVECTOR zero = XMVectorZero();
				auto dPtr = static_cast<uint32_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(uint32_t) + 1); icount += sizeof(uint32_t))
				{
					if (sPtr >= ePtr) break;
					XMFLOAT4 f;
					XMStoreFloat4(&f, XMVectorClamp(*sPtr++, zero, clamp));
					*dPtr++ = (static_cast<uint32_t>(f.x * 16777215.f) & 0xFFFFFF)
						| ((static_cast<uint32_t>(f.y) & 0xFF) << 24);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8G8_UNORM:
			STORE_SCANLINE(XMUBYTEN2, XMStoreUByteN2)

		case DXGI_FORMAT_R8G8_UINT:
			STORE_SCANLINE(XMUBYTE2, XMStoreUByte2)

		case DXGI_FORMAT_R8G8_SNORM:
			STORE_SCANLINE(XMBYTEN2, XMStoreByteN2)

		case DXGI_FORMAT_R8G8_SINT:
			STORE_SCANLINE(XMBYTE2, XMStoreByte2)

		case DXGI_FORMAT_R16_FLOAT:
			if (size >= sizeof(HALF))
			{
				HALF * __restrict dPtr = static_cast<HALF*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(HALF) + 1); icount += sizeof(HALF))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 65504.f), -65504.f);
					*(dPtr++) = XMConvertFloatToHalf(v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
			if (size >= sizeof(uint16_t))
			{
				uint16_t * __restrict dPtr = static_cast<uint16_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(uint16_t) + 1); icount += sizeof(uint16_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 1.f), 0.f);
					*(dPtr++) = static_cast<uint16_t>(v*65535.f + 0.5f);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R16_UINT:
			if (size >= sizeof(uint16_t))
			{
				uint16_t * __restrict dPtr = static_cast<uint16_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(uint16_t) + 1); icount += sizeof(uint16_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 65535.f), 0.f);
					*(dPtr++) = static_cast<uint16_t>(v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R16_SNORM:
			if (size >= sizeof(int16_t))
			{
				int16_t * __restrict dPtr = static_cast<int16_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(int16_t) + 1); icount += sizeof(int16_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 1.f), -1.f);
					*(dPtr++) = static_cast<int16_t>(v * 32767.f);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R16_SINT:
			if (size >= sizeof(int16_t))
			{
				int16_t * __restrict dPtr = static_cast<int16_t*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(int16_t) + 1); icount += sizeof(int16_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 32767.f), -32767.f);
					*(dPtr++) = static_cast<int16_t>(v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8_UNORM:
			if (size >= sizeof(uint8_t))
			{
				uint8_t * __restrict dPtr = static_cast<uint8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(uint8_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 1.f), 0.f);
					*(dPtr++) = static_cast<uint8_t>(v * 255.f);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8_UINT:
			if (size >= sizeof(uint8_t))
			{
				uint8_t * __restrict dPtr = static_cast<uint8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(uint8_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 255.f), 0.f);
					*(dPtr++) = static_cast<uint8_t>(v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8_SNORM:
			if (size >= sizeof(int8_t))
			{
				int8_t * __restrict dPtr = static_cast<int8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(int8_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 1.f), -1.f);
					*(dPtr++) = static_cast<int8_t>(v * 127.f);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R8_SINT:
			if (size >= sizeof(int8_t))
			{
				int8_t * __restrict dPtr = static_cast<int8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(int8_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetX(*sPtr++);
					v = std::max<float>(std::min<float>(v, 127.f), -127.f);
					*(dPtr++) = static_cast<int8_t>(v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_A8_UNORM:
			if (size >= sizeof(uint8_t))
			{
				uint8_t * __restrict dPtr = static_cast<uint8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(uint8_t))
				{
					if (sPtr >= ePtr) break;
					float v = XMVectorGetW(*sPtr++);
					v = std::max<float>(std::min<float>(v, 1.f), 0.f);
					*(dPtr++) = static_cast<uint8_t>(v * 255.f);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R1_UNORM:
			if (size >= sizeof(uint8_t))
			{
				uint8_t * __restrict dPtr = static_cast<uint8_t*>(pDestination);
				for (uint32_t icount = 0; icount < size; icount += sizeof(uint8_t))
				{
					uint8_t pixels = 0;
					for (uint32_t bcount = 8; bcount > 0; --bcount)
					{
						if (sPtr >= ePtr) break;
						float v = XMVectorGetX(*sPtr++);

						// Absolute thresholding generally doesn't give good results for all images
						// Picking the 'right' threshold automatically requires whole-image analysis

						if (v > 0.25f)
							pixels |= 1 << (bcount - 1);
					}
					*(dPtr++) = pixels;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			STORE_SCANLINE(XMFLOAT3SE, StoreFloat3SE)

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v0 = *sPtr++;
					XMVECTOR v1 = (sPtr < ePtr) ? XMVectorSplatY(*sPtr++) : XMVectorZero();
					XMVECTOR v = XMVectorSelect(v1, v0, g_XMSelect1110);
					v = XMVectorAdd(v, g_8BitBias);
					XMStoreUByteN4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_G8R8_G8B8_UNORM:
			if (size >= sizeof(XMUBYTEN4))
			{
				static XMVECTORU32 select1101 = { { { XM_SELECT_1, XM_SELECT_1, XM_SELECT_0, XM_SELECT_1 } } };

				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v0 = XMVectorSwizzle<1, 0, 3, 2>(*sPtr++);
					XMVECTOR v1 = (sPtr < ePtr) ? XMVectorSplatY(*sPtr++) : XMVectorZero();
					XMVECTOR v = XMVectorSelect(v1, v0, select1101);
					v = XMVectorAdd(v, g_8BitBias);
					XMStoreUByteN4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_B5G6R5_UNORM:
			if (size >= sizeof(XMU565))
			{
				static const XMVECTORF32 s_Scale = { { { 31.f, 63.f, 31.f, 1.f } } };
				XMU565 * __restrict dPtr = static_cast<XMU565*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMU565) + 1); icount += sizeof(XMU565))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorSwizzle<2, 1, 0, 3>(*sPtr++);
					v = XMVectorMultiply(v, s_Scale);
					XMStoreU565(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			if (size >= sizeof(XMU555))
			{
				static const XMVECTORF32 s_Scale = { { { 31.f, 31.f, 31.f, 1.f } } };
				XMU555 * __restrict dPtr = static_cast<XMU555*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMU555) + 1); icount += sizeof(XMU555))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorSwizzle<2, 1, 0, 3>(*sPtr++);
					v = XMVectorMultiply(v, s_Scale);
					XMStoreU555(dPtr, v);
					dPtr->w = (XMVectorGetW(v) > threshold) ? 1u : 0u;
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorSwizzle<2, 1, 0, 3>(*sPtr++);
					v = XMVectorAdd(v, g_8BitBias);
					XMStoreUByteN4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorPermute<2, 1, 0, 7>(*sPtr++, g_XMIdentityR3);
					v = XMVectorAdd(v, g_8BitBias);
					XMStoreUByteN4(dPtr++, v);
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_AYUV:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;

					XMUBYTEN4 rgba;
					XMStoreUByteN4(&rgba, *sPtr++);

					// http://msdn.microsoft.com/en-us/library/windows/desktop/dd206750.aspx

					// Y  =  0.2568R + 0.5041G + 0.1001B + 16
					// Cb = -0.1482R - 0.2910G + 0.4392B + 128
					// Cr =  0.4392R - 0.3678G - 0.0714B + 128

					int y = ((66 * rgba.x + 129 * rgba.y + 25 * rgba.z + 128) >> 8) + 16;
					int u = ((-38 * rgba.x - 74 * rgba.y + 112 * rgba.z + 128) >> 8) + 128;
					int v = ((112 * rgba.x - 94 * rgba.y - 18 * rgba.z + 128) >> 8) + 128;

					dPtr->x = static_cast<uint8_t>(std::min<int>(std::max<int>(v, 0), 255));
					dPtr->y = static_cast<uint8_t>(std::min<int>(std::max<int>(u, 0), 255));
					dPtr->z = static_cast<uint8_t>(std::min<int>(std::max<int>(y, 0), 255));
					dPtr->w = rgba.w;
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_Y410:
			if (size >= sizeof(XMUDECN4))
			{
				XMUDECN4 * __restrict dPtr = static_cast<XMUDECN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUDECN4) + 1); icount += sizeof(XMUDECN4))
				{
					if (sPtr >= ePtr) break;

					XMUDECN4 rgba;
					XMStoreUDecN4(&rgba, *sPtr++);

					// http://msdn.microsoft.com/en-us/library/windows/desktop/bb970578.aspx

					// Y  =  0.2560R + 0.5027G + 0.0998B + 64
					// Cb = -0.1478R - 0.2902G + 0.4379B + 512
					// Cr =  0.4379R - 0.3667G - 0.0712B + 512

					int64_t r = rgba.x;
					int64_t g = rgba.y;
					int64_t b = rgba.z;

					int y = static_cast<int>((16780 * r + 32942 * g + 6544 * b + 32768) >> 16) + 64;
					int u = static_cast<int>((-9683 * r - 19017 * g + 28700 * b + 32768) >> 16) + 512;
					int v = static_cast<int>((28700 * r - 24033 * g - 4667 * b + 32768) >> 16) + 512;

					dPtr->x = static_cast<uint32_t>(std::min<int>(std::max<int>(u, 0), 1023));
					dPtr->y = static_cast<uint32_t>(std::min<int>(std::max<int>(y, 0), 1023));
					dPtr->z = static_cast<uint32_t>(std::min<int>(std::max<int>(v, 0), 1023));
					dPtr->w = rgba.w;
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_Y416:
			if (size >= sizeof(XMUSHORTN4))
			{
				XMUSHORTN4 * __restrict dPtr = static_cast<XMUSHORTN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUSHORTN4) + 1); icount += sizeof(XMUSHORTN4))
				{
					if (sPtr >= ePtr) break;

					XMUSHORTN4 rgba;
					XMStoreUShortN4(&rgba, *sPtr++);

					// http://msdn.microsoft.com/en-us/library/windows/desktop/bb970578.aspx

					// Y  =  0.2558R + 0.5022G + 0.0998B + 4096
					// Cb = -0.1476R - 0.2899G + 0.4375B + 32768
					// Cr =  0.4375R - 0.3664G - 0.0711B + 32768

					int64_t r = int64_t(rgba.x);
					int64_t g = int64_t(rgba.y);
					int64_t b = int64_t(rgba.z);

					int y = static_cast<int>((16763 * r + 32910 * g + 6537 * b + 32768) >> 16) + 4096;
					int u = static_cast<int>((-9674 * r - 18998 * g + 28672 * b + 32768) >> 16) + 32768;
					int v = static_cast<int>((28672 * r - 24010 * g - 4662 * b + 32768) >> 16) + 32768;

					dPtr->x = static_cast<uint16_t>(std::min<int>(std::max<int>(u, 0), 65535));
					dPtr->y = static_cast<uint16_t>(std::min<int>(std::max<int>(y, 0), 65535));
					dPtr->z = static_cast<uint16_t>(std::min<int>(std::max<int>(v, 0), 65535));
					dPtr->w = rgba.w;
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_YUY2:
			if (size >= sizeof(XMUBYTEN4))
			{
				XMUBYTEN4 * __restrict dPtr = static_cast<XMUBYTEN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUBYTEN4) + 1); icount += sizeof(XMUBYTEN4))
				{
					if (sPtr >= ePtr) break;

					XMUBYTEN4 rgb1;
					XMStoreUByteN4(&rgb1, *sPtr++);

					// See AYUV
					int y0 = ((66 * rgb1.x + 129 * rgb1.y + 25 * rgb1.z + 128) >> 8) + 16;
					int u0 = ((-38 * rgb1.x - 74 * rgb1.y + 112 * rgb1.z + 128) >> 8) + 128;
					int v0 = ((112 * rgb1.x - 94 * rgb1.y - 18 * rgb1.z + 128) >> 8) + 128;

					XMUBYTEN4 rgb2;
					if (sPtr < ePtr)
					{
						XMStoreUByteN4(&rgb2, *sPtr++);
					}
					else
					{
						rgb2.x = rgb2.y = rgb2.z = rgb2.w = 0;
					}

					int y1 = ((66 * rgb2.x + 129 * rgb2.y + 25 * rgb2.z + 128) >> 8) + 16;
					int u1 = ((-38 * rgb2.x - 74 * rgb2.y + 112 * rgb2.z + 128) >> 8) + 128;
					int v1 = ((112 * rgb2.x - 94 * rgb2.y - 18 * rgb2.z + 128) >> 8) + 128;

					dPtr->x = static_cast<uint8_t>(std::min<int>(std::max<int>(y0, 0), 255));
					dPtr->y = static_cast<uint8_t>(std::min<int>(std::max<int>((u0 + u1) >> 1, 0), 255));
					dPtr->z = static_cast<uint8_t>(std::min<int>(std::max<int>(y1, 0), 255));
					dPtr->w = static_cast<uint8_t>(std::min<int>(std::max<int>((v0 + v1) >> 1, 0), 255));
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_Y210:
			// Same as Y216 with least significant 6 bits set to zero
			if (size >= sizeof(XMUSHORTN4))
			{
				XMUSHORTN4 * __restrict dPtr = static_cast<XMUSHORTN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUSHORTN4) + 1); icount += sizeof(XMUSHORTN4))
				{
					if (sPtr >= ePtr) break;

					XMUDECN4 rgb1;
					XMStoreUDecN4(&rgb1, *sPtr++);

					// See Y410
					int64_t r = rgb1.x;
					int64_t g = rgb1.y;
					int64_t b = rgb1.z;

					int y0 = static_cast<int>((16780 * r + 32942 * g + 6544 * b + 32768) >> 16) + 64;
					int u0 = static_cast<int>((-9683 * r - 19017 * g + 28700 * b + 32768) >> 16) + 512;
					int v0 = static_cast<int>((28700 * r - 24033 * g - 4667 * b + 32768) >> 16) + 512;

					XMUDECN4 rgb2;
					if (sPtr < ePtr)
					{
						XMStoreUDecN4(&rgb2, *sPtr++);
					}
					else
					{
						rgb2.x = rgb2.y = rgb2.z = rgb2.w = 0;
					}

					r = rgb2.x;
					g = rgb2.y;
					b = rgb2.z;

					int y1 = static_cast<int>((16780 * r + 32942 * g + 6544 * b + 32768) >> 16) + 64;
					int u1 = static_cast<int>((-9683 * r - 19017 * g + 28700 * b + 32768) >> 16) + 512;
					int v1 = static_cast<int>((28700 * r - 24033 * g - 4667 * b + 32768) >> 16) + 512;

					dPtr->x = static_cast<uint16_t>(std::min<int>(std::max<int>(y0, 0), 1023) << 6);
					dPtr->y = static_cast<uint16_t>(std::min<int>(std::max<int>((u0 + u1) >> 1, 0), 1023) << 6);
					dPtr->z = static_cast<uint16_t>(std::min<int>(std::max<int>(y1, 0), 1023) << 6);
					dPtr->w = static_cast<uint16_t>(std::min<int>(std::max<int>((v0 + v1) >> 1, 0), 1023) << 6);
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_Y216:
			if (size >= sizeof(XMUSHORTN4))
			{
				XMUSHORTN4 * __restrict dPtr = static_cast<XMUSHORTN4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUSHORTN4) + 1); icount += sizeof(XMUSHORTN4))
				{
					if (sPtr >= ePtr) break;

					XMUSHORTN4 rgb1;
					XMStoreUShortN4(&rgb1, *sPtr++);

					// See Y416
					int64_t r = int64_t(rgb1.x);
					int64_t g = int64_t(rgb1.y);
					int64_t b = int64_t(rgb1.z);

					int y0 = static_cast<int>((16763 * r + 32910 * g + 6537 * b + 32768) >> 16) + 4096;
					int u0 = static_cast<int>((-9674 * r - 18998 * g + 28672 * b + 32768) >> 16) + 32768;
					int v0 = static_cast<int>((28672 * r - 24010 * g - 4662 * b + 32768) >> 16) + 32768;

					XMUSHORTN4 rgb2;
					if (sPtr < ePtr)
					{
						XMStoreUShortN4(&rgb2, *sPtr++);
					}
					else
					{
						rgb2.x = rgb2.y = rgb2.z = rgb2.w = 0;
					}

					r = int64_t(rgb2.x);
					g = int64_t(rgb2.y);
					b = int64_t(rgb2.z);

					int y1 = static_cast<int>((16763 * r + 32910 * g + 6537 * b + 32768) >> 16) + 4096;
					int u1 = static_cast<int>((-9674 * r - 18998 * g + 28672 * b + 32768) >> 16) + 32768;
					int v1 = static_cast<int>((28672 * r - 24010 * g - 4662 * b + 32768) >> 16) + 32768;

					dPtr->x = static_cast<uint16_t>(std::min<int>(std::max<int>(y0, 0), 65535));
					dPtr->y = static_cast<uint16_t>(std::min<int>(std::max<int>((u0 + u1) >> 1, 0), 65535));
					dPtr->z = static_cast<uint16_t>(std::min<int>(std::max<int>(y1, 0), 65535));
					dPtr->w = static_cast<uint16_t>(std::min<int>(std::max<int>((v0 + v1) >> 1, 0), 65535));
					++dPtr;
				}
				return true;
			}
			return false;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			if (size >= sizeof(XMUNIBBLE4))
			{
				static const XMVECTORF32 s_Scale = { { { 15.f, 15.f, 15.f, 15.f } } };
				XMUNIBBLE4 * __restrict dPtr = static_cast<XMUNIBBLE4*>(pDestination);
				for (uint32_t icount = 0; icount < (size - sizeof(XMUNIBBLE4) + 1); icount += sizeof(XMUNIBBLE4))
				{
					if (sPtr >= ePtr) break;
					XMVECTOR v = XMVectorSwizzle<2, 1, 0, 3>(*sPtr++);
					v = XMVectorMultiply(v, s_Scale);
					XMStoreUNibble4(dPtr++, v);
				}
				return true;
			}
			return false;

		default:
			return false;
		}
	}

#undef STORE_SCANLINE

	namespace
	{
		struct ConvertData
		{
			DXGI_FORMAT format;
			uint32_t datasize;
			DWORD flags;
		};

		const ConvertData g_ConvertTable[] =
		{
			{ DXGI_FORMAT_R32G32B32A32_FLOAT,           32, CONVF_FLOAT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R32G32B32A32_UINT,            32, CONVF_UINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R32G32B32A32_SINT,            32, CONVF_SINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R32G32B32_FLOAT,              32, CONVF_FLOAT | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R32G32B32_UINT,               32, CONVF_UINT | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R32G32B32_SINT,               32, CONVF_SINT | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R16G16B16A16_FLOAT,           16, CONVF_FLOAT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R16G16B16A16_UNORM,           16, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R16G16B16A16_UINT,            16, CONVF_UINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R16G16B16A16_SNORM,           16, CONVF_SNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R16G16B16A16_SINT,            16, CONVF_SINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R32G32_FLOAT,                 32, CONVF_FLOAT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R32G32_UINT,                  32, CONVF_UINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R32G32_SINT,                  32, CONVF_SINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_D32_FLOAT_S8X24_UINT,         32, CONVF_FLOAT | CONVF_DEPTH | CONVF_STENCIL },
			{ DXGI_FORMAT_R10G10B10A2_UNORM,            10, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R10G10B10A2_UINT,             10, CONVF_UINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R11G11B10_FLOAT,              10, CONVF_FLOAT | CONVF_POS_ONLY | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R8G8B8A8_UNORM,               8, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,          8, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R8G8B8A8_UINT,                8, CONVF_UINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R8G8B8A8_SNORM,               8, CONVF_SNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R8G8B8A8_SINT,                8, CONVF_SINT | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_R16G16_FLOAT,                 16, CONVF_FLOAT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R16G16_UNORM,                 16, CONVF_UNORM | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R16G16_UINT,                  16, CONVF_UINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R16G16_SNORM,                 16, CONVF_SNORM | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R16G16_SINT,                  16, CONVF_SINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_D32_FLOAT,                    32, CONVF_FLOAT | CONVF_DEPTH },
			{ DXGI_FORMAT_R32_FLOAT,                    32, CONVF_FLOAT | CONVF_R },
			{ DXGI_FORMAT_R32_UINT,                     32, CONVF_UINT | CONVF_R },
			{ DXGI_FORMAT_R32_SINT,                     32, CONVF_SINT | CONVF_R },
			{ DXGI_FORMAT_D24_UNORM_S8_UINT,            32, CONVF_UNORM | CONVF_DEPTH | CONVF_STENCIL },
			{ DXGI_FORMAT_R8G8_UNORM,                   8, CONVF_UNORM | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R8G8_UINT,                    8, CONVF_UINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R8G8_SNORM,                   8, CONVF_SNORM | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R8G8_SINT,                    8, CONVF_SINT | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_R16_FLOAT,                    16, CONVF_FLOAT | CONVF_R },
			{ DXGI_FORMAT_D16_UNORM,                    16, CONVF_UNORM | CONVF_DEPTH },
			{ DXGI_FORMAT_R16_UNORM,                    16, CONVF_UNORM | CONVF_R },
			{ DXGI_FORMAT_R16_UINT,                     16, CONVF_UINT | CONVF_R },
			{ DXGI_FORMAT_R16_SNORM,                    16, CONVF_SNORM | CONVF_R },
			{ DXGI_FORMAT_R16_SINT,                     16, CONVF_SINT | CONVF_R },
			{ DXGI_FORMAT_R8_UNORM,                     8, CONVF_UNORM | CONVF_R },
			{ DXGI_FORMAT_R8_UINT,                      8, CONVF_UINT | CONVF_R },
			{ DXGI_FORMAT_R8_SNORM,                     8, CONVF_SNORM | CONVF_R },
			{ DXGI_FORMAT_R8_SINT,                      8, CONVF_SINT | CONVF_R },
			{ DXGI_FORMAT_A8_UNORM,                     8, CONVF_UNORM | CONVF_A },
			{ DXGI_FORMAT_R1_UNORM,                     1, CONVF_UNORM | CONVF_R },
			{ DXGI_FORMAT_R9G9B9E5_SHAREDEXP,           9, CONVF_FLOAT | CONVF_SHAREDEXP | CONVF_POS_ONLY | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R8G8_B8G8_UNORM,              8, CONVF_UNORM | CONVF_PACKED | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_G8R8_G8B8_UNORM,              8, CONVF_UNORM | CONVF_PACKED | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_BC1_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC1_UNORM_SRGB,               8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC2_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC2_UNORM_SRGB,               8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC3_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC3_UNORM_SRGB,               8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC4_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R },
			{ DXGI_FORMAT_BC4_SNORM,                    8, CONVF_SNORM | CONVF_BC | CONVF_R },
			{ DXGI_FORMAT_BC5_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_BC5_SNORM,                    8, CONVF_SNORM | CONVF_BC | CONVF_R | CONVF_G },
			{ DXGI_FORMAT_B5G6R5_UNORM,                 5, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_B5G5R5A1_UNORM,               5, CONVF_UNORM | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_B8G8R8A8_UNORM,               8, CONVF_UNORM | CONVF_BGR | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_B8G8R8X8_UNORM,               8, CONVF_UNORM | CONVF_BGR | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,   10, CONVF_UNORM | CONVF_XR | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,          8, CONVF_UNORM | CONVF_BGR | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,          8, CONVF_UNORM | CONVF_BGR | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_BC6H_UF16,                    16, CONVF_FLOAT | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC6H_SF16,                    16, CONVF_FLOAT | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC7_UNORM,                    8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_BC7_UNORM_SRGB,               8, CONVF_UNORM | CONVF_BC | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_AYUV,                         8, CONVF_UNORM | CONVF_YUV | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_Y410,                         10, CONVF_UNORM | CONVF_YUV | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_Y416,                         16, CONVF_UNORM | CONVF_YUV | CONVF_R | CONVF_G | CONVF_B | CONVF_A },
			{ DXGI_FORMAT_YUY2,                         8, CONVF_UNORM | CONVF_YUV | CONVF_PACKED | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_Y210,                         10, CONVF_UNORM | CONVF_YUV | CONVF_PACKED | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_Y216,                         16, CONVF_UNORM | CONVF_YUV | CONVF_PACKED | CONVF_R | CONVF_G | CONVF_B },
			{ DXGI_FORMAT_B4G4R4A4_UNORM,               4, CONVF_UNORM | CONVF_BGR | CONVF_R | CONVF_G | CONVF_B | CONVF_A }
		};

#pragma prefast( suppress : 25004, "Signature must match bsearch_s" );
		int __cdecl ConvertCompare(void *context, const void* ptr1, const void *ptr2) noexcept
		{
			UNREFERENCED_PARAMETER(context);
			auto p1 = static_cast<const ConvertData*>(ptr1);
			auto p2 = static_cast<const ConvertData*>(ptr2);
			if (p1->format == p2->format) return 0;
			else return (p1->format < p2->format) ? -1 : 1;
		}
	}

	_Use_decl_annotations_
		DWORD _GetConvertFlags(DXGI_FORMAT format)
	{
#ifdef _DEBUG
		// Ensure conversion table is in ascending order
		assert(_countof(g_ConvertTable) > 0);
		DXGI_FORMAT lastvalue = g_ConvertTable[0].format;
		for (uint32_t index = 1; index < _countof(g_ConvertTable); ++index)
		{
			assert(g_ConvertTable[index].format > lastvalue);
			lastvalue = g_ConvertTable[index].format;
		}
#endif

		ConvertData key = { format, 0, 0 };
		auto in = reinterpret_cast<const ConvertData*>(bsearch_s(&key, g_ConvertTable, _countof(g_ConvertTable), sizeof(ConvertData),
			ConvertCompare, nullptr));
		return (in) ? in->flags : 0;
	}

	_Use_decl_annotations_
		void _ConvertScanline(
			XMVECTOR* pBuffer,
			uint32_t count,
			DXGI_FORMAT outFormat,
			DXGI_FORMAT inFormat,
			DWORD flags)
	{
		assert(pBuffer && count > 0 && ((reinterpret_cast<uintptr_t>(pBuffer) & 0xF) == 0));

		if (!pBuffer)
			return;

#ifdef _DEBUG
		// Ensure conversion table is in ascending order
		assert(_countof(g_ConvertTable) > 0);
		DXGI_FORMAT lastvalue = g_ConvertTable[0].format;
		for (uint32_t index = 1; index < _countof(g_ConvertTable); ++index)
		{
			assert(g_ConvertTable[index].format > lastvalue);
			lastvalue = g_ConvertTable[index].format;
		}
#endif

		// Determine conversion details about source and dest formats
		ConvertData key = { inFormat, 0, 0 };
		auto in = reinterpret_cast<const ConvertData*>(
			bsearch_s(&key, g_ConvertTable, _countof(g_ConvertTable), sizeof(ConvertData), ConvertCompare, nullptr));
		key.format = outFormat;
		auto out = reinterpret_cast<const ConvertData*>(
			bsearch_s(&key, g_ConvertTable, _countof(g_ConvertTable), sizeof(ConvertData), ConvertCompare, nullptr));
		if (!in || !out)
		{
			assert(false);
			return;
		}

		assert(_GetConvertFlags(inFormat) == in->flags);
		assert(_GetConvertFlags(outFormat) == out->flags);

		// Handle SRGB filtering modes
		switch (inFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			flags |= TEX_FILTER_SRGB_IN;
			break;

		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			flags &= ~static_cast<uint32_t>(TEX_FILTER_SRGB_IN);
			break;

		default:
			break;
		}

		switch (outFormat)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			flags |= TEX_FILTER_SRGB_OUT;
			break;

		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			flags &= ~static_cast<uint32_t>(TEX_FILTER_SRGB_OUT);
			break;

		default:
			break;
		}

		if ((flags & (TEX_FILTER_SRGB_IN | TEX_FILTER_SRGB_OUT)) == (TEX_FILTER_SRGB_IN | TEX_FILTER_SRGB_OUT))
		{
			flags &= ~static_cast<uint32_t>(TEX_FILTER_SRGB_IN | TEX_FILTER_SRGB_OUT);
		}

		// sRGB input processing (sRGB -> Linear RGB)
		if (flags & TEX_FILTER_SRGB_IN)
		{
			if (!(in->flags & CONVF_DEPTH) && ((in->flags & CONVF_FLOAT) || (in->flags & CONVF_UNORM)))
			{
				XMVECTOR* ptr = pBuffer;
				for (uint32_t i = 0; i < count; ++i, ++ptr)
				{
					*ptr = XMColorSRGBToRGB(*ptr);
				}
			}
		}

		// Handle conversion special cases
		DWORD diffFlags = in->flags ^ out->flags;
		if (diffFlags != 0)
		{
			if (diffFlags & CONVF_DEPTH)
			{
				//--- Depth conversions ---
				if (in->flags & CONVF_DEPTH)
				{
					// CONVF_DEPTH -> !CONVF_DEPTH
					if (in->flags & CONVF_STENCIL)
					{
						// Stencil -> Alpha
						static const XMVECTORF32 S = { { { 1.f, 1.f, 1.f, 255.f } } };

						if (out->flags & CONVF_UNORM)
						{
							// UINT -> UNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorSplatY(v);
								v1 = XMVectorClamp(v1, g_XMZero, S);
								v1 = XMVectorDivide(v1, S);
								*ptr++ = XMVectorSelect(v1, v, g_XMSelect1110);
							}
						}
						else if (out->flags & CONVF_SNORM)
						{
							// UINT -> SNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorSplatY(v);
								v1 = XMVectorClamp(v1, g_XMZero, S);
								v1 = XMVectorDivide(v1, S);
								v1 = XMVectorMultiplyAdd(v1, g_XMTwo, g_XMNegativeOne);
								*ptr++ = XMVectorSelect(v1, v, g_XMSelect1110);
							}
						}
						else
						{
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorSplatY(v);
								*ptr++ = XMVectorSelect(v1, v, g_XMSelect1110);
							}
						}
					}

					// Depth -> RGB
					if ((out->flags & CONVF_UNORM) && (in->flags & CONVF_FLOAT))
					{
						// Depth FLOAT -> UNORM
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSaturate(v);
							v1 = XMVectorSplatX(v1);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
						}
					}
					else if (out->flags & CONVF_SNORM)
					{
						if (in->flags & CONVF_UNORM)
						{
							// Depth UNORM -> SNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorMultiplyAdd(v, g_XMTwo, g_XMNegativeOne);
								v1 = XMVectorSplatX(v1);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
							}
						}
						else
						{
							// Depth FLOAT -> SNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorClamp(v, g_XMNegativeOne, g_XMOne);
								v1 = XMVectorSplatX(v1);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
							}
						}
					}
					else
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatX(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
						}
					}
				}
				else
				{
					// !CONVF_DEPTH -> CONVF_DEPTH

					// RGB -> Depth (red channel)
					switch (flags & (TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_GREEN | TEX_FILTER_RGB_COPY_BLUE))
					{
					case TEX_FILTER_RGB_COPY_GREEN:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatY(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
						}
					}
					break;

					case TEX_FILTER_RGB_COPY_BLUE:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatZ(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
						}
					}
					break;

					default:
						if ((in->flags & CONVF_UNORM) && ((in->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G | CONVF_B)))
						{
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVector3Dot(v, g_Grayscale);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
							}
							break;
						}

#ifdef _MSC_VER
						__fallthrough;
#endif
#ifdef __clang__
						[[clang::fallthrough]];
#endif

					case TEX_FILTER_RGB_COPY_RED:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatX(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
						}
					}
					break;
					}

					// Finialize type conversion for depth (red channel)
					if (out->flags & CONVF_UNORM)
					{
						if (in->flags & CONVF_SNORM)
						{
							// SNORM -> UNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
							}
						}
						else if (in->flags & CONVF_FLOAT)
						{
							// FLOAT -> UNORM
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorSaturate(v);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
							}
						}
					}

					if (out->flags & CONVF_STENCIL)
					{
						// Alpha -> Stencil (green channel)
						static const XMVECTORU32 select0100 = { { { XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0 } } };
						static const XMVECTORF32 S = { { { 255.f, 255.f, 255.f, 255.f } } };

						if (in->flags & CONVF_UNORM)
						{
							// UNORM -> UINT
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorMultiply(v, S);
								v1 = XMVectorSplatW(v1);
								*ptr++ = XMVectorSelect(v, v1, select0100);
							}
						}
						else if (in->flags & CONVF_SNORM)
						{
							// SNORM -> UINT
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
								v1 = XMVectorMultiply(v1, S);
								v1 = XMVectorSplatW(v1);
								*ptr++ = XMVectorSelect(v, v1, select0100);
							}
						}
						else
						{
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVectorSplatW(v);
								*ptr++ = XMVectorSelect(v, v1, select0100);
							}
						}
					}
				}
			}
			else if (out->flags & CONVF_DEPTH)
			{
				// CONVF_DEPTH -> CONVF_DEPTH
				if (diffFlags & CONVF_FLOAT)
				{
					if (in->flags & CONVF_FLOAT)
					{
						// FLOAT -> UNORM depth, preserve stencil
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSaturate(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1000);
						}
					}
				}
			}
			else if (out->flags & CONVF_UNORM)
			{
				//--- Converting to a UNORM ---
				if (in->flags & CONVF_SNORM)
				{
					// SNORM -> UNORM
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						*ptr++ = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
					}
				}
				else if (in->flags & CONVF_FLOAT)
				{
					XMVECTOR* ptr = pBuffer;
					if (!(in->flags & CONVF_POS_ONLY) && (flags & TEX_FILTER_FLOAT_X2BIAS))
					{
						// FLOAT -> UNORM (x2 bias)
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							v = XMVectorClamp(v, g_XMNegativeOne, g_XMOne);
							*ptr++ = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
						}
					}
					else
					{
						// FLOAT -> UNORM
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							*ptr++ = XMVectorSaturate(v);
						}
					}
				}
			}
			else if (out->flags & CONVF_SNORM)
			{
				//--- Converting to a SNORM ---
				if (in->flags & CONVF_UNORM)
				{
					// UNORM -> SNORM
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						*ptr++ = XMVectorMultiplyAdd(v, g_XMTwo, g_XMNegativeOne);
					}
				}
				else if (in->flags & CONVF_FLOAT)
				{
					XMVECTOR* ptr = pBuffer;
					if ((in->flags & CONVF_POS_ONLY) && (flags & TEX_FILTER_FLOAT_X2BIAS))
					{
						// FLOAT (positive only, x2 bias) -> SNORM
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							v = XMVectorSaturate(v);
							*ptr++ = XMVectorMultiplyAdd(v, g_XMTwo, g_XMNegativeOne);
						}
					}
					else
					{
						// FLOAT -> SNORM
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							*ptr++ = XMVectorClamp(v, g_XMNegativeOne, g_XMOne);
						}
					}
				}
			}
			else if (diffFlags & CONVF_UNORM)
			{
				//--- Converting from a UNORM ---
				assert(in->flags & CONVF_UNORM);
				if (out->flags & CONVF_FLOAT)
				{
					if (!(out->flags & CONVF_POS_ONLY) && (flags & TEX_FILTER_FLOAT_X2BIAS))
					{
						// UNORM (x2 bias) -> FLOAT
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							*ptr++ = XMVectorMultiplyAdd(v, g_XMTwo, g_XMNegativeOne);
						}
					}
				}
			}
			else if (diffFlags & CONVF_POS_ONLY)
			{
				if (flags & TEX_FILTER_FLOAT_X2BIAS)
				{
					if (in->flags & CONVF_POS_ONLY)
					{
						if (out->flags & CONVF_FLOAT)
						{
							// FLOAT (positive only, x2 bias) -> FLOAT
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								v = XMVectorSaturate(v);
								*ptr++ = XMVectorMultiplyAdd(v, g_XMTwo, g_XMNegativeOne);
							}
						}
					}
					else if (out->flags & CONVF_POS_ONLY)
					{
						if (in->flags & CONVF_FLOAT)
						{
							// FLOAT -> FLOAT (positive only, x2 bias)
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								v = XMVectorClamp(v, g_XMNegativeOne, g_XMOne);
								*ptr++ = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
							}
						}
						else if (in->flags & CONVF_SNORM)
						{
							// SNORM -> FLOAT (positive only, x2 bias)
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								*ptr++ = XMVectorMultiplyAdd(v, g_XMOneHalf, g_XMOneHalf);
							}
						}
					}
				}
			}

			// !CONVF_A -> CONVF_A is handled because LoadScanline ensures alpha defaults to 1.0 for no-alpha formats

			// CONVF_PACKED cases are handled because LoadScanline/StoreScanline handles packing/unpacking

			if (((out->flags & CONVF_RGBA_MASK) == CONVF_A) && !(in->flags & CONVF_A))
			{
				// !CONVF_A -> A format
				switch (flags & (TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_GREEN | TEX_FILTER_RGB_COPY_BLUE))
				{
				case TEX_FILTER_RGB_COPY_GREEN:
				{
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						*ptr++ = XMVectorSplatY(v);
					}
				}
				break;

				case TEX_FILTER_RGB_COPY_BLUE:
				{
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						*ptr++ = XMVectorSplatZ(v);
					}
				}
				break;

				default:
					if ((in->flags & CONVF_UNORM) && ((in->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G | CONVF_B)))
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							*ptr++ = XMVector3Dot(v, g_Grayscale);
						}
						break;
					}

#ifdef _MSC_VER
					__fallthrough;
#endif
#ifdef __clang__
					[[clang::fallthrough]];
#endif

				case TEX_FILTER_RGB_COPY_RED:
				{
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						*ptr++ = XMVectorSplatX(v);
					}
				}
				break;
				}
			}
			else if (((in->flags & CONVF_RGBA_MASK) == CONVF_A) && !(out->flags & CONVF_A))
			{
				// A format -> !CONVF_A
				XMVECTOR* ptr = pBuffer;
				for (uint32_t i = 0; i < count; ++i)
				{
					XMVECTOR v = *ptr;
					*ptr++ = XMVectorSplatW(v);
				}
			}
			else if ((in->flags & CONVF_RGB_MASK) == CONVF_R)
			{
				if ((out->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G | CONVF_B))
				{
					// R format -> RGB format
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						XMVECTOR v1 = XMVectorSplatX(v);
						*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
					}
				}
				else if ((out->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G))
				{
					// R format -> RG format
					XMVECTOR* ptr = pBuffer;
					for (uint32_t i = 0; i < count; ++i)
					{
						XMVECTOR v = *ptr;
						XMVECTOR v1 = XMVectorSplatX(v);
						*ptr++ = XMVectorSelect(v, v1, g_XMSelect1100);
					}
				}
			}
			else if ((in->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G | CONVF_B))
			{
				if ((out->flags & CONVF_RGB_MASK) == CONVF_R)
				{
					// RGB format -> R format
					switch (flags & (TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_GREEN | TEX_FILTER_RGB_COPY_BLUE))
					{
					case TEX_FILTER_RGB_COPY_GREEN:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatY(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
						}
					}
					break;

					case TEX_FILTER_RGB_COPY_BLUE:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSplatZ(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
						}
					}
					break;

					default:
						if (in->flags & CONVF_UNORM)
						{
							XMVECTOR* ptr = pBuffer;
							for (uint32_t i = 0; i < count; ++i)
							{
								XMVECTOR v = *ptr;
								XMVECTOR v1 = XMVector3Dot(v, g_Grayscale);
								*ptr++ = XMVectorSelect(v, v1, g_XMSelect1110);
							}
							break;
						}

#ifdef _MSC_VER
						__fallthrough;
#endif
#ifdef __clang__
						[[clang::fallthrough]];
#endif

					case TEX_FILTER_RGB_COPY_RED:
						// Leave data unchanged and the store will handle this...
						break;
					}
				}
				else if ((out->flags & CONVF_RGB_MASK) == (CONVF_R | CONVF_G))
				{
					// RGB format -> RG format
					switch (flags & (TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_GREEN | TEX_FILTER_RGB_COPY_BLUE))
					{
					case TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_BLUE:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSwizzle<0, 2, 0, 2>(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1100);
						}
					}
					break;

					case TEX_FILTER_RGB_COPY_GREEN | TEX_FILTER_RGB_COPY_BLUE:
					{
						XMVECTOR* ptr = pBuffer;
						for (uint32_t i = 0; i < count; ++i)
						{
							XMVECTOR v = *ptr;
							XMVECTOR v1 = XMVectorSwizzle<1, 2, 3, 0>(v);
							*ptr++ = XMVectorSelect(v, v1, g_XMSelect1100);
						}
					}
					break;

					case TEX_FILTER_RGB_COPY_RED | TEX_FILTER_RGB_COPY_GREEN:
					default:
						// Leave data unchanged and the store will handle this...
						break;
					}
				}
			}
		}

		// sRGB output processing (Linear RGB -> sRGB)
		if (flags & TEX_FILTER_SRGB_OUT)
		{
			if (!(out->flags & CONVF_DEPTH) && ((out->flags & CONVF_FLOAT) || (out->flags & CONVF_UNORM)))
			{
				XMVECTOR* ptr = pBuffer;
				for (uint32_t i = 0; i < count; ++i, ++ptr)
				{
					*ptr = XMColorRGBToSRGB(*ptr);
				}
			}
		}
	}

	DDSLoader::DDSLoader(const std::string& ddsFile)
		: fileName(ddsFile),
		width(0), height(0), depth(0), arraySize(0), mipLevels(0), miscFlags(0), miscFlags2(0), rowPitch(0),
		pixelSize(0), pngSize(0), pixels(nullptr), pngBuffer(nullptr)
	{

	}

	DDSLoader::~DDSLoader()
	{

	}

	bool DDSLoader::ConvertDDSToPng()
	{
		if (!LoadDDSFile())
			return false;

		png_image image;
		memset(&image, 0, sizeof(image));
		image.version = PNG_IMAGE_VERSION;
		image.width = width;
		image.height = height;
		image.format = PNG_FORMAT_RGBA;
		uint8_t* buffer = pixels.get();
		if (!png_image_write_to_memory(&image, nullptr, &pngSize, 0, buffer, 0, nullptr))
			return false;

		pngBuffer.reset(new uint8_t[pngSize]);
		if (!png_image_write_to_memory(&image, pngBuffer.get(), &pngSize, 0, buffer, 0, nullptr))
			return false;
		return true;
	}

	bool DDSLoader::LoadDDSFile()
	{
		CParaFile file(fileName.c_str());
		uint32_t fileSize = file.getSize();
		std::unique_ptr<uint8_t[]> ddsData = nullptr;
		ddsData.reset(new uint8_t[fileSize]);
		if (file.read(ddsData.get(), fileSize) != fileSize)
			return false;

		uint32_t flags = DDS_FLAGS_NONE;
		uint32_t convFlags = 0;
		if (!DecodeDDSHeader(ddsData.get(), fileSize, flags, convFlags))
			return false;

		uint32_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER);
		if (convFlags & CONV_FLAGS_DX10)
			offset += sizeof(DDS_HEADER_DXT10);

		if (convFlags & CONV_FLAGS_PAL8)
		{
			//
		}

		if ((convFlags & CONV_FLAGS_EXPAND) || (flags & (DDS_FLAGS_LEGACY_DWORD | DDS_FLAGS_BAD_DXTN_TAILS)))
		{
			//
		}
		else
		{
		}
		if (IsCompressed(format))
		{
			DXGI_FORMAT srcFormat = format;
			format = DefaultDecompress(format);
			if (format == DXGI_FORMAT_UNKNOWN)
				return false;

			if (!InitializeImage(DDS_FLAGS_NONE))
				return false;
			if (!Decompress(ddsData.get() + offset, srcFormat))
				return false;
		}
		return true;
	}

	bool DDSLoader::DecodeDDSHeader(const void* source, uint32_t size, uint32_t flags, uint32_t& convFlags)
	{
		if (!source) return false;
		if (size < (sizeof(DDS_HEADER) + sizeof(uint32_t))) return false;

		auto magic = *reinterpret_cast<const uint32_t*>(source);
		if (magic != DDS_MAGIC)
			return false;

		auto header = reinterpret_cast<const DDS_HEADER*>(static_cast<const uint8_t*>(source) + sizeof(uint32_t));
		if (header->size != sizeof(DDS_HEADER) || header->ddspf.size != sizeof(DDS_PIXELFORMAT))
			return false;

		mipLevels = header->mipMapCount;
		if (mipLevels == 0) mipLevels = 1;

		if ((header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
		{
			if (size < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
				return false;
			//
		}
		else
		{
			arraySize = 1;
			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				//
			}
			else
			{
				if (header->caps2 & DDS_CUBEMAP)
				{
					//
				}
				width = header->width;
				height = header->height;
				depth = 1;
				dimension = TEX_DIMENSION_TEXTURE2D;
			}

			format = GetDXGIFormat(*header, header->ddspf, flags, convFlags);
			if (format == DXGI_FORMAT_UNKNOWN)
				return false;
			if (convFlags & CONV_FLAGS_PMALPHA)
				miscFlags2 |= TEX_ALPHA_MODE_PREMULTIPLIED;

			if (flags & DDS_FLAGS_EXPAND_LUMINANCE)
			{
				//
			}
		}

		if (flags & DDS_FLAGS_FORCE_RGB)
		{
			//
		}

		// Special flag for handling 16bpp formats
		if (flags & DDS_FLAGS_NO_16BPP)
		{
			//
		}

		return true;
	}

	DXGI_FORMAT DDSLoader::GetDXGIFormat(const DDS_HEADER& hdr, const DDS_PIXELFORMAT& ddpf, uint32_t flags, uint32_t& convFlags)
	{
		uint32_t ddpfFlags = ddpf.flags;
		if (hdr.reserved1[9] == MAKEFOURCC('N', 'V', 'T', 'T'))
		{
			// Clear out non-standard nVidia DDS flags
			ddpfFlags &= ~0xC0000000 /* DDPF_SRGB | DDPF_NORMAL */;
		}

		const uint32_t MAP_SIZE = sizeof(g_LegacyDDSMap) / sizeof(LegacyDDS);
		uint32_t index = 0;
		for (index = 0; index < MAP_SIZE; ++index)
		{
			const LegacyDDS* entry = &g_LegacyDDSMap[index];

			if ((ddpfFlags & DDS_FOURCC) && (entry->ddpf.flags & DDS_FOURCC))
			{
				// In case of FourCC codes, ignore any other bits in ddpf.flags
				if (ddpf.fourCC == entry->ddpf.fourCC)
					break;
			}
			else if (ddpfFlags == entry->ddpf.flags)
			{
				if (entry->ddpf.flags & DDS_PAL8)
				{
					if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount)
						break;
				}
				else if (entry->ddpf.flags & DDS_ALPHA)
				{
					if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount
						&& ddpf.ABitMask == entry->ddpf.ABitMask)
						break;
				}
				else if (entry->ddpf.flags & DDS_LUMINANCE)
				{
					if (entry->ddpf.flags & DDS_ALPHAPIXELS)
					{
						// LUMINANCEA
						if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount
							&& ddpf.RBitMask == entry->ddpf.RBitMask
							&& ddpf.ABitMask == entry->ddpf.ABitMask)
							break;
					}
					else
					{
						// LUMINANCE
						if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount
							&& ddpf.RBitMask == entry->ddpf.RBitMask)
							break;
					}
				}
				else if (entry->ddpf.flags & DDS_BUMPDUDV)
				{
					if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount
						&& ddpf.RBitMask == entry->ddpf.RBitMask
						&& ddpf.GBitMask == entry->ddpf.GBitMask
						&& ddpf.BBitMask == entry->ddpf.BBitMask
						&& ddpf.ABitMask == entry->ddpf.ABitMask)
						break;
				}
				else if (ddpf.RGBBitCount == entry->ddpf.RGBBitCount)
				{
					if (entry->ddpf.flags & DDS_ALPHAPIXELS)
					{
						// RGBA
						if (ddpf.RBitMask == entry->ddpf.RBitMask
							&& ddpf.GBitMask == entry->ddpf.GBitMask
							&& ddpf.BBitMask == entry->ddpf.BBitMask
							&& ddpf.ABitMask == entry->ddpf.ABitMask)
							break;
					}
					else
					{
						// RGB
						if (ddpf.RBitMask == entry->ddpf.RBitMask
							&& ddpf.GBitMask == entry->ddpf.GBitMask
							&& ddpf.BBitMask == entry->ddpf.BBitMask)
							break;
					}
				}
			}
		}

		if (index >= MAP_SIZE)
			return DXGI_FORMAT_UNKNOWN;

		DWORD cflags = g_LegacyDDSMap[index].convFlags;
		DXGI_FORMAT format = g_LegacyDDSMap[index].format;

		if ((cflags & CONV_FLAGS_EXPAND) && (flags & DDS_FLAGS_NO_LEGACY_EXPANSION))
			return DXGI_FORMAT_UNKNOWN;

		if ((format == DXGI_FORMAT_R10G10B10A2_UNORM) && (flags & DDS_FLAGS_NO_R10B10G10A2_FIXUP))
		{
			cflags ^= CONV_FLAGS_SWIZZLE;
		}

		if ((hdr.reserved1[9] == MAKEFOURCC('N', 'V', 'T', 'T'))
			&& (ddpf.flags & 0x40000000 /* DDPF_SRGB */))
		{
			format = MakeSRGB(format);
		}

		convFlags = cflags;

		return format;
	}

	DXGI_FORMAT DDSLoader::MakeSRGB(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_BC1_UNORM:
			return DXGI_FORMAT_BC1_UNORM_SRGB;

		case DXGI_FORMAT_BC2_UNORM:
			return DXGI_FORMAT_BC2_UNORM_SRGB;

		case DXGI_FORMAT_BC3_UNORM:
			return DXGI_FORMAT_BC3_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
			return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		case DXGI_FORMAT_BC7_UNORM:
			return DXGI_FORMAT_BC7_UNORM_SRGB;

		default:
			return fmt;
		}
	}

	bool DDSLoader::InitializeImage(uint32_t flags)
	{
		switch (dimension)
		{
		case TEX_DIMENSION_TEXTURE1D:
			//
			break;
		case TEX_DIMENSION_TEXTURE2D:
			if (!width || !height || depth != 1 || !arraySize)
				return false;
			if (miscFlags & TEX_MISC_TEXTURECUBE)
			{
				if (arraySize % 6 != 0)
					return false;
			}
			if (!CalculateMipLevels(width, height, mipLevels))
				return false;

			if (!ComputePitch(format, width, height, rowPitch, pixelSize, flags))
				return false;
			pixels.reset(new uint8_t[pixelSize]);
			break;
		case TEX_DIMENSION_TEXTURE3D:
			//
			break;
		default:
			break;
		}
		return true;
	}

	bool DDSLoader::CalculateMipLevels(uint32_t width, uint32_t height, uint32_t& mipLevels)
	{
		if (mipLevels > 1)
		{
			uint32_t maxMips = CountMips(width, height);
			if (mipLevels > maxMips)
				return false;
		}
		else if (mipLevels == 0)
		{
			mipLevels = CountMips(width, height);
		}
		else
		{
			mipLevels = 1;
		}
		return true;
	}

	uint32_t DDSLoader::CountMips(uint32_t width, uint32_t height)
	{
		uint32_t mipLevels = 1;

		while (height > 1 || width > 1)
		{
			if (height > 1)
				height >>= 1;

			if (width > 1)
				width >>= 1;

			++mipLevels;
		}

		return mipLevels;
	}

	bool DDSLoader::ComputePitch(DXGI_FORMAT fmt, uint32_t width, uint32_t height, uint32_t& rowPitch, uint32_t& slicePitch, uint32_t flags)
	{
		uint32_t pitch = 0;
		uint32_t slice = 0;

		switch (static_cast<int>(fmt))
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			assert(IsCompressed(fmt));
			{
				//
			}
			break;
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			assert(IsCompressed(fmt));
			{
				if (flags & CP_FLAGS_BAD_DXTN_TAILS)
				{
					uint32_t nbw = width >> 2;
					uint32_t nbh = height >> 2;
					pitch = std::max(1u, nbw * 16u);
					slice = std::max(1u, pitch * nbh);
				}
				else
				{
					uint32_t nbw = std::max(1u, (width + 3u) / 4u);
					uint32_t nbh = std::max(1u, (height + 3u) / 4u);
					pitch = nbw * 16u;
					slice = pitch * nbh;
				}
			}
			break;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_YUY2:
			break;
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			break;
		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
			break;
		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			break;
		case DXGI_FORMAT_NV11:
			break;
		default:
			assert(!IsCompressed(fmt));
			{
				uint32_t bpp;

				if (flags & CP_FLAGS_24BPP)
					bpp = 24;
				else if (flags & CP_FLAGS_16BPP)
					bpp = 16;
				else if (flags & CP_FLAGS_8BPP)
					bpp = 8;
				else
					bpp = BitsPerPixel(fmt);

				if (!bpp)
					return false;

				if (flags & (CP_FLAGS_LEGACY_DWORD | CP_FLAGS_PARAGRAPH | CP_FLAGS_YMM | CP_FLAGS_ZMM | CP_FLAGS_PAGE4K))
				{
					if (flags & CP_FLAGS_PAGE4K)
					{
						pitch = ((width * bpp + 32767u) / 32768u) * 4096u;
						slice = pitch * height;
					}
					else if (flags & CP_FLAGS_ZMM)
					{
						pitch = ((width * bpp + 511u) / 512u) * 64u;
						slice = pitch * height;
					}
					else if (flags & CP_FLAGS_YMM)
					{
						pitch = ((width * bpp + 255u) / 256u) * 32u;
						slice = pitch * height;
					}
					else if (flags & CP_FLAGS_PARAGRAPH)
					{
						pitch = ((width * bpp + 127u) / 128u) * 16u;
						slice = pitch * height;
					}
					else // DWORD alignment
					{
						// Special computation for some incorrectly created DDS files based on
						// legacy DirectDraw assumptions about pitch alignment
						pitch = ((width * bpp + 31u) / 32u) * sizeof(uint32_t);
						slice = pitch * height;
					}
				}
				else
				{
					// Default byte alignment
					pitch = (width * bpp + 7u) / 8u;
					slice = pitch * height;
				}
			}
			break;
		}

		rowPitch = pitch;
		slicePitch = slice;
		return true;
	}

	uint32_t DDSLoader::BitsPerPixel(DXGI_FORMAT fmt)
	{
		switch (static_cast<int>(fmt))
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_YUY2:
			return 32;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			return 24;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 16;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
			return 12;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			return 0;
		}
	}

	bool DDSLoader::IsCompressed(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return true;

		default:
			return false;
		}
	}

	bool DDSLoader::Decompress(const uint8_t* pSrc, DXGI_FORMAT srcFormat)
	{
		uint32_t dbpp = BitsPerPixel(format);
		if (dbpp < 8)
			return false;
		dbpp = (dbpp + 7) / 8;
		uint8_t* pDest = pixels.get();
		if (!pDest)
			return false;

		DXGI_FORMAT cformat;
		switch (srcFormat)
		{
		case DXGI_FORMAT_BC1_TYPELESS:  cformat = DXGI_FORMAT_BC1_UNORM; break;
		case DXGI_FORMAT_BC2_TYPELESS:  cformat = DXGI_FORMAT_BC2_UNORM; break;
		case DXGI_FORMAT_BC3_TYPELESS:  cformat = DXGI_FORMAT_BC3_UNORM; break;
		case DXGI_FORMAT_BC4_TYPELESS:  cformat = DXGI_FORMAT_BC4_UNORM; break;
		case DXGI_FORMAT_BC5_TYPELESS:  cformat = DXGI_FORMAT_BC5_UNORM; break;
		case DXGI_FORMAT_BC6H_TYPELESS: cformat = DXGI_FORMAT_BC6H_UF16; break;
		case DXGI_FORMAT_BC7_TYPELESS:  cformat = DXGI_FORMAT_BC7_UNORM; break;
		default:                        cformat = srcFormat;         break;
		}

		// Determine BC format decoder
		BC_DECODE pfDecode;
		uint32_t sbpp;
		switch (cformat)
		{
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:    pfDecode = D3DXDecodeBC1;   sbpp = 8;   break;
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:    pfDecode = D3DXDecodeBC2;   sbpp = 16;  break;
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:    pfDecode = D3DXDecodeBC3;   sbpp = 16;  break;
		case DXGI_FORMAT_BC4_UNORM:         pfDecode = D3DXDecodeBC4U;  sbpp = 8;   break;
		case DXGI_FORMAT_BC4_SNORM:         pfDecode = D3DXDecodeBC4S;  sbpp = 8;   break;
		case DXGI_FORMAT_BC5_UNORM:         pfDecode = D3DXDecodeBC5U;  sbpp = 16;  break;
		case DXGI_FORMAT_BC5_SNORM:         pfDecode = D3DXDecodeBC5S;  sbpp = 16;  break;
		case DXGI_FORMAT_BC6H_UF16:         pfDecode = D3DXDecodeBC6HU; sbpp = 16;  break;
		case DXGI_FORMAT_BC6H_SF16:         pfDecode = D3DXDecodeBC6HS; sbpp = 16;  break;
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:    pfDecode = D3DXDecodeBC7;   sbpp = 16;  break;
		default:
			return false;
		}

		__declspec(align(16)) XMVECTOR temp[16];
		for (uint32_t h = 0; h < height; h += 4)
		{
			const uint8_t *sptr = pSrc;
			uint8_t* dptr = pDest;
			uint32_t ph = std::min<uint32_t>(4, height - h);
			uint32_t w = 0;
			for (uint32_t count = 0; (count < rowPitch) && (w < width); count += sbpp, w += 4)
			{
				pfDecode(temp, sptr);
				_ConvertScanline(temp, 16, format, cformat, 0);

				uint32_t pw = std::min<uint32_t>(4, width - w);
				assert(pw > 0 && ph > 0);

				if (!_StoreScanline(dptr, rowPitch, format, &temp[0], pw))
					return false;

				if (ph > 1)
				{
					if (!_StoreScanline(dptr + rowPitch, rowPitch, format, &temp[4], pw))
						return false;

					if (ph > 2)
					{
						if (!_StoreScanline(dptr + rowPitch * 2, rowPitch, format, &temp[8], pw))
							return false;

						if (ph > 3)
						{
							if (!_StoreScanline(dptr + rowPitch * 3, rowPitch, format, &temp[12], pw))
								return false;
						}
					}
				}

				sptr += sbpp;
				dptr += dbpp * 4;
			}

			pSrc += rowPitch;
			pDest += rowPitch * 4;
		}
		return true;
	}

	DXGI_FORMAT DDSLoader::DefaultDecompress(DXGI_FORMAT format)
	{
		switch (format)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
			return DXGI_FORMAT_R8_UNORM;

		case DXGI_FORMAT_BC4_SNORM:
			return DXGI_FORMAT_R8_SNORM;

		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;

		case DXGI_FORMAT_BC5_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;

		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
			// We could use DXGI_FORMAT_R32G32B32_FLOAT here since BC6H is always Alpha 1.0,
			// but this format is more supported by viewers
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

}