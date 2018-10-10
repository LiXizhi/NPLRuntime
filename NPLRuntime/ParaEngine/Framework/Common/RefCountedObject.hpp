#pragma once
#include <cstdint>
#include <cassert>
namespace ParaEngine
{
	
	class RefCountedObject
	{
	public:
		RefCountedObject():m_nRefCount(INIT_REF_COUNT)
		{

		}
		virtual ~RefCountedObject()
		{
			assert(m_nRefCount == 0);
		}

		uint32_t AddRef()
		{
			++m_nRefCount;
			return m_nRefCount;
		}
		uint32_t DefRef()
		{
			--m_nRefCount;
			assert(m_nRefCount >= 0);
			return m_nRefCount;
		}

		uint32_t GetRef() const
		{
			return m_nRefCount;
		}

		void Release()
		{
			if (m_nRefCount == INIT_REF_COUNT)
			{
				OnRelease();
			}
			DefRef();
		}

		void ResetRef()
		{
			m_nRefCount = 0;
		}
	protected:
		virtual void OnRelease() = 0;

	private:
		const unsigned  int INIT_REF_COUNT = 1;
		unsigned int m_nRefCount;

	};
}