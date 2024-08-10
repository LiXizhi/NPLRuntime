#pragma once
#include "AssetManager.h"
#include <unordered_set>

namespace ParaEngine
{
	class ParaVertexBuffer;

	/** pool of vertex buffer object. */
	class ParaVertexBufferPool : public AssetEntity
	{
	public:
		ParaVertexBufferPool(const AssetKey& key);
		ParaVertexBufferPool();
		virtual ~ParaVertexBufferPool();

		ATTRIBUTE_DEFINE_CLASS(ParaVertexBufferPool);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		ATTRIBUTE_METHOD1(ParaVertexBufferPool, GetFullSizedBufferSize_s, int*)		{ *p1 = cls->GetFullSizedBufferSize(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVertexBufferPool, SetFullSizedBufferSize_s, int)	{ cls->SetFullSizedBufferSize((uint32)p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaVertexBufferPool, GetMaxPooledCount_s, int*)		{ *p1 = cls->GetMaxPooledCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVertexBufferPool, SetMaxPooledCount_s, int)	{ cls->SetMaxPooledCount((uint32)p1); return S_OK; }

		ATTRIBUTE_METHOD1(ParaVertexBufferPool, GetActiveBufferCount_s, int*)		{ *p1 = cls->GetActiveBufferCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVertexBufferPool, GetTotalBufferCount_s, int*)		{ *p1 = cls->GetTotalBufferCount(); return S_OK; }
		ATTRIBUTE_METHOD1(ParaVertexBufferPool, GetTotalBufferBytes_s, int*)		{ *p1 = (int)cls->GetTotalBufferBytes(); return S_OK; }

	public:

		uint32 GetFullSizedBufferSize() const;
		void SetFullSizedBufferSize(uint32 val);
		uint32 GetMaxPooledCount() const;
		void SetMaxPooledCount(uint32 val);
		size_t GetTotalBufferBytes(bool bRecalculate=false);

		int GetActiveBufferCount() const;
		/** including active and unused buffer count. */
		int GetTotalBufferCount() const;

		/** recreate the buffer based on the new size. it will create from pool if buffer equals GetFullSizedBufferSize()
		* otherwise it will create a new buffer object. 
		* @param dwFormat: only used in DirectX, ignored in openGL
		* @param dwUsage: only used in DirectX, ignored in openGL
		* @return: never delete the returned buffer, instead one should call ReleaseBuffer. 
		*/
		ParaVertexBuffer* CreateBuffer(uint32 nBufferSize, DWORD dwFormat = 0, DWORD dwUsage = 0, EPoolType dwPool = EPoolType::Managed);

		/** release the buffer. If it is a full sized buffer we will not release it but add it to a pool for reuse. 
		*/
		void ReleaseBuffer(ParaVertexBuffer* pBuffer);

		/** called between render tick to release some cached pool object if needed. */
		void TickCache();

		/** clear all pooled objects. */
		virtual void Cleanup();
	
		virtual HRESULT DeleteDeviceObjects();
		virtual HRESULT RendererRecreated();
		
		
	protected:
		std::unordered_set<ParaVertexBuffer*> m_activeBuffers;
		std::unordered_set<ParaVertexBuffer*> m_unusedFullSizedBuffers;

		/** we will reuse full sized buffer */
		uint32 m_nFullSizedBufferSize;

		/** max number of pooled objects. */
		uint32 m_nMaxPooledCount;

		/** total number of bytes in memory.*/
		size_t m_nTotalBufferBytes;
		
	};


	/** a manger of sequences */
	class CVertexBufferPoolManager : public AssetManager < ParaVertexBufferPool, ParaVertexBufferPool, ParaVertexBufferPool >
	{
	public:
		CVertexBufferPoolManager(){};

		ParaVertexBufferPool* CreateGetPool(const std::string& name);

		static CVertexBufferPoolManager& GetInstance();
	public:
		void TickCache();

		/** get total number of bytes in vertex buffer pool for quick stats. */
		size_t GetVertexBufferPoolTotalBytes();

	};

}
	
