#include "RenderDeviceD3D12.h"
#ifdef PE_BLOCK_SKY_DXR
#include "BlockSkyDXIL.h"
#include "BlockReflectionDXIL.h"
#include <algorithm>
#include <cstring>
#endif

using Microsoft::WRL::ComPtr;

namespace ParaEngine
{
	bool RenderDeviceD3D12::SupportsBlockSkyTracing()
	{
		if (m_checkedSkyTracing)
			return m_skyPipeline && m_reflectionPipeline;
		m_checkedSkyTracing = true;
#ifdef PE_BLOCK_SKY_DXR
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options = {};
		D3D12_FEATURE_DATA_SHADER_MODEL model = { D3D_SHADER_MODEL_6_5 };
		if (m_isWarp || FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options, sizeof(options))) ||
			options.RaytracingTier < D3D12_RAYTRACING_TIER_1_1 ||
			FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model))) ||
			model.HighestShaderModel < D3D_SHADER_MODEL_6_5)
			return false;
		if (FAILED(m_device->CreateRootSignature(0, g_blockSkyDXIL, sizeof(g_blockSkyDXIL), IID_PPV_ARGS(&m_skyRoot))))
			return false;
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = m_skyRoot.Get();
		desc.CS = { g_blockSkyDXIL, sizeof(g_blockSkyDXIL) };
		if (FAILED(m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_skyPipeline)))) return false;
		if (FAILED(m_device->CreateRootSignature(0, g_blockReflectionDXIL, sizeof(g_blockReflectionDXIL), IID_PPV_ARGS(&m_reflectionRoot)))) return false;
		desc.pRootSignature = m_reflectionRoot.Get();
		desc.CS = { g_blockReflectionDXIL, sizeof(g_blockReflectionDXIL) };
		return SUCCEEDED(m_device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_reflectionPipeline)));
#else
		return false;
#endif
	}

#ifdef PE_BLOCK_SKY_DXR
	namespace
	{
		ComPtr<ID3D12Resource> CreateSkyBuffer(ID3D12Device* device, UINT64 bytes, D3D12_RESOURCE_STATES state)
		{
			D3D12_HEAP_PROPERTIES heap = {};
			heap.Type = D3D12_HEAP_TYPE_DEFAULT;
			heap.CreationNodeMask = heap.VisibleNodeMask = 1;
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Width = bytes;
			desc.Height = desc.DepthOrArraySize = desc.MipLevels = desc.SampleDesc.Count = 1;
			desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			ComPtr<ID3D12Resource> buffer;
			device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc, state, nullptr, IID_PPV_ARGS(&buffer));
			return buffer;
		}

		void OrderUavWrites(ID3D12GraphicsCommandList* cmd)
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			cmd->ResourceBarrier(1, &barrier);
		}
	}
#endif

#ifdef PE_BLOCK_SKY_DXR
	bool RenderDeviceD3D12::BuildPlayerReflectionScene(const PlayerReflectionMesh& mesh, const BlockReflectionView& view,
		D3D12_GPU_VIRTUAL_ADDRESS& vertices, D3D12_GPU_VIRTUAL_ADDRESS& indices)
	{
		const size_t vbBytes = mesh.vertices.size() * sizeof(ReflectionVertex);
		const size_t ibBytes = mesh.indices.size() * sizeof(uint32_t);
		const size_t materialBytes = mesh.materials.size() * sizeof(uint32_t);
		if (!vbBytes || !ibBytes || mesh.materials.size() != mesh.indices.size() / 3 || vbBytes + ibBytes + materialBytes > UINT32_MAX)
			return false;
		ComPtr<ID3D12Device5> device;
		ComPtr<ID3D12GraphicsCommandList4> cmd;
		if (FAILED(m_device.As(&device)) || FAILED(m_cmdList.As(&cmd))) return false;
		void* cpu;
		ID3D12Resource* upload;
		UINT64 offset;
		if (!AllocateUpload(static_cast<UINT>(vbBytes + ibBytes + materialBytes), 16, cpu, vertices, upload, offset)) return false;
		indices = vertices + vbBytes;
		std::memcpy(cpu, mesh.vertices.data(), vbBytes);
		std::memcpy(static_cast<char*>(cpu) + vbBytes, mesh.indices.data(), ibBytes);
		std::memcpy(static_cast<char*>(cpu) + vbBytes + ibBytes, mesh.materials.data(), materialBytes);
		D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
		geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		geometry.Triangles.VertexBuffer = {vertices, sizeof(ReflectionVertex)};
		geometry.Triangles.VertexCount = static_cast<UINT>(mesh.vertices.size());
		geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometry.Triangles.IndexBuffer = indices;
		geometry.Triangles.IndexCount = static_cast<UINT>(mesh.indices.size());
		geometry.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottom = {}, top = {};
		bottom.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottom.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
		bottom.Inputs.NumDescs = 1;
		bottom.Inputs.pGeometryDescs = &geometry;
		top.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		top.Inputs.NumDescs = 1;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bi = {}, ti = {};
		device->GetRaytracingAccelerationStructurePrebuildInfo(&bottom.Inputs, &bi);
		device->GetRaytracingAccelerationStructurePrebuildInfo(&top.Inputs, &ti);
		auto reserve = [&](ComPtr<ID3D12Resource>& buffer, UINT64 size, D3D12_RESOURCE_STATES state) {
			if (buffer && buffer->GetDesc().Width >= size) return true;
			auto next = CreateSkyBuffer(device.Get(), size, state);
			if (!next) return false;
			DeferRelease(buffer.Get());
			buffer = next;
			return true;
		};
		if (!reserve(m_playerBLAS, bi.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) ||
			!reserve(m_playerTLAS, ti.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) ||
			!reserve(m_playerScratch, (std::max)(bi.ScratchDataSizeInBytes, ti.ScratchDataSizeInBytes), D3D12_RESOURCE_STATE_UNORDERED_ACCESS))
			return false;
		D3D12_RAYTRACING_INSTANCE_DESC instance = {};
		for (int i = 0; i < 3; ++i)
		{
			instance.Transform[i][i] = view.volume[3];
			instance.Transform[i][3] = -view.volume[i] * view.volume[3];
		}
		instance.InstanceMask = 255;
		instance.AccelerationStructure = m_playerBLAS->GetGPUVirtualAddress();
		if (!AllocateUpload(sizeof(instance), 16, cpu, top.Inputs.InstanceDescs, upload, offset)) return false;
		std::memcpy(cpu, &instance, sizeof(instance));
		bottom.DestAccelerationStructureData = instance.AccelerationStructure;
		top.DestAccelerationStructureData = m_playerTLAS->GetGPUVirtualAddress();
		bottom.ScratchAccelerationStructureData = top.ScratchAccelerationStructureData = m_playerScratch->GetGPUVirtualAddress();
		OrderUavWrites(cmd.Get()); // Previous dispatch may still be reading these buffers.
		cmd->BuildRaytracingAccelerationStructure(&bottom, 0, nullptr);
		OrderUavWrites(cmd.Get());
		cmd->BuildRaytracingAccelerationStructure(&top, 0, nullptr);
		OrderUavWrites(cmd.Get());
		return true;
	}
#endif

	bool RenderDeviceD3D12::TraceBlockReflections(const BlockReflectionView& view, IRHITexture* depth,
		IRHITexture* normal, IRHITexture* material, IRHITexture* sceneColor, IRHITexture* output, const PlayerReflectionMesh* player)
	{
#ifdef PE_BLOCK_SKY_DXR
		if (!m_blockTLAS || !output || output->GetFormat() != PixelFormat::A8R8G8B8)
			return false;
		const UINT w = output->GetWidth(), h = output->GetHeight();
		for (auto* input : {depth, normal, material, sceneColor})
			if (!input || input->GetWidth() != w || input->GetHeight() != h) return false;
		const UINT pitch = (w * 4u + 255u) & ~255u;
		const UINT64 bytes = UINT64(pitch) * h;
		if (!m_reflectionBuffer || m_reflectionBuffer->GetDesc().Width < bytes)
		{
			auto buffer = CreateSkyBuffer(m_device.Get(), bytes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			if (!buffer) return false;
			DeferRelease(m_reflectionBuffer.Get());
			m_reflectionBuffer = buffer;
			m_reflectionBufferState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
		if (!BindReflectionInputs(depth, normal, material, sceneColor, player)) return false;
		auto* cmd = EnsureRecording();
		TransitionResource(m_reflectionBuffer.Get(), m_reflectionBufferState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		static_assert(sizeof(view) == 32 * sizeof(float), "reflection shader constant layout");
		cmd->SetPipelineState(m_reflectionPipeline.Get());
		cmd->SetComputeRootShaderResourceView(0, m_blockTLAS->GetGPUVirtualAddress());
		cmd->SetComputeRootUnorderedAccessView(2, m_reflectionBuffer->GetGPUVirtualAddress());
		D3D12_GPU_VIRTUAL_ADDRESS vertices = 0, indices = 0;
		bool hasPlayer = player && BuildPlayerReflectionScene(*player, view, vertices, indices);
		BlockReflectionView constants = view;
		constants.materialByteOffset = hasPlayer ? static_cast<uint32_t>(player->indices.size() * sizeof(uint32_t)) : 0;
		constants.hasPlayer = hasPlayer ? 1.f : 0.f;
		cmd->SetComputeRoot32BitConstants(3, 32, &constants, 0);
		cmd->SetComputeRootShaderResourceView(4, (hasPlayer ? m_playerTLAS : m_blockTLAS)->GetGPUVirtualAddress());
		cmd->SetComputeRootShaderResourceView(5, hasPlayer ? vertices : 0);
		cmd->SetComputeRootShaderResourceView(6, hasPlayer ? indices : 0);
		cmd->Dispatch((w + 7) / 8, (h + 7) / 8, 1);
		TransitionResource(m_reflectionBuffer.Get(), m_reflectionBufferState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		return CopyRayBuffer(m_reflectionBuffer.Get(), output, pitch);
#else
		return false;
#endif
	}

	bool RenderDeviceD3D12::TraceBlockSky(const std::vector<BlockSkySpan>& spans, IRHITexture* atlas)
	{
#ifdef PE_BLOCK_SKY_DXR
		if (!atlas || atlas->GetWidth() != AtlasWidth || atlas->GetHeight() != AtlasHeight || atlas->GetFormat() != PixelFormat::R32F || !SupportsBlockSkyTracing() || spans.size() > Side * Side * Side)
			return false;
		ComPtr<ID3D12Device5> device;
		ComPtr<ID3D12GraphicsCommandList4> cmd;
		if (!EnsureRecording() || FAILED(m_device.As(&device)) || FAILED(m_cmdList.As(&cmd)))
			return false;

		// One triangle BLAS shared by every opaque run, scaled by its TLAS instance.
		static const float vertices[][3] = {
			{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}
		};
		static const uint16_t indices[] = {
			0,2,1,1,2,3, 4,5,6,5,7,6, 0,4,2,2,4,6,
			1,3,5,3,7,5, 0,1,4,1,5,4, 2,6,3,3,6,7
		};
		void* cpu = nullptr;
		ID3D12Resource* upload = nullptr;
		UINT64 offset = 0;
		D3D12_GPU_VIRTUAL_ADDRESS meshVA = 0, instancesVA = 0;
		if (!AllocateUpload(sizeof(vertices) + sizeof(indices), 16, cpu, meshVA, upload, offset))
			return false;
		std::memcpy(cpu, vertices, sizeof(vertices));
		std::memcpy(static_cast<char*>(cpu) + sizeof(vertices), indices, sizeof(indices));
		D3D12_RAYTRACING_GEOMETRY_DESC geometry = {};
		geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometry.Triangles.VertexBuffer = { meshVA, sizeof(vertices[0]) };
		geometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometry.Triangles.VertexCount = 8;
		geometry.Triangles.IndexBuffer = meshVA + sizeof(vertices);
		geometry.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		geometry.Triangles.IndexCount = 36;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottom = {}, top = {};
		bottom.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottom.Inputs.NumDescs = 1;
		bottom.Inputs.pGeometryDescs = &geometry;
		top.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		top.Inputs.NumDescs = (std::max)(1u, static_cast<UINT>(spans.size()));
		if (!AllocateUpload(top.Inputs.NumDescs * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), 16, cpu, instancesVA, upload, offset))
			return false;
		top.Inputs.InstanceDescs = instancesVA;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bi = {}, ti = {};
		device->GetRaytracingAccelerationStructurePrebuildInfo(&bottom.Inputs, &bi);
		device->GetRaytracingAccelerationStructurePrebuildInfo(&top.Inputs, &ti);
		auto blas = CreateSkyBuffer(device.Get(), bi.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		auto tlas = CreateSkyBuffer(device.Get(), ti.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		auto scratch = CreateSkyBuffer(device.Get(), (std::max)(bi.ScratchDataSizeInBytes, ti.ScratchDataSizeInBytes), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		auto result = CreateSkyBuffer(device.Get(), AtlasWidth * AtlasHeight * sizeof(float), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		if (!blas || !tlas || !scratch || !result)
			return false;
		auto* instances = static_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(cpu);
		for (UINT i = 0; i < top.Inputs.NumDescs; ++i)
		{
			const BlockSkySpan span = spans.empty() ? BlockSkySpan{0,0,0,1} : spans[i];
			D3D12_RAYTRACING_INSTANCE_DESC instance = {};
			instance.Transform[0][0] = span.length;
			instance.Transform[1][1] = instance.Transform[2][2] = 1;
			instance.Transform[0][3] = span.x;
			instance.Transform[1][3] = span.y;
			instance.Transform[2][3] = span.z;
			instance.InstanceID = span.color & 0xffffff;
			instance.InstanceMask = spans.empty() ? 0 : 255;
			instance.AccelerationStructure = blas->GetGPUVirtualAddress();
			instances[i] = instance;
		}

		bottom.DestAccelerationStructureData = blas->GetGPUVirtualAddress();
		top.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
		bottom.ScratchAccelerationStructureData = top.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
		cmd->BuildRaytracingAccelerationStructure(&bottom, 0, nullptr);
		OrderUavWrites(cmd.Get());
		cmd->BuildRaytracingAccelerationStructure(&top, 0, nullptr);
		OrderUavWrites(cmd.Get());
		cmd->SetComputeRootSignature(m_skyRoot.Get());
		cmd->SetPipelineState(m_skyPipeline.Get());
		cmd->SetComputeRootShaderResourceView(0, top.DestAccelerationStructureData);
		cmd->SetComputeRootUnorderedAccessView(1, result->GetGPUVirtualAddress());
		cmd->Dispatch(Side / 4, Side / 4, Side / 4);
		D3D12_RESOURCE_STATES resultState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		TransitionResource(result.Get(), resultState, D3D12_RESOURCE_STATE_COPY_SOURCE);
		const bool copied = CopyRayBuffer(result.Get(), atlas, AtlasWidth * sizeof(float));
		DeferRelease(m_blockBLAS.Get());
		DeferRelease(m_blockTLAS.Get());
		m_blockBLAS = blas;
		m_blockTLAS = tlas;
		DeferRelease(scratch.Get());
		DeferRelease(result.Get());
		return copied;
#else
		return false;
#endif
	}
}
