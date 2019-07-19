#pragma once
#include <string>

namespace ParaEngine
{
#define DXGI_FORMAT_DEFINED 1

	typedef enum DXGI_FORMAT
	{
		DXGI_FORMAT_UNKNOWN = 0,
		DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
		DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
		DXGI_FORMAT_R32G32B32A32_UINT = 3,
		DXGI_FORMAT_R32G32B32A32_SINT = 4,
		DXGI_FORMAT_R32G32B32_TYPELESS = 5,
		DXGI_FORMAT_R32G32B32_FLOAT = 6,
		DXGI_FORMAT_R32G32B32_UINT = 7,
		DXGI_FORMAT_R32G32B32_SINT = 8,
		DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
		DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
		DXGI_FORMAT_R16G16B16A16_UNORM = 11,
		DXGI_FORMAT_R16G16B16A16_UINT = 12,
		DXGI_FORMAT_R16G16B16A16_SNORM = 13,
		DXGI_FORMAT_R16G16B16A16_SINT = 14,
		DXGI_FORMAT_R32G32_TYPELESS = 15,
		DXGI_FORMAT_R32G32_FLOAT = 16,
		DXGI_FORMAT_R32G32_UINT = 17,
		DXGI_FORMAT_R32G32_SINT = 18,
		DXGI_FORMAT_R32G8X24_TYPELESS = 19,
		DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
		DXGI_FORMAT_R10G10B10A2_UNORM = 24,
		DXGI_FORMAT_R10G10B10A2_UINT = 25,
		DXGI_FORMAT_R11G11B10_FLOAT = 26,
		DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
		DXGI_FORMAT_R8G8B8A8_UNORM = 28,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		DXGI_FORMAT_R8G8B8A8_UINT = 30,
		DXGI_FORMAT_R8G8B8A8_SNORM = 31,
		DXGI_FORMAT_R8G8B8A8_SINT = 32,
		DXGI_FORMAT_R16G16_TYPELESS = 33,
		DXGI_FORMAT_R16G16_FLOAT = 34,
		DXGI_FORMAT_R16G16_UNORM = 35,
		DXGI_FORMAT_R16G16_UINT = 36,
		DXGI_FORMAT_R16G16_SNORM = 37,
		DXGI_FORMAT_R16G16_SINT = 38,
		DXGI_FORMAT_R32_TYPELESS = 39,
		DXGI_FORMAT_D32_FLOAT = 40,
		DXGI_FORMAT_R32_FLOAT = 41,
		DXGI_FORMAT_R32_UINT = 42,
		DXGI_FORMAT_R32_SINT = 43,
		DXGI_FORMAT_R24G8_TYPELESS = 44,
		DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
		DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
		DXGI_FORMAT_R8G8_TYPELESS = 48,
		DXGI_FORMAT_R8G8_UNORM = 49,
		DXGI_FORMAT_R8G8_UINT = 50,
		DXGI_FORMAT_R8G8_SNORM = 51,
		DXGI_FORMAT_R8G8_SINT = 52,
		DXGI_FORMAT_R16_TYPELESS = 53,
		DXGI_FORMAT_R16_FLOAT = 54,
		DXGI_FORMAT_D16_UNORM = 55,
		DXGI_FORMAT_R16_UNORM = 56,
		DXGI_FORMAT_R16_UINT = 57,
		DXGI_FORMAT_R16_SNORM = 58,
		DXGI_FORMAT_R16_SINT = 59,
		DXGI_FORMAT_R8_TYPELESS = 60,
		DXGI_FORMAT_R8_UNORM = 61,
		DXGI_FORMAT_R8_UINT = 62,
		DXGI_FORMAT_R8_SNORM = 63,
		DXGI_FORMAT_R8_SINT = 64,
		DXGI_FORMAT_A8_UNORM = 65,
		DXGI_FORMAT_R1_UNORM = 66,
		DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
		DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
		DXGI_FORMAT_BC1_TYPELESS = 70,
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_TYPELESS = 73,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_TYPELESS = 76,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_TYPELESS = 79,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_TYPELESS = 82,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_B5G6R5_UNORM = 85,
		DXGI_FORMAT_B5G5R5A1_UNORM = 86,
		DXGI_FORMAT_B8G8R8A8_UNORM = 87,
		DXGI_FORMAT_B8G8R8X8_UNORM = 88,
		DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
		DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
		DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		DXGI_FORMAT_BC6H_TYPELESS = 94,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_TYPELESS = 97,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
		DXGI_FORMAT_AYUV = 100,
		DXGI_FORMAT_Y410 = 101,
		DXGI_FORMAT_Y416 = 102,
		DXGI_FORMAT_NV12 = 103,
		DXGI_FORMAT_P010 = 104,
		DXGI_FORMAT_P016 = 105,
		DXGI_FORMAT_420_OPAQUE = 106,
		DXGI_FORMAT_YUY2 = 107,
		DXGI_FORMAT_Y210 = 108,
		DXGI_FORMAT_Y216 = 109,
		DXGI_FORMAT_NV11 = 110,
		DXGI_FORMAT_AI44 = 111,
		DXGI_FORMAT_IA44 = 112,
		DXGI_FORMAT_P8 = 113,
		DXGI_FORMAT_A8P8 = 114,
		DXGI_FORMAT_B4G4R4A4_UNORM = 115,
		DXGI_FORMAT_FORCE_UINT = 0xffffffff
	} DXGI_FORMAT;

	enum CP_FLAGS
	{
		CP_FLAGS_NONE = 0x0,      // Normal operation
		CP_FLAGS_LEGACY_DWORD = 0x1,      // Assume pitch is DWORD aligned instead of BYTE aligned
		CP_FLAGS_PARAGRAPH = 0x2,      // Assume pitch is 16-byte aligned instead of BYTE aligned
		CP_FLAGS_YMM = 0x4,      // Assume pitch is 32-byte aligned instead of BYTE aligned
		CP_FLAGS_ZMM = 0x8,      // Assume pitch is 64-byte aligned instead of BYTE aligned
		CP_FLAGS_PAGE4K = 0x200,    // Assume pitch is 4096-byte aligned instead of BYTE aligned
		CP_FLAGS_BAD_DXTN_TAILS = 0x1000,   // BC formats with malformed mipchain blocks smaller than 4x4
		CP_FLAGS_24BPP = 0x10000,  // Override with a legacy 24 bits-per-pixel format size
		CP_FLAGS_16BPP = 0x20000,  // Override with a legacy 16 bits-per-pixel format size
		CP_FLAGS_8BPP = 0x40000,  // Override with a legacy 8 bits-per-pixel format size
	};

	enum TEX_DIMENSION
		// Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
	{
		TEX_DIMENSION_TEXTURE1D = 2,
		TEX_DIMENSION_TEXTURE2D = 3,
		TEX_DIMENSION_TEXTURE3D = 4,
	};

	enum TEX_MISC_FLAG
		// Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
	{
		TEX_MISC_TEXTURECUBE = 0x4L,
	};

	enum TEX_MISC_FLAG2
	{
		TEX_MISC2_ALPHA_MODE_MASK = 0x7L,
	};

	enum TEX_ALPHA_MODE
		// Matches DDS_ALPHA_MODE, encoded in MISC_FLAGS2
	{
		TEX_ALPHA_MODE_UNKNOWN = 0,
		TEX_ALPHA_MODE_STRAIGHT = 1,
		TEX_ALPHA_MODE_PREMULTIPLIED = 2,
		TEX_ALPHA_MODE_OPAQUE = 3,
		TEX_ALPHA_MODE_CUSTOM = 4,
	};

	enum DDS_FLAGS
	{
		DDS_FLAGS_NONE = 0x0,

		DDS_FLAGS_LEGACY_DWORD = 0x1,
		// Assume pitch is DWORD aligned instead of BYTE aligned (used by some legacy DDS files)

		DDS_FLAGS_NO_LEGACY_EXPANSION = 0x2,
		// Do not implicitly convert legacy formats that result in larger pixel sizes (24 bpp, 3:3:2, A8L8, A4L4, P8, A8P8) 

		DDS_FLAGS_NO_R10B10G10A2_FIXUP = 0x4,
		// Do not use work-around for long-standing D3DX DDS file format issue which reversed the 10:10:10:2 color order masks

		DDS_FLAGS_FORCE_RGB = 0x8,
		// Convert DXGI 1.1 BGR formats to DXGI_FORMAT_R8G8B8A8_UNORM to avoid use of optional WDDM 1.1 formats

		DDS_FLAGS_NO_16BPP = 0x10,
		// Conversions avoid use of 565, 5551, and 4444 formats and instead expand to 8888 to avoid use of optional WDDM 1.2 formats

		DDS_FLAGS_EXPAND_LUMINANCE = 0x20,
		// When loading legacy luminance formats expand replicating the color channels rather than leaving them packed (L8, L16, A8L8)

		DDS_FLAGS_BAD_DXTN_TAILS = 0x40,
		// Some older DXTn DDS files incorrectly handle mipchain tails for blocks smaller than 4x4

		DDS_FLAGS_FORCE_DX10_EXT = 0x10000,
		// Always use the 'DX10' header extension for DDS writer (i.e. don't try to write DX9 compatible DDS files)

		DDS_FLAGS_FORCE_DX10_EXT_MISC2 = 0x20000,
		// DDS_FLAGS_FORCE_DX10_EXT including miscFlags2 information (result may not be compatible with D3DX10 or D3DX11)
	};

	enum TEX_FILTER_FLAGS
	{
		TEX_FILTER_DEFAULT = 0,

		TEX_FILTER_WRAP_U = 0x1,
		TEX_FILTER_WRAP_V = 0x2,
		TEX_FILTER_WRAP_W = 0x4,
		TEX_FILTER_WRAP = (TEX_FILTER_WRAP_U | TEX_FILTER_WRAP_V | TEX_FILTER_WRAP_W),
		TEX_FILTER_MIRROR_U = 0x10,
		TEX_FILTER_MIRROR_V = 0x20,
		TEX_FILTER_MIRROR_W = 0x40,
		TEX_FILTER_MIRROR = (TEX_FILTER_MIRROR_U | TEX_FILTER_MIRROR_V | TEX_FILTER_MIRROR_W),
		// Wrap vs. Mirror vs. Clamp filtering options

		TEX_FILTER_SEPARATE_ALPHA = 0x100,
		// Resize color and alpha channel independently

		TEX_FILTER_FLOAT_X2BIAS = 0x200,
		// Enable *2 - 1 conversion cases for unorm<->float and positive-only float formats

		TEX_FILTER_RGB_COPY_RED = 0x1000,
		TEX_FILTER_RGB_COPY_GREEN = 0x2000,
		TEX_FILTER_RGB_COPY_BLUE = 0x4000,
		// When converting RGB to R, defaults to using grayscale. These flags indicate copying a specific channel instead
		// When converting RGB to RG, defaults to copying RED | GREEN. These flags control which channels are selected instead.

		TEX_FILTER_DITHER = 0x10000,
		// Use ordered 4x4 dithering for any required conversions
		TEX_FILTER_DITHER_DIFFUSION = 0x20000,
		// Use error-diffusion dithering for any required conversions

		TEX_FILTER_POINT = 0x100000,
		TEX_FILTER_LINEAR = 0x200000,
		TEX_FILTER_CUBIC = 0x300000,
		TEX_FILTER_BOX = 0x400000,
		TEX_FILTER_FANT = 0x400000, // Equiv to Box filtering for mipmap generation
		TEX_FILTER_TRIANGLE = 0x500000,
		// Filtering mode to use for any required image resizing

		TEX_FILTER_SRGB_IN = 0x1000000,
		TEX_FILTER_SRGB_OUT = 0x2000000,
		TEX_FILTER_SRGB = (TEX_FILTER_SRGB_IN | TEX_FILTER_SRGB_OUT),
		// sRGB <-> RGB for use in conversion operations
		// if the input format type is IsSRGB(), then SRGB_IN is on by default
		// if the output format type is IsSRGB(), then SRGB_OUT is on by default

		TEX_FILTER_FORCE_NON_WIC = 0x10000000,
		// Forces use of the non-WIC path when both are an option

		TEX_FILTER_FORCE_WIC = 0x20000000,
		// Forces use of the WIC path even when logic would have picked a non-WIC path when both are an option
	};

	struct DDS_HEADER;
	struct DDS_PIXELFORMAT;
	class DDSLoader
	{
	public:
		DDSLoader(const std::string& ddsFile);
		~DDSLoader();

		bool ConvertDDSToPng();
		std::shared_ptr<uint8_t> GetPngBuffer() { return pngBuffer; }
		uint32_t GetPngSize() { return pngSize; }

	private:
		bool LoadDDSFile();
		bool DecodeDDSHeader(const void* source, uint32_t size, uint32_t flags, uint32_t& convFlags);
		DXGI_FORMAT GetDXGIFormat(const DDS_HEADER& hdr, const DDS_PIXELFORMAT& ddpf, uint32_t flags, uint32_t& convFlags);
		DXGI_FORMAT MakeSRGB(DXGI_FORMAT fmt);
		bool InitializeImage(uint32_t flags);
		bool CalculateMipLevels(uint32_t width, uint32_t height, uint32_t& mipLevels);
		uint32_t CountMips(uint32_t width, uint32_t height);
		bool ComputePitch(DXGI_FORMAT fmt, uint32_t width, uint32_t height, uint32_t& rowPitch, uint32_t& slicePitch, uint32_t flags);
		uint32_t BitsPerPixel(DXGI_FORMAT fmt);
		bool IsCompressed(DXGI_FORMAT fmt);
		bool Decompress(const uint8_t* pSrc, DXGI_FORMAT srcFormat);
		DXGI_FORMAT DefaultDecompress(DXGI_FORMAT format);

		std::string fileName;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t arraySize;
		uint32_t mipLevels;
		uint32_t miscFlags;
		uint32_t miscFlags2;
		uint32_t rowPitch;
		uint32_t pixelSize;
		uint32_t pngSize;
		std::unique_ptr<uint8_t[]> pixels;
		std::shared_ptr<uint8_t> pngBuffer;
		DXGI_FORMAT format;
		TEX_DIMENSION dimension;
	};
}