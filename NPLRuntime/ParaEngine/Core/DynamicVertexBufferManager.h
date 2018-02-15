#pragma once
#include "DynamicVertexBufferEntity.h"

namespace ParaEngine
{
	/** buffer manager */
	class DynamicVertexBufferManager
	{
	protected:
		DynamicVertexBufferEntity	m_pDVBuffers[DVB_COUNT];
	public:
		DynamicVertexBufferManager();

		DynamicVertexBufferEntity* GetDynamicBuffer(DynamicVBAssetType nBufferType);

		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
	};
}
