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

// Character assets expose wrapped RHI textures, unlike the native G-buffer attachments.
class SkinTexture final : public IRHITexture
{
public:
	explicit SkinTexture(IRHITexture& texture) : wrapped(texture) {}
	uint32_t GetWidth() const override { return wrapped.GetWidth(); }
	uint32_t GetHeight() const override { return wrapped.GetHeight(); }
	void* GetNativeHandle() const override { return wrapped.GetNativeHandle(); }
private:
	IRHITexture& wrapped;
};

// Keep the central receiver and its reflected roof hit fixed. Vary the FOV to
// bring the green raster sample inside the viewport over the magenta DXR material.
static bool TestReflectionReprojection(IRenderDevice& device, IBlockSkyRayTracer& tracer)
{
	constexpr unsigned side = 65, middle = side * side / 2;
	std::vector<float> depths(side * side, 5.96f);
	depths[middle] = 2.f;
	std::vector<uint32_t> normals(side * side, 0xff4ff580u), colors(side * side, 0xff00ff00u);
	std::vector<uint32_t> categories(side * side, 0), pixels(side * side);
	categories[middle] = 9;
	auto texture = [&](PixelFormat format, const void* data) {
		RHIMipData mip = {};
		mip.data = data;
		mip.rowPitch = 4 * side;
		mip.dataSize = mip.rowPitch * side;
		return std::unique_ptr<IRHITexture>(device.CreateTexture(side, side, format, 1, ERHITextureUsage::ShaderResource, &mip, 1));
	};
	auto depth = texture(PixelFormat::R32F, depths.data());
	auto normal = texture(PixelFormat::A8R8G8B8, normals.data());
	auto material = texture(PixelFormat::A8R8G8B8, categories.data());
	auto sceneColor = texture(PixelFormat::A8R8G8B8, colors.data());
	std::unique_ptr<IRHIRenderTarget> output(device.CreateRenderTarget(side, side, PixelFormat::A8R8G8B8, PixelFormat::Unknown));
	if (!depth || !normal || !material || !sceneColor || !output) return false;
	BlockReflectionView view = {};
	view.inverseView[0] = view.inverseView[5] = view.inverseView[10] = view.inverseView[15] = 1;
	view.inverseView[12] = 16; view.inverseView[13] = 12; view.inverseView[14] = 10;
	view.viewport[0] = view.viewport[1] = view.volume[3] = view.daylight = view.sun[1] = 1;
	auto readGreen = [&](float lens) -> int {
		view.lens[0] = view.lens[1] = lens;
		if (!tracer.TraceBlockReflections(view, depth.get(), normal.get(), material.get(), sceneColor.get(), output->GetColorTexture()) ||
			!device.ReadTexturePixels(output->GetColorTexture(), pixels.data()) || (pixels[middle] >> 24) != 255) return -1;
		return (pixels[middle] >> 8) & 255;
	};
	if (readGreen(0.62f) != 0) return false;
	int previous = 0;
	for (float lens : {0.69f, 0.72f, 0.78f, 0.86f, 0.97f})
	{
		int green = readGreen(lens);
		std::printf("Reprojected roof at lens %.2f: %d\n", lens, green);
		if (green <= previous || green - previous > 100) return false;
		previous = green;
	}
	if (previous != 255) return false;
	// Projecting onto a foreground occluder must preserve the traced material.
	std::fill(depths.begin(), depths.end(), 3.f);
	depths[middle] = 2.f;
	depth = texture(PixelFormat::R32F, depths.data());
	return depth && readGreen(0.97f) == 0;
}

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
	for (int z = 8; z < 32; ++z) roof.push_back({8, 16, (uint8_t)z, 16, 0xff00ff});
	if (!trace(roof) || !TestReflectionReprojection(*device, *tracer)) return 1;
	float below = sample(16, 15, 16), above = sample(16, 17, 16);
	std::printf("Sky visibility below roof %.4f, above roof %.4f\n", below, above);
	if (!(below >= 0 && below < 0.3f && above == 1)) return 1;
	// The primary rays point down. Only the reflected rays can reach the magenta roof.
	const float depths[] = {2, 2, 2};
	const uint32_t normals[] = {0xff80ff80, 0xff80ff80, 0xff80ff80};
	const uint32_t categories[] = {9, 50, 1};
	auto makeInput = [&](PixelFormat format, const void* data) {
		RHIMipData mip = {};
		mip.data = data;
		mip.dataSize = mip.rowPitch = 12;
		return std::unique_ptr<IRHITexture>(device->CreateTexture(3, 1, format, 1, ERHITextureUsage::ShaderResource, &mip, 1));
	};
	auto depth = makeInput(PixelFormat::R32F, depths);
	auto normal = makeInput(PixelFormat::A8R8G8B8, normals);
	auto material = makeInput(PixelFormat::A8R8G8B8, categories);
	std::unique_ptr<IRHIRenderTarget> reflection(device->CreateRenderTarget(3, 1, PixelFormat::A8R8G8B8, PixelFormat::Unknown));
	if (!depth || !normal || !material || !reflection) return 1;
	BlockReflectionView view = {};
	view.inverseView[9] = -0.7071f; view.inverseView[10] = 0.7071f;
	view.inverseView[12] = 16; view.inverseView[13] = 13; view.inverseView[14] = 12;
	view.viewport[0] = view.viewport[1] = view.volume[3] = view.daylight = view.sun[1] = 1;
	uint32_t reflected[3] = {};
	PlayerReflectionMesh player;
	auto traceReflections = [&]() {
		return tracer->TraceBlockReflections(view, depth.get(), normal.get(), material.get(), material.get(), reflection->GetColorTexture(), &player) &&
			device->ReadTexturePixels(reflection->GetColorTexture(), reflected);
	};
	if (!traceReflections()) return 1;
	std::printf("Off-camera roof: water %08x, metal %08x, matte %08x\n", reflected[0], reflected[1], reflected[2]);
	if ((reflected[0] >> 24) != 255 || !(reflected[0] & 255) || (reflected[0] & 0xff00) ||
		!(reflected[0] & 0xff0000) || reflected[1] != reflected[0] || reflected[2]) return 1;
	// A lamp outside the image retains its emission after nightfall.
	auto lamps = roof;
	for (auto& span : lamps) span.material = 0x0f804020;
	view.daylight = 0;
	if (!trace(lamps) || !traceReflections() || (reflected[0] & 255) <= 128 || (reflected[0] >> 24) != 255) return 1;
	view.daylight = 1;
	if (!trace(roof)) return 1;
	// A green animated triangle lies between the receiving surface and the magenta roof.
	player.vertices = {{10,14,12,0xff00ff00}, {22,14,12,0xff00ff00}, {16,14,22,0xff00ff00}};
	player.indices = {0,1,2};
	player.materials = {PlayerReflectionMesh::MaxTextures};
	if (!traceReflections() || (reflected[0] & 0xff00) == 0 || (reflected[0] & 0xff00ff) ||
		reflected[0] != reflected[1] || reflected[2]) return 1;
	std::printf("Player closest hit: water %08x, metal %08x\n", reflected[0], reflected[1]);
	// The skin texture and its cutout must work even when no source texel is on screen.
	for (auto& v : player.vertices) v.color = 0xffffffff;
	const uint32_t red[] = {0xff0000ff, 0xff0000ff, 0xff0000ff};
	const uint32_t clear[] = {0,0,0};
	auto skin = makeInput(PixelFormat::A8R8G8B8, red);
	auto cutout = makeInput(PixelFormat::A8R8G8B8, clear);
	if (!skin || !cutout) return 1;
	SkinTexture wrappedSkin(*skin);
	player.textures = {&wrappedSkin, cutout.get()};
	player.materials[0] = 0x80000000u;
	if (!traceReflections() || !(reflected[0] & 255) || (reflected[0] & 0xffff00)) return 1;
	player.materials[0] = 0x80000001u;
	if (!traceReflections() || !(reflected[0] & 0xff0000) || !(reflected[0] & 255)) return 1;
	player.materials[0] = 0;
	for (auto& v : player.vertices) v.y += 4;
	if (!traceReflections() || (reflected[0] & 0xff00) || !(reflected[0] & 255)) return 1;
	player.indices.clear();
	uint32_t previous = 256;
	for (int y = 10; y >= 4; --y)
	{
		view.inverseView[13] = float(y);
		if (!traceReflections()) return 1;
		const uint32_t weight = reflected[0] >> 24;
		std::printf("Edge weight at eye Y=%d: %u\n", y, weight);
		if ((y >= 6 && (weight == 0 || weight >= previous)) || (y < 6 && weight > previous) || reflected[1] != reflected[0]) return 1;
		previous = weight;
	}
	if (previous != 0) return 1;
	// Move the receiver away while reflecting the same nearby roof. Re-centering the volume
	// by four blocks must leave both weights identical at a fixed world-space camera.
	const float receiverDepths[] = {9, 12, 12};
	depth = makeInput(PixelFormat::R32F, receiverDepths);
	view.inverseView[9] = -1;
	view.inverseView[10] = 0;
	view.inverseView[13] = 16;
	if (!traceReflections()) return 1;
	uint32_t water = reflected[0], metal = reflected[1];
	if ((water >> 24) >= 255 || (water >> 24) <= (metal >> 24) || (metal >> 24) == 0) return 1;
	view.volume[0] += 4;
	for (auto& span : roof) span.x -= 4;
	if (!trace(roof) || !traceReflections() || water != reflected[0] || metal != reflected[1]) return 1;
	if (!trace({}) || !traceReflections() || reflected[0] || reflected[1]) return 1;
	return std::all_of(pixels.begin(), pixels.end(), [](float v) { return v == 1; }) ? 0 : 1;
}
