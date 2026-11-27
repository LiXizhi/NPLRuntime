#include "ParaEngine.h"
#include "Core/ParaImage.h"
#include "Core/TextureEntity.h"
#include "Core/ImageEntity.h"
#include "OpenGLWrapper/GLTexture2D.h"
#include "renderer/EffectManager.h"
#include "RenderDeviceNull.h"
#include "RHIStateTracker.h"
#include "RHISoftwareS3TC.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace ParaEngine;

static void Check(bool ok, const char* message)
{
	if (!ok) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
	std::printf("PASS: %s\n", message);
	std::fflush(stdout);
}

// Exercise the actual image/texture path without requiring a graphics context.
class RecordingDevice : public RenderDeviceNull
{
public:
	RecordingDevice() { m_caps.supportsBCCompression = true; }
	ERHIBackend GetBackendType() const override { return ERHIBackend::D3D11; }
	void SetBC(bool enabled) { m_caps.supportsBCCompression = enabled; }
	bool failUpload = false;
	PixelFormat uploadedFormat = PixelFormat::Unknown;
	std::vector<std::vector<uint8_t>> uploadedMips;
	IRHITexture* CreateTexture(uint32_t w, uint32_t h, PixelFormat fmt, uint32_t levels, ERHITextureUsage usage,
		const RHIMipData* mips, uint32_t count) override
	{
		uploadedFormat = fmt;
		uploadedMips.clear();
		for (uint32_t i = 0; i < count; ++i)
		{
			const auto* bytes = static_cast<const uint8_t*>(mips[i].data);
			uploadedMips.emplace_back(bytes, bytes + mips[i].dataSize);
		}
		return failUpload ? nullptr : RenderDeviceNull::CreateTexture(w, h, fmt, levels, usage, mips, count);
	}
};

static std::vector<uint8_t> MakeDDS()
{
	std::vector<uint8_t> file(128, 0);
	const auto put = [&](size_t offset, uint32_t value) {
		for (int i = 0; i < 4; ++i) file[offset + i] = static_cast<uint8_t>(value >> (i * 8));
	};
	put(0, 0x20534444); put(4, 124); put(12, 8); put(16, 8); put(28, 4);
	put(76, 32); put(80, 4); put(84, 0x35545844); // DXT5, 8x8 -> 1x1
	for (uint32_t size : { 8u, 4u, 2u, 1u })
	{
		std::vector<uint8_t> rgba(size * size * 4, 255), blocks;
		Check(RHISoftwareS3TC::EncodeFromPixels(PixelFormat::DXT5, PixelFormat::A8R8G8B8, rgba.data(), rgba.size(), size, size, blocks), "DDS fixture encode");
		file.insert(file.end(), blocks.begin(), blocks.end());
	}
	return file;
}

int main()
{
	RecordingDevice device;
	auto* previous = CGlobals::GetRenderDevice();
	CGlobals::SetRenderDevice(&device);
	{
		EffectManager effects;
		const auto address = ESamplerStateType::ADDRESSU;
		effects.SetSamplerState(0, address, D3DTADDRESS_CLAMP);
		device.SetSamplerState(0, address, D3DTADDRESS_WRAP); // End of a terrain pass.
		DWORD saved = 0;
		effects.GetSamplerState(0, address, &saved);
		Check(saved == D3DTADDRESS_WRAP, "effects observe direct sampler changes");
		effects.SetSamplerState(0, address, D3DTADDRESS_CLAMP);
		uint32_t current = 0;
		device.GetSamplerState(0, address, &current);
		Check(current == D3DTADDRESS_CLAMP, "next terrain pass clamps its mask again");
	}
	{
		RHIStateTracker states;
		GLTexture2D artwork;
		artwork.setPointMagnification(true);
		states.ClearAllDirty();
		states.SetSamplerTexture(2, &artwork);
		Check(states.GetDirtySamplerMask() == (1u << 2), "binding artwork updates its sampler stage");
		Check(states.GetEffectiveSamplerState(2, ESamplerStateType::MAGFILTER) == D3DTEXF_POINT &&
			states.GetEffectiveSamplerState(2, ESamplerStateType::MINFILTER) == D3DTEXF_LINEAR,
			"artwork magnifies with nearest and minifies with linear");
		Check(states.GetSamplerState(2, ESamplerStateType::MAGFILTER) == D3DTEXF_LINEAR &&
			states.GetEffectiveSamplerState(0, ESamplerStateType::MAGFILTER) == D3DTEXF_LINEAR,
			"requested and unrelated sampler states stay independent");
		states.SetSamplerTexture(2, nullptr);
		Check(states.GetEffectiveSamplerState(2, ESamplerStateType::MAGFILTER) == D3DTEXF_LINEAR,
			"unbinding artwork restores normal magnification");
	}
	{
		std::vector<uint8_t> rgba(8 * 8 * 4, 255);
		ParaImage opaque;
		Check(opaque.initWithRawData(rgba.data(), rgba.size(), 8, 8, 32), "opaque image");
		GLTexture2D texture;
		Check(texture.initWithImage(&opaque), "default upload");
		Check(device.uploadedFormat == PixelFormat::DXT1 && device.uploadedMips[0].size() == 32, "opaque default must be DXT1");
		rgba[3] = 80;
		ParaImage alpha;
		Check(alpha.initWithRawData(rgba.data(), rgba.size(), 8, 8, 32), "alpha image");
		Check(texture.initWithImage(&alpha), "alpha upload");
		Check(device.uploadedFormat == PixelFormat::DXT5 && device.uploadedMips[0].size() == 64, "alpha default must be DXT5");
		Check(texture.initWithImage(&alpha, PixelFormat::DXT3) && device.uploadedFormat == PixelFormat::DXT3, "explicit DXT3 request");
		Check(texture.initWithImage(&alpha, PixelFormat::A8R8G8B8) && device.uploadedMips[0] == rgba, "explicit RGBA must remain lossless");
		ImageEntity composed;
		Check(composed.LoadFromRawData(rgba.data(), rgba.size(), 8, 8, 32), "composed image");
		TextureEntityOpenGL composedTexture("Texture/composed.png");
		Check(composedTexture.LoadFromImage(&composed, PixelFormat::DXT3) && device.uploadedFormat == PixelFormat::DXT3, "texture composer DXT3 upload");
		TextureEntityOpenGL named("Texture/test_32BITS.PNG");
		Check(named.LoadFromImage(&alpha, 0) && device.uploadedFormat == PixelFormat::A8R8G8B8, "32bits filename opt-out");
		TextureEntity::g_bEnable32bitsTexture = false;
		Check(named.LoadFromImage(&alpha, 0) && device.uploadedFormat == PixelFormat::DXT5, "32bits global override");
		TextureEntity::g_bEnable32bitsTexture = true;
		device.SetBC(false);
		Check(texture.initWithImage(&alpha) && device.uploadedFormat == PixelFormat::A8R8G8B8, "unsupported BC fallback");
		device.SetBC(true);
		ParaImage oddSize;
		std::vector<uint8_t> oddPixels(3 * 5 * 4, 255);
		Check(oddSize.initWithRawData(oddPixels.data(), oddPixels.size(), 3, 5, 32), "unaligned image");
		Check(texture.initWithImage(&oddSize) && device.uploadedFormat == PixelFormat::A8R8G8B8, "unaligned BC base upload must fall back");

		auto dds = MakeDDS();
		ParaImage gpuImage, cpuImage;
		Check(gpuImage.initWithImageData(dds.data(), dds.size(), true), "DDS preserve load");
		Check(gpuImage.isCompressed() && gpuImage.getNumberOfMipmaps() == 4, "DDS format/mips lost");
		Check(texture.initWithImage(&gpuImage), "DDS upload");
		Check(device.uploadedFormat == PixelFormat::DXT5 && device.uploadedMips.size() == 4, "DDS upload expanded");
		size_t offset = 128;
		for (const auto& mip : device.uploadedMips)
		{
			Check(std::equal(mip.begin(), mip.end(), dds.begin() + offset), "DDS block bytes changed");
			offset += mip.size();
		}
		Check(offset == dds.size(), "DDS mip tail lost");
		Check(cpuImage.initWithImageData(dds.data(), dds.size()) && !cpuImage.isCompressed(), "CPU consumers must still decode DDS");
		Check(named.LoadFromMemory(reinterpret_cast<const char*>(dds.data()), static_cast<DWORD>(dds.size()), 0) == S_OK &&
			device.uploadedFormat == PixelFormat::A8R8G8B8, "serial DDS 32bits opt-out");
		Check(texture.initWithImage(&gpuImage, PixelFormat::A8R8G8B8), "DDS explicit 32bits upload");
		Check(device.uploadedMips.size() == 4 && device.uploadedMips[0].size() == 256 && device.uploadedMips[3].size() == 4, "DDS decoded mip sizes");
		ParaImage truncated, tooManyMips;
		Check(!truncated.initWithImageData(dds.data(), dds.size() - 1, true), "truncated DDS accepted");
		dds[28] = 17;
		Check(!tooManyMips.initWithImageData(dds.data(), dds.size(), true), "invalid mip count accepted");
		device.failUpload = true;
		Check(!texture.initWithImage(&alpha), "upload failure hidden");
		Check(!named.LoadFromImage(&alpha, 0), "asset upload failure hidden");
	}
	CGlobals::SetRenderDevice(previous);
	std::puts("Texture upload and DDS tests passed");
}
