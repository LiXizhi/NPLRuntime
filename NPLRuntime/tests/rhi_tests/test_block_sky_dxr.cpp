#include "d3d12/RenderContextD3D12.h"
#include "d3d12/RenderDeviceD3D12.h"
#include "Framework/Interface/Render/IRenderWindow.h"
#include "Framework/Interface/Render/IRenderDevice.h"
#include "Framework/Interface/Render/IBlockSkyRayTracer.h"
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <memory>

using namespace ParaEngine;

class TestWindow final : public IRenderWindow
{
public:
	HWND hwnd = CreateWindowExW(0, L"STATIC", L"DXR test", WS_POPUP, 0, 0, 64, 64, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
	~TestWindow() { DestroyWindow(hwnd); }
	unsigned GetWidth() const override { return 64; }
	unsigned GetHeight() const override { return 64; }
	float GetScaleX() const override { return 1; }
	float GetScaleY() const override { return 1; }
	intptr_t GetNativeHandle() const override { return reinterpret_cast<intptr_t>(hwnd); }
};

int main()
{
	TestWindow window;
	RenderConfiguration config;
	config.renderWindow = &window;
	config.screenWidth = config.screenHeight = 64;
	RenderContextD3D12 context;
	std::unique_ptr<IRenderDevice> device(context.CreateDevice(config));
	if (!device) return 77;
	auto* tracer = dynamic_cast<IBlockSkyRayTracer*>(device.get());
	std::printf("Adapter: %s\n", device->GetCaps().adapterName);
	auto* nativeDevice = static_cast<RenderDeviceD3D12*>(device.get())->GetD3D12Device();
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
	D3D12_FEATURE_DATA_SHADER_MODEL model = { D3D_SHADER_MODEL_6_5 };
	const bool expected = !device->GetCaps().isSoftwareAdapter &&
		SUCCEEDED(nativeDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options))) &&
		options.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1 &&
		SUCCEEDED(nativeDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model))) &&
		model.HighestShaderModel >= D3D_SHADER_MODEL_6_5;
	if (!tracer || tracer->SupportsBlockSkyTracing() != expected) return 1;
	if (!expected) return 77;
	using Sky = IBlockSkyRayTracer;
	std::unique_ptr<IRHIRenderTarget> rt(device->CreateRenderTarget(Sky::AtlasWidth, Sky::AtlasHeight, PixelFormat::R32F, PixelFormat::Unknown));
	if (!rt) return 1;
	auto* texture = rt->GetColorTexture();
	std::vector<float> pixels(Sky::AtlasWidth * Sky::AtlasHeight);
	auto trace = [&](const std::vector<BlockSkySpan>& spans) {
		return tracer->TraceBlockSky(spans, texture) && device->ReadTexturePixels(texture, pixels.data());
	};
	auto sample = [&](int x, int y, int z) {
		return pixels[(z + 32 * (y / 8)) * Sky::AtlasWidth + x + 32 * (y % 8)];
	};
	std::vector<BlockSkySpan> roof;
	for (int z = 8; z < 24; ++z) roof.push_back({8, 16, (uint8_t)z, 16});
	if (!trace(roof)) return 1;
	float below = sample(16, 15, 16), above = sample(16, 17, 16);
	std::printf("Sky visibility below roof %.4f, above roof %.4f\n", below, above);
	if (!(below >= 0 && below < 0.3f && above == 1)) return 1;
	if (!trace({})) return 1;
	return std::all_of(pixels.begin(), pixels.end(), [](float v) { return v == 1; }) ? 0 : 1;
}
