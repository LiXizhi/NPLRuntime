#include "RHISoftwareS3TC.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

using namespace ParaEngine;

static void Check(bool passed, const char* message)
{
	if (!passed) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}

static void Benchmark()
{
	for (unsigned size : { 256u, 1024u, 2048u })
	{
		std::vector<uint8_t> rgba(size * size * 4);
		// Repeatable mix of gradients and fine detail; report measurements, never assert timing.
		for (unsigned y = 0; y < size; ++y)
			for (unsigned x = 0; x < size; ++x)
			{
				const size_t i = (y * size + x) * 4;
				rgba[i] = static_cast<uint8_t>(x + (y % 13) * 3);
				rgba[i + 1] = static_cast<uint8_t>(y + (x % 17) * 2);
				rgba[i + 2] = static_cast<uint8_t>((x + y) / 2);
				rgba[i + 3] = 255;
			}
		for (bool highQuality : { false, true })
		{
			std::vector<uint8_t> blocks, decoded;
			for (int warmup = 0; warmup < 2; ++warmup)
				Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), size, size, blocks, highQuality), "benchmark warmup");
			const auto start = std::chrono::steady_clock::now();
			for (int repeat = 0; repeat < 5; ++repeat)
				Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), size, size, blocks, highQuality), "benchmark encode");
			const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() / 5;
			Check(RHISoftwareS3TC::DecodeToRGBA8(PixelFormat::DXT1, blocks.data(), blocks.size(), size, size, decoded), "benchmark decode");
			double error = 0;
			for (size_t i = 0; i < rgba.size(); ++i)
				if (i % 4 != 3) { const double d = double(rgba[i]) - decoded[i]; error += d * d; }
			const double psnr = 10 * std::log10(255.0 * 255.0 / (error / (size * size * 3)));
			std::printf("%ux%u %s: %.2f ms, %.2f dB RGB PSNR, %zu -> %zu bytes\n", size, size, highQuality ? "high" : "normal", ms, psnr, rgba.size(), blocks.size());
		}
	}
}

int main(int argc, char** argv)
{
	if (argc > 1 && std::strcmp(argv[1], "--benchmark") == 0) { Benchmark(); return 0; }
	for (PixelFormat format : { PixelFormat::DXT1, PixelFormat::DXT3, PixelFormat::DXT5 })
	{
		// Include partial edge blocks and the 1x1/2x2 tail of a DDS mip chain.
		for (unsigned size : { 1u, 2u, 4u, 7u, 16u })
		{
			const unsigned width = size, height = size == 7 ? 5 : size;
			std::vector<uint8_t> rgba(width * height * 4);
			for (size_t i = 0; i < rgba.size(); i += 4)
			{
				rgba[i] = 231; rgba[i + 1] = 65; rgba[i + 2] = 33;
				rgba[i + 3] = format == PixelFormat::DXT1 ? 255 : static_cast<uint8_t>((i / 4 % 16) * 17);
			}
			std::vector<uint8_t> compressed, decoded;
			Check(RHISoftwareS3TC::EncodeFromPixels(format, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), width, height, compressed), "encode failed");
			Check(compressed.size() == ((width + 3) / 4) * ((height + 3) / 4) * (format == PixelFormat::DXT1 ? 8 : 16), "wrong block size");
			Check(RHISoftwareS3TC::DecodeToRGBA8(format, compressed.data(), compressed.size(), width, height, decoded), "decode failed");
			Check(decoded.size() == rgba.size(), "wrong decoded size");
			for (size_t i = 0; i < rgba.size(); ++i)
			{
				const int tolerance = i % 4 == 3 ? (format == PixelFormat::DXT5 ? 18 : 0) : 8;
				Check(std::abs(int(rgba[i]) - int(decoded[i])) <= tolerance, "color/alpha round trip exceeded tolerance");
			}
			Check(!RHISoftwareS3TC::DecodeToRGBA8(format, compressed.data(), compressed.size() - 1, width, height, decoded), "truncated block accepted");
			Check(!RHISoftwareS3TC::EncodeFromPixels(format, PixelFormat::A8R8G8B8, rgba.data(), rgba.size() - 1, width, height, compressed), "truncated RGBA accepted");
		}
	}
	// DXT3 must preserve sharp 4-bit alpha steps independently of the color encoder.
	std::vector<uint8_t> rgba(64, 255), compressed, decoded;
	for (int i = 0; i < 16; ++i) rgba[i * 4 + 3] = static_cast<uint8_t>(i * 17);
	Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT3, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), 4, 4, compressed), "DXT3 encode failed");
	for (int i = 0; i < 8; ++i) Check(compressed[i] == ((i * 2) | ((i * 2 + 1) << 4)), "incorrect DXT3 alpha nibble order");
	Check(!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT2, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), 4, 4, compressed), "unsupported encoder format accepted");
	Check(!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, nullptr, 64, 4, 4, compressed), "null data accepted");
	Check(!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, rgba.data(), 64, 0, 4, compressed), "zero dimension accepted");
	Check(!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, rgba.data(), 64, 0xffffffffu, 0xffffffffu, compressed), "oversized dimensions accepted");
	std::vector<uint8_t> rgb(4 * 4 * 3, 255), rgbBlocks, rgbaBlocks;
	Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::R8G8B8, rgb.data(), rgb.size(), 4, 4, rgbBlocks), "direct RGB encode");
	std::fill(rgba.begin(), rgba.end(), 255);
	Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), 4, 4, rgbaBlocks), "RGBA comparison encode");
	Check(rgbBlocks == rgbaBlocks, "direct RGB differs from expanded RGBA");
	Check(!RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT1, PixelFormat::R8G8B8, rgb.data(), rgb.size() - 1, 4, 4, rgbBlocks), "truncated RGB accepted");
	std::puts("Texture compression tests passed");
}
