#pragma once
#include "AssetEntity.h"
#include "DynamicVertexBufferEntity.h"

namespace ParaEngine
{
	/** When use DirectX, this is a real dynamic vertex buffer for rendering dynamic mesh
	When use OpenGL, we simulate it with client memory vertex array. Although VBO is preferred in opengl, 
	it does not perform well on mobile devices in opengl es 2.0. However, we can easily switch to VBO with unchanged interface in future. 
	*/
	class DynamicVertexBufferEntityOpenGL : public AssetEntity
	{
	private:
		std::vector<char> m_lpVB;

	public:
		/// @see DynamicVBAssetType
		DWORD	m_dwDataFormat;
		/// current base or start of the unused space
		DWORD	m_dwBase;
		/// current base or start of the unused space
		DWORD	m_dwNextBase;
		/// must flush after reaching this size
		DWORD	m_dwFlush;
		/// size of the buffer
		DWORD	m_dwDiscard;
		/// sizeof(custom vertex)
		uint32	m_nUnitSize;

	public:
		uint32 Lock(uint32 nSize, void** pVertices);
		void Unlock();

		/** always return 0, since we will disable VBO and use vertex array instead. */
		GLuint GetBuffer();

		DWORD GetBaseVertex();
		/** whether we are using system memory as dynamic vertex buffer. */
		virtual bool IsMemoryBuffer();
		void* GetBaseVertexPointer();

		DynamicVertexBufferEntityOpenGL();
		virtual ~DynamicVertexBufferEntityOpenGL(){};

		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
	};

	typedef DynamicVertexBufferEntityOpenGL DynamicVertexBufferEntity;
}