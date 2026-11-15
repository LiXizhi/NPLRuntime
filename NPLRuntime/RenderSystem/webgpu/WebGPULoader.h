#pragma once
#include "webgpu_c_api.h"

namespace ParaEngine
{
	struct WebGPUDispatch
	{
		void* moduleHandle = nullptr;

#if !defined(__EMSCRIPTEN__)
		WGPUInstance (WGPU_CALL* wgpuCreateInstance)(const WGPUInstanceDescriptor* descriptor) = nullptr;
		void (WGPU_CALL* wgpuInstanceRelease)(WGPUInstance instance) = nullptr;
		void (WGPU_CALL* wgpuInstanceProcessEvents)(WGPUInstance instance) = nullptr;
		WGPUSurface (WGPU_CALL* wgpuInstanceCreateSurface)(WGPUInstance instance, const WGPUSurfaceDescriptor* descriptor) = nullptr;
		void (WGPU_CALL* wgpuInstanceRequestAdapter)(WGPUInstance instance, const WGPURequestAdapterOptions* options, WGPURequestAdapterCallback callback, void* userdata) = nullptr;

		void (WGPU_CALL* wgpuSurfaceRelease)(WGPUSurface surface) = nullptr;
		void (WGPU_CALL* wgpuSurfaceConfigure)(WGPUSurface surface, const WGPUSurfaceConfiguration* config) = nullptr;
		void (WGPU_CALL* wgpuSurfaceUnconfigure)(WGPUSurface surface) = nullptr;
		void (WGPU_CALL* wgpuSurfaceGetCurrentTexture)(WGPUSurface surface, WGPUSurfaceTexture* surfaceTexture) = nullptr;
		void (WGPU_CALL* wgpuSurfacePresent)(WGPUSurface surface) = nullptr;
		WGPUTextureFormat (WGPU_CALL* wgpuSurfaceGetPreferredFormat)(WGPUSurface surface, WGPUAdapter adapter) = nullptr;

		void (WGPU_CALL* wgpuAdapterRelease)(WGPUAdapter adapter) = nullptr;
		WGPUBool (WGPU_CALL* wgpuAdapterHasFeature)(WGPUAdapter adapter, WGPUFeatureName feature) = nullptr;
		void (WGPU_CALL* wgpuAdapterGetProperties)(WGPUAdapter adapter, WGPUAdapterProperties* properties) = nullptr;
		void (WGPU_CALL* wgpuAdapterRequestDevice)(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor, WGPURequestDeviceCallback callback, void* userdata) = nullptr;

		void (WGPU_CALL* wgpuDeviceRelease)(WGPUDevice device) = nullptr;
		void (WGPU_CALL* wgpuDeviceDestroy)(WGPUDevice device) = nullptr;
		void (WGPU_CALL* wgpuDeviceTick)(WGPUDevice device) = nullptr;
		WGPUBool (WGPU_CALL* wgpuDevicePoll)(WGPUDevice device, WGPUBool wait, const void* wrappedSubmissionIndex) = nullptr;
		WGPUBool (WGPU_CALL* wgpuDeviceHasFeature)(WGPUDevice device, WGPUFeatureName feature) = nullptr;
		void (WGPU_CALL* wgpuDeviceSetUncapturedErrorCallback)(WGPUDevice device, WGPUErrorCallback callback, void* userdata) = nullptr;
		WGPUQueue (WGPU_CALL* wgpuDeviceGetQueue)(WGPUDevice device) = nullptr;

		WGPUBuffer (WGPU_CALL* wgpuDeviceCreateBuffer)(WGPUDevice device, const WGPUBufferDescriptor* descriptor) = nullptr;
		WGPUTexture (WGPU_CALL* wgpuDeviceCreateTexture)(WGPUDevice device, const WGPUTextureDescriptor* descriptor) = nullptr;
		WGPUSampler (WGPU_CALL* wgpuDeviceCreateSampler)(WGPUDevice device, const WGPUSamplerDescriptor* descriptor) = nullptr;
		WGPUBindGroupLayout (WGPU_CALL* wgpuDeviceCreateBindGroupLayout)(WGPUDevice device, const WGPUBindGroupLayoutDescriptor* descriptor) = nullptr;
		WGPUPipelineLayout (WGPU_CALL* wgpuDeviceCreatePipelineLayout)(WGPUDevice device, const WGPUPipelineLayoutDescriptor* descriptor) = nullptr;
		WGPUBindGroup (WGPU_CALL* wgpuDeviceCreateBindGroup)(WGPUDevice device, const WGPUBindGroupDescriptor* descriptor) = nullptr;
		WGPUShaderModule (WGPU_CALL* wgpuDeviceCreateShaderModule)(WGPUDevice device, const WGPUShaderModuleDescriptor* descriptor) = nullptr;
		WGPURenderPipeline (WGPU_CALL* wgpuDeviceCreateRenderPipeline)(WGPUDevice device, const WGPURenderPipelineDescriptor* descriptor) = nullptr;
		WGPUCommandEncoder (WGPU_CALL* wgpuDeviceCreateCommandEncoder)(WGPUDevice device, const WGPUCommandEncoderDescriptor* descriptor) = nullptr;
		WGPUQuerySet (WGPU_CALL* wgpuDeviceCreateQuerySet)(WGPUDevice device, const WGPUQuerySetDescriptor* descriptor) = nullptr;

		void (WGPU_CALL* wgpuQueueRelease)(WGPUQueue queue) = nullptr;
		void (WGPU_CALL* wgpuQueueSubmit)(WGPUQueue queue, size_t commandCount, const WGPUCommandBuffer* commands) = nullptr;
		void (WGPU_CALL* wgpuQueueWriteBuffer)(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, const void* data, size_t size) = nullptr;
		void (WGPU_CALL* wgpuQueueWriteTexture)(WGPUQueue queue, const WGPUImageCopyTexture* destination, const void* data, size_t dataSize, const WGPUTextureDataLayout* dataLayout, const WGPUExtent3D* writeSize) = nullptr;

		void (WGPU_CALL* wgpuBufferRelease)(WGPUBuffer buffer) = nullptr;
		void (WGPU_CALL* wgpuBufferDestroy)(WGPUBuffer buffer) = nullptr;
		void (WGPU_CALL* wgpuBufferMapAsync)(WGPUBuffer buffer, WGPUMapModeFlags mode, size_t offset, size_t size, WGPUBufferMapCallback callback, void* userdata) = nullptr;
		void* (WGPU_CALL* wgpuBufferGetMappedRange)(WGPUBuffer buffer, size_t offset, size_t size) = nullptr;
		const void* (WGPU_CALL* wgpuBufferGetConstMappedRange)(WGPUBuffer buffer, size_t offset, size_t size) = nullptr;
		void (WGPU_CALL* wgpuBufferUnmap)(WGPUBuffer buffer) = nullptr;

		void (WGPU_CALL* wgpuTextureRelease)(WGPUTexture texture) = nullptr;
		void (WGPU_CALL* wgpuTextureDestroy)(WGPUTexture texture) = nullptr;
		WGPUTextureView (WGPU_CALL* wgpuTextureCreateView)(WGPUTexture texture, const WGPUTextureViewDescriptor* descriptor) = nullptr;
		void (WGPU_CALL* wgpuTextureViewRelease)(WGPUTextureView textureView) = nullptr;

		void (WGPU_CALL* wgpuSamplerRelease)(WGPUSampler sampler) = nullptr;
		void (WGPU_CALL* wgpuBindGroupLayoutRelease)(WGPUBindGroupLayout layout) = nullptr;
		void (WGPU_CALL* wgpuPipelineLayoutRelease)(WGPUPipelineLayout layout) = nullptr;
		void (WGPU_CALL* wgpuBindGroupRelease)(WGPUBindGroup bindGroup) = nullptr;
		void (WGPU_CALL* wgpuShaderModuleRelease)(WGPUShaderModule shaderModule) = nullptr;
		void (WGPU_CALL* wgpuRenderPipelineRelease)(WGPURenderPipeline renderPipeline) = nullptr;
		void (WGPU_CALL* wgpuQuerySetRelease)(WGPUQuerySet querySet) = nullptr;
		void (WGPU_CALL* wgpuQuerySetDestroy)(WGPUQuerySet querySet) = nullptr;

		void (WGPU_CALL* wgpuCommandEncoderRelease)(WGPUCommandEncoder encoder) = nullptr;
		WGPURenderPassEncoder (WGPU_CALL* wgpuCommandEncoderBeginRenderPass)(WGPUCommandEncoder encoder, const WGPURenderPassDescriptor* descriptor) = nullptr;
		void (WGPU_CALL* wgpuCommandEncoderCopyBufferToBuffer)(WGPUCommandEncoder encoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size) = nullptr;
		void (WGPU_CALL* wgpuCommandEncoderCopyBufferToTexture)(WGPUCommandEncoder encoder, const WGPUImageCopyBuffer* source, const WGPUImageCopyTexture* destination, const WGPUExtent3D* copySize) = nullptr;
		void (WGPU_CALL* wgpuCommandEncoderCopyTextureToBuffer)(WGPUCommandEncoder encoder, const WGPUImageCopyTexture* source, const WGPUImageCopyBuffer* destination, const WGPUExtent3D* copySize) = nullptr;
		void (WGPU_CALL* wgpuCommandEncoderCopyTextureToTexture)(WGPUCommandEncoder encoder, const WGPUImageCopyTexture* source, const WGPUImageCopyTexture* destination, const WGPUExtent3D* copySize) = nullptr;
		void (WGPU_CALL* wgpuCommandEncoderResolveQuerySet)(WGPUCommandEncoder encoder, WGPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, WGPUBuffer destination, uint64_t destinationOffset) = nullptr;
		WGPUCommandBuffer (WGPU_CALL* wgpuCommandEncoderFinish)(WGPUCommandEncoder encoder, const WGPUCommandBufferDescriptor* descriptor) = nullptr;
		void (WGPU_CALL* wgpuCommandBufferRelease)(WGPUCommandBuffer commandBuffer) = nullptr;

		void (WGPU_CALL* wgpuRenderPassEncoderRelease)(WGPURenderPassEncoder encoder) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderEnd)(WGPURenderPassEncoder encoder) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetPipeline)(WGPURenderPassEncoder encoder, WGPURenderPipeline pipeline) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetBindGroup)(WGPURenderPassEncoder encoder, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, const uint32_t* dynamicOffsets) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetVertexBuffer)(WGPURenderPassEncoder encoder, uint32_t slot, WGPUBuffer buffer, uint64_t offset, uint64_t size) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetIndexBuffer)(WGPURenderPassEncoder encoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetViewport)(WGPURenderPassEncoder encoder, float x, float y, float width, float height, float minDepth, float maxDepth) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetScissorRect)(WGPURenderPassEncoder encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetBlendConstant)(WGPURenderPassEncoder encoder, const WGPUColor* color) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderSetStencilReference)(WGPURenderPassEncoder encoder, uint32_t reference) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderBeginOcclusionQuery)(WGPURenderPassEncoder encoder, uint32_t queryIndex) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderEndOcclusionQuery)(WGPURenderPassEncoder encoder) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderDraw)(WGPURenderPassEncoder encoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = nullptr;
		void (WGPU_CALL* wgpuRenderPassEncoderDrawIndexed)(WGPURenderPassEncoder encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) = nullptr;
#endif

		bool Load();
		void Unload();
		void ProcessEvents(WGPUInstance instance, WGPUDevice device) const;
		void WaitAsync(WGPUInstance instance, WGPUDevice device) const;
	};
}

#if defined(__EMSCRIPTEN__)
#define PARA_WGPU_CALL(disp, fn, ...) ::fn(__VA_ARGS__)
#define PARA_WGPU_HAS(disp, fn) (true)
#else
#define PARA_WGPU_CALL(disp, fn, ...) (disp).fn(__VA_ARGS__)
#define PARA_WGPU_HAS(disp, fn) ((disp).fn != nullptr)
#endif

