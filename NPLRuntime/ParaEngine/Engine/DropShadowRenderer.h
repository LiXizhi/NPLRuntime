

#pragma once
#include <stdint.h>
#include"ParaEngine.h"
#include "VertexFVF.h"

namespace ParaEngine
{
	class VertexPositionIndex
	{
	public:
		Vector3 Position;
		float InstanceIndex;
		float v;

		VertexPositionIndex();
	};

	class DropShadowRenderer
	{
	public:
		DropShadowRenderer();
		~DropShadowRenderer();
		void Render();
		void CreateBuffer();
		void BeginFrame();
		void DropShadowRenderer::AddInstance(Vector3& worldPos,Vector3& renderPos,float size);

		uint32_t GetInstanceCount(){return m_instanceCount;}
		uint32_t GetMaxInstanceCount(){return g_maxInstanceCount;}

		void InitDeviceObjects();
		void InvalidateDeviceObjects();
		void DeleteDeviceObjects();
		void Cleanup();

	private:
		static const uint32_t g_maxInstancePerBatch = 70;
		static const uint32_t g_maxInstanceCount = 100;

		VertexBufferDevicePtr_type m_pGeometryBuffer;
		IndexBufferDevicePtr_type m_pIndexBuffer;
		
		uint16_t m_instanceVertexCount;
		uint16_t m_instanceIndexCount;
		
		uint16_t m_totalVertexCount;
		uint16_t m_totalIndexCount;

		uint16_t m_instanceCount;

		std::vector<float> m_constantBuffer;

		void CreateCylinder(float height,float radius,int tesselationFactor,Vector3* pVertices,uint16_t* pIndices);

		void DrawAllBatch(IRenderDevice* pDevice);
	};
}