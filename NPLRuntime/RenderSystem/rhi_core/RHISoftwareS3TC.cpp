#include "RHISoftwareS3TC.h"
#include <string.h>
#include <limits>
#define STB_DXT_STATIC
#define STB_DXT_IMPLEMENTATION
#include "third_party/stb_dxt.h"

namespace ParaEngine
{
	namespace
	{
		inline void UnpackRGB565(uint16_t c, uint8_t& r, uint8_t& g, uint8_t& b)
		{
			uint32_t rv = (c >> 11) & 0x1Fu;
			uint32_t gv = (c >> 5) & 0x3Fu;
			uint32_t bv = c & 0x1Fu;
			r = static_cast<uint8_t>((rv << 3) | (rv >> 2));
			g = static_cast<uint8_t>((gv << 2) | (gv >> 4));
			b = static_cast<uint8_t>((bv << 3) | (bv >> 2));
		}

		void DecodeColorBlock(const uint8_t* block, bool allowOneBitAlpha, uint8_t palette[4][4])
		{
			uint16_t c0 = 0, c1 = 0;
			memcpy(&c0, block, 2);
			memcpy(&c1, block + 2, 2);

			UnpackRGB565(c0, palette[0][0], palette[0][1], palette[0][2]);
			palette[0][3] = 255;
			UnpackRGB565(c1, palette[1][0], palette[1][1], palette[1][2]);
			palette[1][3] = 255;

			if (c0 > c1 || !allowOneBitAlpha)
			{
				for (int ch = 0; ch < 3; ++ch)
				{
					palette[2][ch] = static_cast<uint8_t>((2u * palette[0][ch] + palette[1][ch] + 1u) / 3u);
					palette[3][ch] = static_cast<uint8_t>((palette[0][ch] + 2u * palette[1][ch] + 1u) / 3u);
				}
				palette[2][3] = 255;
				palette[3][3] = 255;
			}
			else
			{
				for (int ch = 0; ch < 3; ++ch)
				{
					palette[2][ch] = static_cast<uint8_t>((palette[0][ch] + palette[1][ch]) / 2u);
					palette[3][ch] = 0;
				}
				palette[2][3] = 255;
				palette[3][3] = 0;
			}
		}
	}

	bool RHISoftwareS3TC::IsS3TCFormat(PixelFormat format)
	{
		return format == PixelFormat::DXT1 || format == PixelFormat::DXT2 ||
			format == PixelFormat::DXT3 || format == PixelFormat::DXT4 || format == PixelFormat::DXT5;
	}

	size_t RHISoftwareS3TC::ComputeCompressedMipSize(PixelFormat format, uint32_t width, uint32_t height)
	{
		if (!IsS3TCFormat(format) || width == 0 || height == 0)
			return 0;
		size_t blocksX = width / 4u + (width % 4u != 0);
		size_t blocksY = height / 4u + (height % 4u != 0);
		size_t blockBytes = (format == PixelFormat::DXT1) ? 8u : 16u;
		if (blocksX > (std::numeric_limits<size_t>::max)() / blockBytes / blocksY)
			return 0;
		return blocksX * blocksY * blockBytes;
	}

	bool RHISoftwareS3TC::EncodeFromPixels(PixelFormat format, PixelFormat sourceFormat, const void* srcData, size_t srcByteSize, uint32_t width, uint32_t height, std::vector<uint8_t>& outBlocks, bool highQuality)
	{
		outBlocks.clear();
		const size_t channels = sourceFormat == PixelFormat::R8G8B8 ? 3 : 4;
		if ((sourceFormat != PixelFormat::R8G8B8 && sourceFormat != PixelFormat::A8R8G8B8) ||
			(format != PixelFormat::DXT1 && format != PixelFormat::DXT3 && format != PixelFormat::DXT5) ||
			!srcData || !width || !height || width > (std::numeric_limits<size_t>::max)() / channels / height ||
			srcByteSize < static_cast<size_t>(width) * height * channels)
			return false;
		const size_t bytes = ComputeCompressedMipSize(format, width, height);
		if (!bytes)
			return false;
		outBlocks.resize(bytes);
		const uint8_t* rgba = static_cast<const uint8_t*>(srcData);
		uint8_t* dest = outBlocks.data();
		for (size_t y = 0; y < height; y += 4)
		{
			for (size_t x = 0; x < width; x += 4)
			{
				uint8_t block[64];
				for (size_t dy = 0; dy < 4; ++dy)
					for (size_t dx = 0; dx < 4; ++dx)
					{
						const size_t sx = x + dx < width ? x + dx : width - 1;
						const size_t sy = y + dy < height ? y + dy : height - 1;
						memcpy(block + (dy * 4 + dx) * 4, rgba + (sy * width + sx) * channels, channels);
						if (channels == 3) block[(dy * 4 + dx) * 4 + 3] = 255;
					}
				if (format == PixelFormat::DXT3)
				{
					for (int i = 0; i < 8; ++i)
						dest[i] = static_cast<uint8_t>(((block[i * 8 + 3] + 8) / 17) |
							(((block[i * 8 + 7] + 8) / 17) << 4));
					dest += 8;
				}
				// stb's color compressor requires constant alpha when alpha=0.
				if (format != PixelFormat::DXT5)
					for (int i = 0; i < 16; ++i)
						block[i * 4 + 3] = 255;
				stb_compress_dxt_block(dest, block, format == PixelFormat::DXT5, highQuality ? STB_DXT_HIGHQUAL : STB_DXT_NORMAL);
				dest += format == PixelFormat::DXT5 ? 16 : 8;
			}
		}
		return true;
	}

	bool RHISoftwareS3TC::DecodeToRGBA8(PixelFormat format, const void* srcData, size_t srcByteSize, uint32_t width, uint32_t height, std::vector<uint8_t>& outRGBA)
	{
		if (!IsS3TCFormat(format) || !srcData || width == 0 || height == 0)
			return false;

		size_t requiredBytes = ComputeCompressedMipSize(format, width, height);
		if (!requiredBytes || srcByteSize < requiredBytes || width > (std::numeric_limits<size_t>::max)() / 4u / height)
			return false;

		outRGBA.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u);

		const uint8_t* ptr = static_cast<const uint8_t*>(srcData);
		const bool isDXT1 = (format == PixelFormat::DXT1);
		const bool isDXT3 = (format == PixelFormat::DXT2 || format == PixelFormat::DXT3);

		for (uint32_t by = 0; by < height; by += 4u)
		{
			const uint32_t maxY = (height - by < 4u) ? (height - by) : 4u;
			for (uint32_t bx = 0; bx < width; bx += 4u)
			{
				const uint32_t maxX = (width - bx < 4u) ? (width - bx) : 4u;
				uint64_t alphaBits = 0;
				uint8_t alphaTable[8] = { 255, 255, 255, 255, 255, 255, 255, 255 };

				if (!isDXT1)
				{
					memcpy(&alphaBits, ptr, 8);
					ptr += 8;
					if (!isDXT3)
					{
						// DXT5 interpolated alpha
						alphaTable[0] = static_cast<uint8_t>(alphaBits & 0xFFu);
						alphaTable[1] = static_cast<uint8_t>((alphaBits >> 8) & 0xFFu);
						if (alphaTable[0] > alphaTable[1])
						{
							for (int i = 1; i <= 6; ++i)
								alphaTable[i + 1] = static_cast<uint8_t>(((7 - i) * alphaTable[0] + i * alphaTable[1] + 3) / 7);
						}
						else
						{
							for (int i = 1; i <= 4; ++i)
								alphaTable[i + 1] = static_cast<uint8_t>(((5 - i) * alphaTable[0] + i * alphaTable[1] + 2) / 5);
							alphaTable[6] = 0;
							alphaTable[7] = 255;
						}
						alphaBits >>= 16;
					}
				}

				uint8_t palette[4][4];
				DecodeColorBlock(ptr, isDXT1, palette);
				uint32_t colorIndices = 0;
				memcpy(&colorIndices, ptr + 4, 4);
				ptr += 8;

				for (uint32_t py = 0; py < 4u; ++py)
				{
					for (uint32_t px = 0; px < 4u; ++px)
					{
						uint32_t cIdx = colorIndices & 0x3u;
						colorIndices >>= 2;

						uint8_t a = palette[cIdx][3];
						if (!isDXT1)
						{
							if (isDXT3)
							{
								uint8_t a4 = static_cast<uint8_t>(alphaBits & 0xFu);
								a = static_cast<uint8_t>((a4 << 4) | a4);
								alphaBits >>= 4;
							}
							else
							{
								a = alphaTable[alphaBits & 0x7u];
								alphaBits >>= 3;
							}
						}

						if (px < maxX && py < maxY)
						{
							size_t dstOffset = (static_cast<size_t>(by + py) * width + (bx + px)) * 4u;
							outRGBA[dstOffset + 0] = palette[cIdx][0];
							outRGBA[dstOffset + 1] = palette[cIdx][1];
							outRGBA[dstOffset + 2] = palette[cIdx][2];
							outRGBA[dstOffset + 3] = a;
						}
					}
				}
			}
		}

		return true;
	}

	bool RHISoftwareS3TC::NeedsRGBA8Expansion(PixelFormat format, bool backendSupportsA8)
	{
		switch (format)
		{
		case PixelFormat::R8G8B8:
		case PixelFormat::L8:
		case PixelFormat::A8L8:
		case PixelFormat::R5G6B5:
		case PixelFormat::A4R4G4B4:
		case PixelFormat::A1R5G5B5:
			return true;
		case PixelFormat::A8:
			return !backendSupportsA8;
		default:
			return false;
		}
	}

	bool RHISoftwareS3TC::ExpandToRGBA8(PixelFormat format, const void* srcData, uint32_t width, uint32_t height, uint32_t srcRowPitch, std::vector<uint8_t>& outRGBA)
	{
		if (width == 0 || height == 0)
			return false;

		outRGBA.assign(static_cast<size_t>(width) * static_cast<size_t>(height) * 4u, 0);
		if (!srcData)
			return true;

		const uint8_t* srcBase = static_cast<const uint8_t*>(srcData);
		switch (format)
		{
		case PixelFormat::R8G8B8:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : (width * 3u);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					dst[x * 4u + 0] = row[x * 3u + 0];
					dst[x * 4u + 1] = row[x * 3u + 1];
					dst[x * 4u + 2] = row[x * 3u + 2];
					dst[x * 4u + 3] = 255;
				}
			}
			return true;
		}
		case PixelFormat::L8:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : width;
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					uint8_t l = row[x];
					dst[x * 4u + 0] = l;
					dst[x * 4u + 1] = l;
					dst[x * 4u + 2] = l;
					dst[x * 4u + 3] = 255;
				}
			}
			return true;
		}
		case PixelFormat::A8L8:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : (width * 2u);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					uint8_t l = row[x * 2u + 0];
					uint8_t a = row[x * 2u + 1];
					dst[x * 4u + 0] = l;
					dst[x * 4u + 1] = l;
					dst[x * 4u + 2] = l;
					dst[x * 4u + 3] = a;
				}
			}
			return true;
		}
		case PixelFormat::A8:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : width;
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					dst[x * 4u + 0] = 255;
					dst[x * 4u + 1] = 255;
					dst[x * 4u + 2] = 255;
					dst[x * 4u + 3] = row[x];
				}
			}
			return true;
		}
		case PixelFormat::R5G6B5:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : (width * 2u);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					uint16_t c = 0;
					memcpy(&c, row + x * 2u, 2);
					UnpackRGB565(c, dst[x * 4u + 0], dst[x * 4u + 1], dst[x * 4u + 2]);
					dst[x * 4u + 3] = 255;
				}
			}
			return true;
		}
		case PixelFormat::A4R4G4B4:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : (width * 2u);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					uint16_t c = 0;
					memcpy(&c, row + x * 2u, 2);
					uint8_t r = static_cast<uint8_t>((c >> 12) & 0xFu);
					uint8_t g = static_cast<uint8_t>((c >> 8) & 0xFu);
					uint8_t b = static_cast<uint8_t>((c >> 4) & 0xFu);
					uint8_t a = static_cast<uint8_t>(c & 0xFu);
					dst[x * 4u + 0] = static_cast<uint8_t>((r << 4) | r);
					dst[x * 4u + 1] = static_cast<uint8_t>((g << 4) | g);
					dst[x * 4u + 2] = static_cast<uint8_t>((b << 4) | b);
					dst[x * 4u + 3] = static_cast<uint8_t>((a << 4) | a);
				}
			}
			return true;
		}
		case PixelFormat::A1R5G5B5:
		{
			uint32_t pitch = srcRowPitch ? srcRowPitch : (width * 2u);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = srcBase + static_cast<size_t>(y) * pitch;
				uint8_t* dst = outRGBA.data() + static_cast<size_t>(y) * width * 4u;
				for (uint32_t x = 0; x < width; ++x)
				{
					uint16_t c = 0;
					memcpy(&c, row + x * 2u, 2);
					uint8_t r = static_cast<uint8_t>((c >> 11) & 0x1Fu);
					uint8_t g = static_cast<uint8_t>((c >> 6) & 0x1Fu);
					uint8_t b = static_cast<uint8_t>((c >> 1) & 0x1Fu);
					dst[x * 4u + 0] = static_cast<uint8_t>((r << 3) | (r >> 2));
					dst[x * 4u + 1] = static_cast<uint8_t>((g << 3) | (g >> 2));
					dst[x * 4u + 2] = static_cast<uint8_t>((b << 3) | (b >> 2));
					dst[x * 4u + 3] = (c & 1u) ? 255 : 0;
				}
			}
			return true;
		}
		default:
			return false;
		}
	}
}
